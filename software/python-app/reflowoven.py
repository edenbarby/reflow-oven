import collections
import serial
import serial.tools.list_ports
import re
import struct
import time

import cobs

class ReflowOven:
    '''
    All communications are little endian.
    '''
    COMMS_TYPE_NACK      = 0
    COMMS_TYPE_CMD_IDLE  = 1
    COMMS_TYPE_CMD_RUN   = 2
    COMMS_TYPE_QUERY_ALL = 3
    COMMS_TYPES = (COMMS_TYPE_NACK,
                   COMMS_TYPE_CMD_RUN,
                   COMMS_TYPE_CMD_IDLE,
                   COMMS_TYPE_QUERY_ALL)

    ERROR_TC_COM = (1 << 0)
    ERROR_TC_FLT = (1 << 1)
    ERROR_TC_OPN = (1 << 2)
    ERROR_TC_GND = (1 << 3)
    ERROR_TC_VCC = (1 << 4)
    ERROR_TP_OVN = (1 << 5)
    ERROR_TP_REF = (1 << 6)

    OVEN_STATE_INIT        = 0
    OVEN_STATE_IDLE        = 1
    OVEN_STATE_SOAK_RAMP   = 2
    OVEN_STATE_SOAK        = 3
    OVEN_STATE_REFLOW_RAMP = 4
    OVEN_STATE_REFLOW      = 5
    OVEN_STATES = (OVEN_STATE_INIT,
                   OVEN_STATE_IDLE,
                   OVEN_STATE_SOAK_RAMP,
                   OVEN_STATE_SOAK,
                   OVEN_STATE_REFLOW_RAMP,
                   OVEN_STATE_REFLOW)

    ReflowProfile = collections.namedtuple('ReflowProfile',
                           ('ramp_rate',   'soak_time',
                            'soak_temp',   'reflow_time',
                            'reflow_temp', 'ramp_p',
                            'ramp_i',      'ramp_d',
                            'ramp_i_max',  'fixed_p',
                            'fixed_i',     'fixed_d',
                            'fixed_i_max'))
    OvenState = collections.namedtuple('OvenState',
                           ('errors',      'state',
                            'temp_oven',   'temp_ref',
                            'p_current',   'i_current',
                            'd_current',   'power_current',
                            'ramp_rate',
                            'soak_time',   'soak_temp',
                            'reflow_time', 'reflow_temp',
                            'ramp_p',      'ramp_i',
                            'ramp_d',      'ramp_i_max',
                            'fixed_p',     'fixed_i',
                            'fixed_d',     'fixed_i_max'))
    
    def __init__(self, timeout=0.1, retries=3):
        self.timeout = timeout
        self.retries = retries
        self.oven = serial.Serial(
            bytesize           = serial.EIGHTBITS,
            parity             = serial.PARITY_NONE,
            stopbits           = serial.STOPBITS_ONE,
            timeout            = self.timeout,
            xonxoff            = False,
            rtscts             = False,
            write_timeout      = self.timeout,
            dsrdtr             = False,
            inter_byte_timeout = self.timeout,
            exclusive          = None
        )

    def __repr__(self):
        return f'ReflowOven(timeout={self.timeout},retries={self.retries},port={repr(self.oven)})'

    def connect(self, port=None, baudrate=None):
        try:
            if port and self.oven.port != port:
                self.oven.port = port
            if baudrate and self.oven.baudrate != baudrate:
                self.oven.baudrate = baudrate
            if not self.oven.is_open:
                self.oven.open()
            return True
        except Exception as e:
            print(f'Error: Failed to connect on port {port} with baud rate {baudrate} got exception {e}.')
            self.disconnect()
            return False

    def disconnect(self):
        try:
            self.oven.close()
        except:
            pass

    def reconnect(self):
        self.disconnect()
        return self.connect()

    def receive(self):
        success = False
        packet = bytes()
        data = bytes()
        try:
            packet = self.oven.read_until(bytes([0x00]))
            # print('>', str(packet))
        except Exception as e:
            print(f'Error: Exception encountered on serial.read_until {e}.')
        if packet:
            data = cobs.decode(packet[:-1])
            # print('>', str(list(data)))
            success = True
        return success, data

    def transmit(self, data):
        success = False
        try:
            # print('<', str(list(data)))
            packet = cobs.encode(data)+bytes([0])
            # print('<', str(packet))
            len_sent = self.oven.write(packet)
            if len(packet) == len_sent:
                success = True
            else:
                print('Error: Failed to transmit {} of {} bytes.'.format(len(packet) - len_sent, len(packet)))
        except Exception as e:
            print(f'Error: Failed to transmit got exception {e}.')
        return success        

    def communicate(self, comms_type, data=bytes()):
        success = False
        response = bytes()
        assert(comms_type in self.COMMS_TYPES)
        for attempt in range(self.retries):
            if not self.transmit(bytes([comms_type]) + bytes(data)):
                print('Warning: ({}/{}) transmission failed, reconnecting.'.format(attempt+1, self.retries))
                self.reconnect()
                continue
            recv_success, response = self.receive()
            if not recv_success:
                raise Exception('Timed out waiting on response to {} {}.'.format(comms_type, data))
            if not response:
                raise Exception('Received an empty response to {} {}.'.format(comms_type, data))
            if response[0] == comms_type:
                success = True
                break
            elif packet[0] == self.COMMS_TYPE_NACK:
                print('Warning: ({}/{}) NACK received in response to {} {}.'.format(attempt+1, self.retries, comms_type, data))
                continue
            else:
                raise Exception('Received unexpected response type ({}) to {} {}.'.format(packet, comms_type, data))
        return success, response[1:]
        
    def cmd_idle(self):
        success, _ = self.communicate(self.COMMS_TYPE_CMD_IDLE)
        return success

    def cmd_run(self, relfow_profile):
        assert(type(relfow_profile) == self.ReflowProfile)
        data = struct.pack('<13f', *relfow_profile)
        success, _ = self.communicate(self.COMMS_TYPE_CMD_RUN, data)
        return success

    def query_all(self):
        oven_state_all = None
        success, response = self.communicate(self.COMMS_TYPE_QUERY_ALL)
        if success:
            oven_state_all = self.OvenState(*struct.unpack('<2I19f', response))
        return success, oven_state_all

    def state2str(self, state):
        if state == self.OVEN_STATE_INIT:
            return 'INIT'
        if state == self.OVEN_STATE_IDLE:
            return 'IDLE'
        if state == self.OVEN_STATE_SOAK_RAMP:
            return 'RAMP TO SOAK'
        if state == self.OVEN_STATE_SOAK:
            return 'SOAK'
        if state == self.OVEN_STATE_REFLOW_RAMP:
            return 'RAMP TO RELFOW'
        if state == self.OVEN_STATE_REFLOW:
            return 'REFLOW'
        return ''

    def error2str(self, error):
        error_str = []
        if error & self.ERROR_TC_COM:
            error_str.append('Thermocouple chip communications error.')
        if error & self.ERROR_TC_FLT:
            error_str.append('Thermocouple chip reporting unknown fault.')
        if error & self.ERROR_TC_OPN:
            error_str.append('Thermocouple not connected.')
        if error & self.ERROR_TC_GND:
            error_str.append('Thermocouple shorted to ground.')
        if error & self.ERROR_TC_VCC:
            error_str.append('Thermocouple shorted to power.')
        if error & self.ERROR_TP_OVN:
            error_str.append('Maximum oven temperature exceeded.')
        if error & self.ERROR_TP_REF:
            error_str.append('Maximum controller board temperature exceeded.')
        return error_str

