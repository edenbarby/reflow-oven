import serial, serial.tools.list_ports, sys, re, struct, time
import cobs

class ReflowOven:
    """
    All transmissions are big endian.
    Serial Examples:
    1. Start reflowing:
    Reflow | <--[TYPE_QUERY ][QUERY_STATE                    ]--  | PC
    Oven   |  --[TYPE_RESP  ][STATE_WAIT                     ]--> | # Confirming reflow oven ready to receive a new profile.
           | <--[TYPE_REFLOW][...Serialised Reflow Profile...]--  | # Sending new profile to being immediately
           |  --[TYPE_ACK   ]-->                                  | # Acknowledging receipt and confirming correctness of new profile
    """
    TYPE_ACK = 0x00
    TYPE_NACK = 0x01
    TYPE_QUERY = 0x10
    TYPE_RESP = 0x11
    TYPE_WAIT = 0x20
    TYPE_REFLOW = 0x21
    TYPES = (TYPE_ACK, TYPE_NACK, TYPE_QUERY, TYPE_RESP, TYPE_WAIT, TYPE_REFLOW)

    QUERY_STATE = 0x00
    QUERY_TPOVN = 0x10
    QUERY_TPCPU = 0x11
    QUERIES = (QUERY_STATE, QUERY_TPOVN, QUERY_TPCPU)

    STATE_WAIT = 0x00
    STATE_REFLOW = 0x01
    STATES = (STATE_WAIT, STATE_REFLOW)

    TPOVN_OK = 0x00
    TPOVN_ERROR_COMMS = 0x01
    TPOVN_FAULT = 0x02
    TPOVN_FAULT_OPEN = 0x03
    TPOVN_FAULT_SHORT_GND = 0x04
    TPOVN_FAULT_SHORT_VCC = 0x05
    
    def __init__(self, timeout=0.1):
        self.timeout = timeout
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
        return 'ReflowOven(port=\'{}\',baud={},timeout={})'.format(self.port, self.baud, self.timeout)

    def connect(self, port, baudrate):
        try:
            if self.oven.port != port:
                self.oven.port = port
            if self.oven.baudrate != baudrate:
                self.oven.baudrate = baudrate
            if not self.oven.is_open:
                self.oven.open()
            return True
        except Exception as e:
            print('Error: Connect failed on port {} with baud rate {} got exception {}.'.format(self.port, self.baud, e))
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
        packet = self.oven.read_until(bytes([0x00]))
##        print('>'+str(packet))
        return cobs.decode(packet[:-1])

    def transmit(self, data):
        success = False
        try:
            packet = cobs.encode(data)+bytes([0])
##            print('<'+str(packet))
            len_sent = self.oven.write(packet)
            if len(packet) == len_sent:
                success = True
            else:
                print('Error: Transmitted {} of {} bytes.'.format(len_sent, len(data)))
        except Exception as e:
            print('Error: Failed to write got exception {}.'.format(e))
        return success

    def query(self, query, expected_length=0):
        response = bytes()
        if query in self.QUERIES:
            if self.transmit([self.TYPE_QUERY, query]):
                packet = self.receive()
                if (expected_length == 0 and len(packet) > 0) or (expected_length > 0 and expected_length == len(packet)):
                    if packet[0] == self.TYPE_RESP:
                        response = packet[1:]
                    else:
                        print('Error: Received incorrect packet type ({}) in response to a query.'.format(packet[0]))
                        print(packet)
                else:
                    print('Error: Received a packet of unexpected length (got {} expected {}).'.format(len(packet), expected_length))
                    print(packet)
            else:
                print('Error: Transmission failed.')
        else:
            print('Error: query() called with incorrect query type ({}).'.format(query))
        return response

    def query_state(self):
        state = -1
        resp = self.query(self.QUERY_STATE, 2)
        if len(resp) > 0 and resp[0] in self.STATES:
            state = resp[0]
        return state

    def query_temp_oven(self):
        resp = self.query(self.QUERY_TPOVN, 6)
        status, temp_tc, temp_ref = struct.unpack('>B2h', resp)
        return status, float(temp_tc)/4, float(temp_ref)/16

    def query_temp_cpu(self):
        resp = self.query(self.QUERY_TPCPU)
        return struct.unpack('>h', resp)[0]

    def reflow(self, profile_filename):
        success = False
        state = self.query_state()
        if state == self.STATE_WAIT:
            success = self.transmit(bytes([self.TYPE_REFLOW]) + profile_serialise(profile_filename))
        elif state == self.STATE_REFLOW:
            print('Warning: Unable to being reflow, as reflow is already in progress.')
        else:
            print('Error: Receive an unrecognised state ({}) from query_state.'.format(state))
        return success

    def wait(self):
        return self.transmit([self.TYPE_WAIT])

    def state2str(self, state):
        if state == self.STATE_WAIT:
            return 'WAITING'
        if state == self.STATE_REFLOW:
            return 'REFLOWING'
        return ''

    def state2str_ovn(self, state):
        if state == self.TPOVN_OK:
            return 'OK'
        if state == self.TPOVN_ERROR_COMMS:
            return 'COMMS ERROR'
        if state == self.TPOVN_FAULT:
            return 'UNKOWN FAULT'
        if state == self.TPOVN_FAULT_OPEN:
            return 'TC UNCONNECTED'
        if state == self.TPOVN_FAULT_SHORT_GND:
            return 'SHORT TO GND'
        if state == self.TPOVN_FAULT_SHORT_VCC:
            return 'SHORT TO VCC'
        return ''

def profile_serialise(filename):
    profile = bytes()
    with open(filename, 'r') as f:
        # First line only contains column labels, ditch it.
        f.readline()
        # Read the rest, split it, discard empty strings and convert it
        # all to ints.
        profile = [int(i) for i in re.split('[\n\r, ]', f.read()) if i]
        # Now pack it all into big endian ('>') signed shorts ('h', 2
        # bytes). Note struct.pack() supports a while bunch of data
        # types, where as int.to_bytes() only supports ints.
        profile = struct.pack('>{}h'.format(len(profile)), *profile)
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
    profile_filename = 'reflow-profile-1.csv'

    oven = ReflowOven()
    if not oven.connect(port, baudrate):
        sys.exit(0)
    
    state = oven.query_state()
    temp_oven_status, temp_oven_tc, temp_oven_ref = oven.query_temp_oven()
    temp_cpu = oven.query_temp_cpu()
    print('{} Temperatures (ovn/ref/cpu): {} {} ({}) {}'.format(
        oven.state2str(state),
        temp_oven_tc,
        temp_oven_ref,
        oven.state2str_ovn(temp_oven_status),
        temp_cpu
        ))

    if state == oven.STATE_WAIT:
        if not oven.reflow(profile_filename):
            sys.exit(0)
        state = oven.query_state()

    while state == oven.STATE_REFLOW:
        state = oven.query_state()
        temp_oven_status, temp_oven_tc, temp_oven_ref = oven.query_temp_oven()
        temp_cpu = oven.query_temp_cpu()
        print('{} Temperatures (ovn/ref/cpu): {} {} ({}) {}'.format(
            oven.state2str(state),
            temp_oven_tc,
            temp_oven_ref,
            oven.state2str_ovn(temp_oven_status),
            temp_cpu
            ))
        time.sleep(0.5)

    print('Reflow completed!')
