import serial, serial.tools.list_ports, sys, re, struct, time
import cobs

class ReflowOven:
    """\
    All transmissions are big endian.
    """
    COMMS_TYPE_NACK        = 0
    COMMS_TYPE_CMD_WAIT    = 10
    COMMS_TYPE_CMD_RUN     = 11
    COMMS_TYPE_CMD_COOL    = 12
    COMMS_TYPE_QUERY_STATE = 20
    COMMS_TYPE_QUERY_TPOVN = 21
    COMMS_TYPE_QUERY_TPCPU = 22
    COMMS_TYPES = (COMMS_TYPE_NACK,
                   COMMS_TYPE_CMD_WAIT,
                   COMMS_TYPE_CMD_RUN,
                   COMMS_TYPE_CMD_COOL,
                   COMMS_TYPE_QUERY_STATE,
                   COMMS_TYPE_QUERY_TPOVN,
                   COMMS_TYPE_QUERY_TPCPU)

    COMMS_EXPECTED_LENGTHS = (1, 3, 3, 3, 4, 11, 7)

    ERROR_TC_MASK      = 0x0007
    ERROR_TC_COMMS     = 0x0001
    ERROR_TC_FAULT     = 0x0002
    ERROR_TC_OPEN      = 0x0003
    ERROR_TC_SHORT_GND = 0x0004
    ERROR_TC_SHORT_VCC = 0x0005

    REFLOW_STATE_WAIT = 0
    REFLOW_STATE_RUN  = 1
    REFLOW_STATE_COOL = 2
    REFLOW_STATES = (REFLOW_STATE_WAIT,
                     REFLOW_STATE_RUN,
                     REFLOW_STATE_COOL)
    
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
        return 'ReflowOven(timeout={},retries={},port={})'.format(self.timeout, self.retries, repr(self.oven))

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
            print('Error: Failed to connect on port {} with baud rate {} got exception {}.'.format(port, baudrate, e))
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
        packet = bytes()
        try:
            packet = self.oven.read_until(bytes([0x00]))
##            print('>'+str(packet))
        except Exception as e:
            print('Error: exception encountered on serial.read_until {}.'.format(e))
        if packet:
            data = cobs.decode(packet[:-1])
##            print('>'+str(list(data)))
            return True, data
        return False, bytes()

    def transmit(self, data):
        success = False
        try:
##            print('<'+str(list(data)))
            packet = cobs.encode(data)+bytes([0])
