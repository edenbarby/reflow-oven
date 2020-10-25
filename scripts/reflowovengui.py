import matplotlib.backends.backend_tkagg
import matplotlib.pyplot
import numpy
import time
import tkinter as tk
import tkinter.filedialog
import reflowoven

class ReflowOvenApp(tk.Frame):
    BAUDRATES = (9600, 115200)
    DEFAULT_BAUDRATE = 115200
    DEFAULT_PORT = 'COM3'
    
    def __init__(self, parent):
        self.parent = parent
        super().__init__(self.parent)
        self.parent.title('Reflow Oven Manager')
        self.pack()

        self.str_port_desc = tk.StringVar(self, '')
        self.str_baudrate = tk.StringVar(self, self.DEFAULT_BAUDRATE)
        self.str_profile = tk.StringVar(self, '')
        self.str_temp_oven = tk.StringVar(self, 'N/A')
        self.str_temp_target = tk.StringVar(self, 'N/A')
        self.str_temp_ref = tk.StringVar(self, 'N/A')
        self.str_temp_cpu = tk.StringVar(self, 'N/A')
        self.str_eta = tk.StringVar(self, 'N/A')
        self.oven = reflowoven.ReflowOven(timeout=0.1)
        self.reflow_start_time = time.time()
        self.plot_target_time = []
        self.plot_target_temp = []
        self.plot_meas_time = []
        self.plot_meas_temp = []

        ## Menu Bar ##
        menu = tk.Menu(self.parent)
        self.parent['menu'] = menu
        
        # File menu
        menu_file = tk.Menu(menu)
        menu_file.add_command(label='Open', command=self.open)
        menu.add_cascade(label='File', menu=menu_file)

        ## Control Panel ##
        control_panel = tk.Frame(self)
        control_panel.pack(side=tk.LEFT)
        
        # Serial port selection menu.
        serial_devices = reflowoven.available_serial_ports()
        if serial_devices:
            if self.DEFAULT_PORT in serial_devices.keys():
                self.str_port_desc.set(serial_devices[self.DEFAULT_PORT])
            else:
                self.str_port_desc.set(next(iter(serial_devices.values())))
        else:
            self.str_port_desc.set('NO SERIAL DEVICES FOUND')
        
        label_port = tk.Label(control_panel, text='PORT')
        label_port.grid(column=0, row=0)
        self.menu_port = tk.OptionMenu(control_panel, self.str_port_desc, *serial_devices.values())
        self.menu_port.grid(column=1, row=0)
        button_port = tk.Button(control_panel, command=self.refresh_ports, text='REFRESH')
        button_port.grid(column=2, row=0)
        
        # BUAD rate selection menu.
        label_baud = tk.Label(control_panel, text='BAUD RATE')
        label_baud.grid(column=0, row=1)
        menu_baud = tk.OptionMenu(control_panel, self.str_baudrate, *self.BAUDRATES)
        menu_baud.grid(column=1, row=1)
        
        # Connect/disconnect button.
        self.button_connect = tk.Button(control_panel, command=self.connect, text='CONNECT')
        self.button_connect.grid(column=0, row=2)
        
        # Start/stop button.
        self.button_start = tk.Button(control_panel, command=self.start, state='disabled', text='START')
        self.button_start.grid(column=1, row=2)
        
        # Oven status display label.
        self.label_state = tk.Label(control_panel, bg='red', text='DISCONNECTED')
        self.label_state.grid(column=0, row=3)

        # Profile filename display label.
        label_profile = tk.Label(control_panel, textvariable=self.str_profile)
        label_profile.grid(column=1, row=3)
        
        # Oven temperature display label.
        tk.Label(control_panel, text='OVEN TEMPERATURE (\u00b0C)').grid(column=0, row=4)
        label_temp_oven = tk.Label(control_panel, textvariable=self.str_temp_oven)
        label_temp_oven.grid(column=1, row=4)
        
        # Target temperature display label.
        tk.Label(control_panel, text='OVEN TEMPERATURE TARGET (\u00b0C)').grid(column=0, row=5)
        label_temp_target = tk.Label(control_panel, textvariable=self.str_temp_target)
        label_temp_target.grid(column=1, row=5)
        
        # Thermocouple IC reference temperature display label.
        tk.Label(control_panel, text='COLD JUNCTION TEMPERATURE (\u00b0C)').grid(column=0, row=6)
        label_temp_ref = tk.Label(control_panel, textvariable=self.str_temp_ref)
        label_temp_ref.grid(column=1, row=6)
        
        # CPU temperature display label.
        tk.Label(control_panel, text='CPU TEMPERATURE (\u00b0C)').grid(column=0, row=7)
        label_temp_cpu = tk.Label(control_panel, textvariable=self.str_temp_cpu)
        label_temp_cpu.grid(column=1, row=7)
        
        # Completion time estimate display label.
        tk.Label(control_panel, text='ESTIMATED TIME OF COMPLETION').grid(column=0, row=8)
        label_temp_cpu = tk.Label(control_panel, textvariable=self.str_eta)
        label_temp_cpu.grid(column=1, row=8)

        ## Temperature Graph ##
        frame_graph = tk.Frame(self)
        frame_graph.pack(side=tk.LEFT)
        
        figure, self.plot_axis = matplotlib.pyplot.subplots()
        self.canvas = matplotlib.backends.backend_tkagg.FigureCanvasTkAgg(figure, master=frame_graph)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH)

    def refresh_ports(self):
        menu = self.menu_port['menu']
        menu.delete(0, 'end')
        serial_devices = reflowoven.available_serial_ports()
        for description in serial_devices.values():
            menu.add_command(label=description,
                             command=lambda value=description: self.str_port_desc.set(value))
        if not self.str_port_desc.get() in serial_devices.values():
            self.str_port_desc.set('')

    def open(self):
        file_types = [('Comma Seperated Values', '*.csv'), ('All Files', '*')]
        filename = tk.filedialog.askopenfilename(filetypes=file_types)
        self.str_profile.set(filename)
        profile = reflowoven.profile_load(filename)
        self.plot_target_time = [time for time,_ in profile]
        self.plot_target_temp = [temp for _,temp in profile]
        self.update_plot()

    def connect(self):
        action = self.button_connect['text']
        if action == 'CONNECT':
            port_desc = self.str_port_desc.get()
            serial_devices = reflowoven.available_serial_ports()
            for port in serial_devices:
                if serial_devices[port] == port_desc:
                    break
            baudrate = int(self.str_baudrate.get())
            if port and baudrate > 0:
                if self.oven.connect(port, baudrate):
                    print('Connection successful.')
                else:
                    print('Error: Failed to connect to reflow oven.')
            else:
                print('Error: Invalid serial port ({}) and/or baudrate ({}) selected.'.format(repr(port), baudrate))
        elif action == 'DISCONNECT':
            self.oven.disconnect()
        else:
            raise Exception('Unexpected connect button text encountered ({}).'.format(repr(action)))
        self.update()

    def start(self):
        action = self.button_start['text']
        if action == 'START':
            print('starting')
            profile_filename = self.str_profile.get()
            if profile_filename:
                self.oven.reflow(profile_filename)
                self.reflow_start_time = time.time()
                self.plot_meas_time = []
                self.plot_meas_temp = []
                self.update_plot()
            else:
                print('Error: No reflow profile specified.')
        elif action == 'STOP':
            print('stopping')
            self.oven.wait()
        else:
            raise Exception('Unexpected start button text encountered ({}).'.format(repr(action)))

    def update(self):
        state = self.oven.query_state()        
        if state == -1: # Not connected.
            self.button_connect['text'] = 'CONNECT'
            self.button_start['state'] = 'disabled'
            self.button_start['text'] = 'START'
            self.label_state['bg'] = 'red'
            self.label_state['text'] = 'DISCONNECTED'
            self.str_temp_oven.set('N/A')
            self.str_temp_target.set('N/A')
            self.str_temp_ref.set('N/A')
            self.str_temp_cpu.set('N/A')
            self.str_eta.set('N/A')
        elif state == self.oven.STATE_WAIT or state == self.oven.STATE_REFLOW:
            state_label = self.oven.state2str(state)
            tc_state, temp_oven, temp_ref = self.oven.query_temp_oven()
            if tc_state == self.oven.TPOVN_ERROR_COMMS:
                temp_oven = 'THERMOCOUPLE CHIP COMMS ERROR'
                temp_ref = 'THERMOCOUPLE CHIP COMMS ERROR'
            if tc_state == self.oven.TPOVN_FAULT:
                temp_oven = 'UNKNOWN THERMOCOUPLE CHIP FAULT'
                temp_ref = 'UNKNOWN THERMOCOUPLE CHIP FAULT'
            if tc_state == self.oven.TPOVN_FAULT_OPEN:
                temp_oven = 'THERMOCOUPLE DISCONNECTED'
            if tc_state == self.oven.TPOVN_FAULT_SHORT_GND:
                temp_oven = 'THERMOCOUPLE SHORTED TO GROUND'
            if tc_state == self.oven.TPOVN_FAULT_SHORT_VCC:
                temp_oven = 'THERMOCOUPLE SHORTED TO POWER'
            temp_cpu = self.oven.query_temp_cpu()

            if state == self.oven.STATE_REFLOW:
                self.plot_meas_time.append(time.time() - self.reflow_start_time)
                self.plot_meas_temp.append(temp_ref)
                self.update_plot()
            
            self.button_connect['text'] = 'DISCONNECT'
            self.button_start['state'] = 'normal'
            self.label_state['bg'] = 'green'
            self.label_state['text'] = state_label
            self.str_temp_oven.set(temp_oven)
            self.str_temp_target.set('N/A')
            self.str_temp_ref.set(temp_ref)
            self.str_temp_cpu.set(temp_cpu)
            self.str_eta.set('N/A')
            if state == self.oven.STATE_WAIT:
                self.button_start['text'] = 'START'
            else:
                self.button_start['text'] = 'STOP'
            self.after(500, self.update)
        else:
            print('Error: Unknown state ({}) encountered, disconnecting.'.format(state))
            self.oven.disconnect()
            self.button_connect['text'] = 'CONNECT'
            self.button_start['state'] = 'disabled'
            self.button_start['text'] = 'START'
            self.label_state['bg'] = 'red'
            self.label_state['text'] = 'DISCONNECTED'
            self.str_temp_oven.set('N/A')
            self.str_temp_target.set('N/A')
            self.str_temp_ref.set('N/A')
            self.str_temp_cpu.set('N/A')
            self.str_eta.set('N/A')

    def update_plot(self):
        self.plot_axis.clear()
        if self.plot_target_time and self.plot_target_temp:
            x = numpy.asarray(self.plot_target_time)
            y = numpy.asarray(self.plot_target_temp)
            self.plot_axis.plot(x, y)
        if self.plot_meas_time and self.plot_meas_temp:
            x = numpy.asarray(self.plot_meas_time)
            y = numpy.asarray(self.plot_meas_temp)
            self.plot_axis.plot(x, y)
        self.canvas.draw()

if __name__ == '__main__':
    root = tk.Tk()
    app = ReflowOvenApp(root)
    app.mainloop()