def profile_load_packed(filename):
    profile = bytes()
    with open(filename, 'r') as f:
        # First line only contains column labels, ditch it.
        f.readline()
        # Read the rest, split it, discard empty strings and convert it
        # all to floats.
        profile = [float(i) for i in re.split('[\n\r, ]', f.read()) if i]
        # Pack it all into floats ('f', 4 bytes).
        profile = struct.pack('{}f'.format(len(profile)), *profile)
    return profile

def profile_load(filename):
    profile = []
    with open(filename, 'r') as f:
        f.readline()
        profile = [[int(i) for i in re.split('[\n\r, ]', line) if i] for line in f]
    return profile

def available_serial_ports():
    ports = serial.tools.list_ports.comports()
    devices = dict()
    for p in ports:
        devices[p.device] = p.description
    return devices

if __name__ == '__main__':
    port = 'COM3'
    baudrate = 115200
    profile_filename = 'reflow-profile-3.csv'

    oven = ReflowOven()
    assert(oven.connect(port, baudrate))

    success, errors, state_all = oven.query_all()
    print(success)
    print(errors)
    print(state_all)

    # success, _, state = oven.get_state()
    # assert(success)
    # success, _, temp_oven, temp_ref = oven.get_temp_tc()
    # assert(success)
    # success, errors, temp_cpu = oven.get_temp_cpu()
    # assert(success)
    # print('{} Temperatures (ovn/ref/cpu): {} {} {} Errors: {}'.format(
    #     oven.state2str(state),
    #     temp_oven,
    #     temp_ref,
    #     temp_cpu,
    #     oven.error2str(errors)
    #     ))

    # if state != oven.STATE_RUN:
    #     success, _ = oven.run(profile_filename)
    #     assert(success)
    #     success, _, state = oven.get_state()
    #     assert(success)

    # while state == oven.STATE_RUN:
    #     success, _, state = oven.get_state()
    #     assert(success)
    #     success, _, temp_oven, temp_ref = oven.get_temp_tc()
    #     assert(success)
    #     success, errors, temp_cpu = oven.get_temp_cpu()
    #     assert(success)
    #     print('{} Temperatures (ovn/ref/cpu): {} {} {} Errors: {}'.format(
    #         oven.state2str(state),
    #         temp_oven,
    #         temp_ref,
    #         temp_cpu,
    #         oven.error2str(errors)
    #         ))
    #     time.sleep(0.5)

    # print('Reflow completed!')