##            print('<'+str(packet))
            len_sent = self.oven.write(packet)
            if len(packet) == len_sent:
                success = True
            else:
                print('Error: Failed to transmit {} of {} bytes.'.format(len(packet) - len_sent, len(packet)))
        except Exception as e:
            print('Error: Failed to transmit got exception {}.'.format(e))
        return success        

    def communicate(self, comms_type, data=bytes()):
        success = False
        errors = 0
        response = bytes()
        if not comms_type in self.COMMS_TYPES:
            raise Exception('Argument comms_type must be a valid packet type.')
        expected_length = self.COMMS_EXPECTED_LENGTHS[self.COMMS_TYPES.index(comms_type)]
        for attempt in range(self.retries):
            if not self.transmit(bytes([comms_type]) + bytes(data)):
                print('Warning: ({}/{}) transmission failed, reconnecting.'.format(attempt+1, self.retries))
                self.reconnect()
                continue
            recv_success, packet = self.receive()
            if not recv_success:
                raise Exception('Timed out waiting on response to {} {}.'.format(comms_type, data))
            if not packet:
                raise Exception('Received an empty response to {} {}.'.format(comms_type, data))
            if packet[0] == comms_type:
                if len(packet) == expected_length:
                    success = True
                    errors = (packet[1] << 8) | packet[2]
                    response = packet[3:]
                    break
                else:
                    raise Exception('Received unexpected response length: expected {} actual {} packet {} tx type {} tx data {}'.format(expected_length, len(packet), packet, comms_type, data))
            elif packet[0] == self.COMMS_TYPE_NACK:
                print('Warning: ({}/{}) NACK received in response to {} {}.'.format(attempt+1, self.retries, comms_type, data))
                continue
            else:
                raise Exception('Received unexpected response type ({}) to {} {}.'.format(packet, comms_type, data))
        return success, errors, response

    def wait(self):
        success, errors, _ = self.communicate(self.COMMS_TYPE_CMD_WAIT)
        return success, errors

    def run(self, profile_filename):
        success, errors, _ = self.communicate(self.COMMS_TYPE_CMD_RUN, profile_load_packed(profile_filename))
        return success, errors
        
    def cool(self):
        success, errors, _ = self.communicate(self.COMMS_TYPE_CMD_COOL)
        return success, errors

    def get_state(self):
        state = 0
        success, errors, response = self.communicate(self.COMMS_TYPE_QUERY_STATE)
        if success:
            state = response[0]
        return success, errors, state

    def get_temp_tc(self):
        temp_oven = 0.0
        temp_ref = 0.0
        success, errors, response = self.communicate(self.COMMS_TYPE_QUERY_TPOVN)
        if success:
            temp_oven, temp_ref = struct.unpack('ff', response)
        return success, errors, temp_oven, temp_ref

    def get_temp_cpu(self):
        temp_cpu = 0.0
        success, errors, response = self.communicate(self.COMMS_TYPE_QUERY_TPCPU)
        if success:
            temp_cpu, = struct.unpack('f', response)
        return success, errors, temp_cpu

    def state2str(self, state):
        if state == self.REFLOW_STATE_WAIT:
            return 'WAITING'
        if state == self.REFLOW_STATE_RUN:
            return 'REFLOWING'
        if state == self.REFLOW_STATE_COOL:
            return 'COOLING'
        return ''

    def error2str(self, error):
        error_str = []
        if (error & self.ERROR_TC_MASK) == self.ERROR_TC_COMMS:
            error_str.append('TC COMMS ERROR')
        if (error & self.ERROR_TC_MASK) == self.ERROR_TC_FAULT:
            error_str.append('TC UNKNOWN FAULT')
        if (error & self.ERROR_TC_MASK) == self.ERROR_TC_OPEN:
            error_str.append('TC UNCONNECTED')
        if (error & self.ERROR_TC_MASK) == self.ERROR_TC_SHORT_GND:
            error_str.append('TC SHORT TO GND')
        if (error & self.ERROR_TC_MASK) == self.ERROR_TC_SHORT_VCC:
            error_str.append('TC SHORT TO VCC')
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
    if not oven.connect(port, baudrate):
        sys.exit(0)
    
    success, _, state = oven.get_state()
    assert(success)
    success, _, temp_oven, temp_ref = oven.get_temp_tc()
    assert(success)
    success, errors, temp_cpu = oven.get_temp_cpu()
    assert(success)
    print('{} Temperatures (ovn/ref/cpu): {} {} {} Errors: {}'.format(
        oven.state2str(state),
        temp_oven,
        temp_ref,
        temp_cpu,
        oven.error2str(errors)
        ))

    if state != oven.REFLOW_STATE_RUN:
        success, _ = oven.run(profile_filename)
        assert(success)
        success, _, state = oven.get_state()
        assert(success)

    while state == oven.REFLOW_STATE_RUN:
        success, _, state = oven.get_state()
        assert(success)
        success, _, temp_oven, temp_ref = oven.get_temp_tc()
        assert(success)
        success, errors, temp_cpu = oven.get_temp_cpu()
        assert(success)
        print('{} Temperatures (ovn/ref/cpu): {} {} {} Errors: {}'.format(
            oven.state2str(state),
            temp_oven,
            temp_ref,
            temp_cpu,
            oven.error2str(errors)
            ))
        time.sleep(0.5)

    print('Reflow completed!')
