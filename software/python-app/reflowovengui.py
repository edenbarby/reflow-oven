import matplotlib.backends.backend_tkagg
import matplotlib.pyplot
import numpy
import time
import tkinter as tk
import tkinter.filedialog
import reflowoven

def is_float(a):
    try:
        float(a)
    except:
        return False
    return True

class ReflowOvenApp(tk.Frame):
    BAUDRATES = (9600, 115200)
    DEFAULT_BAUDRATE = 115200
    DEFAULT_PORT = 'COM3'
    
    def __init__(self, parent):
        self.parent = parent
        super().__init__(self.parent)
        self.parent.title('Reflow Oven Manager')
        self.pack()

        self.str_port = tk.StringVar(self, '')
        self.str_baud = tk.StringVar(self, self.DEFAULT_BAUDRATE)
        self.oven = reflowoven.ReflowOven()
        self.reflow_start_time = time.time()
        self.plot_time = []
        self.plot_temp = []
        self.plot_p = []
        self.plot_i = []
        self.plot_d = []
        self.plot_power = []

        # ## Menu Bar ##
        # menu = tk.Menu(self.parent)
        # self.parent['menu'] = menu
        
        # # File menu
        # menu_file = tk.Menu(menu)
        # menu_file.add_command(label='Open', command=self.open)
        # menu.add_cascade(label='File', menu=menu_file)

        ## Control Panel ##
        control_panel = tk.Frame(self)
        control_panel.pack(side=tk.LEFT)
        control_panel_row = 0
        
        # Serial port selection menu.
        serial_devices = reflowoven.available_serial_ports()
        if serial_devices:
            if self.DEFAULT_PORT in serial_devices.keys():
                self.str_port.set(serial_devices[self.DEFAULT_PORT])
            else:
                self.str_port.set(next(iter(serial_devices.values())))
        else:
            self.str_port.set('NO SERIAL DEVICES FOUND')
        
        tk.Label(control_panel, text='PORT').grid(column=0, row=control_panel_row)
        self.menu_port = tk.OptionMenu(control_panel, self.str_port, *serial_devices.values())
        self.menu_port.grid(column=1, row=control_panel_row)
        tk.Button(control_panel, command=self.refresh_ports, text='REFRESH').grid(column=2, row=control_panel_row)
        control_panel_row += 1
        
        # BUAD rate selection menu.
        tk.Label(control_panel, text='BAUD RATE').grid(column=0, row=control_panel_row)
        tk.OptionMenu(control_panel, self.str_baud, *self.BAUDRATES).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        
        # Connect/disconnect button.
        self.button_connect = tk.Button(control_panel, command=self.connect, text='CONNECT')
        self.button_connect.grid(column=0, row=control_panel_row)
        
        # Oven status display label.
        self.label_state = tk.Label(control_panel, bg='red', text='DISCONNECTED')
        self.label_state.grid(column=1, row=control_panel_row)
        
        # Start/stop button.
        self.button_start = tk.Button(control_panel, command=self.start, state='disabled', text='START')
        self.button_start.grid(column=2, row=control_panel_row)
        control_panel_row += 1

        # Reflow profile settings.
        def entry_init(label):
            nonlocal control_panel_row
            nonlocal control_panel
            tk.Label(control_panel, text=label).grid(column=0, row=control_panel_row)
            string_var = tk.StringVar(self, 'N/A')
            tk.Label(control_panel, textvariable=string_var).grid(column=1, row=control_panel_row)
            entry = tk.Entry(control_panel)
            entry.grid(column=2, row=control_panel_row)
            control_panel_row += 1
            return string_var, entry
        
        self.str_ramp_rate,   self.entry_ramp_rate   = entry_init('Ramp Rate (\u00b0C/s)')
        self.str_soak_time,   self.entry_soak_time   = entry_init('Soak Time (s)')
        self.str_soak_temp,   self.entry_soak_temp   = entry_init('Soak Temp (\u00b0C)')
        self.str_reflow_time, self.entry_reflow_time = entry_init('Reflow Time (s)')
        self.str_reflow_temp, self.entry_reflow_temp = entry_init('Reflow Temp (\u00b0C)')
        self.str_ramp_p,      self.entry_ramp_p      = entry_init('Ramping P Term')
        self.str_ramp_i,      self.entry_ramp_i      = entry_init('Ramping I Term')
        self.str_ramp_d,      self.entry_ramp_d      = entry_init('Ramping D Term')
        self.str_ramp_i_max,  self.entry_ramp_i_max  = entry_init('Ramping Max I Term')
        self.str_fixed_p,     self.entry_fixed_p     = entry_init('Fixed P Term')
        self.str_fixed_i,     self.entry_fixed_i     = entry_init('Fixed I Term')
        self.str_fixed_d,     self.entry_fixed_d     = entry_init('Fixed D Term')
        self.str_fixed_i_max, self.entry_fixed_i_max = entry_init('Fixed Max I Term')

        # Reflow profile state.
        self.str_current_p    = tk.StringVar(self, 'N/A')
        self.str_current_i    = tk.StringVar(self, 'N/A')
        self.str_current_d    = tk.StringVar(self, 'N/A')
        self.str_current_pow  = tk.StringVar(self, 'N/A')
        tk.Label(control_panel, text='Controller P Term'          ).grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_current_p   ).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        tk.Label(control_panel, text='Controller I Term'          ).grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_current_i   ).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        tk.Label(control_panel, text='Controller D Term'          ).grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_current_d   ).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        tk.Label(control_panel, text='Heating Element On Time (%)').grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_current_pow ).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        
        # Oven temperature display label.
        self.str_temp_oven = tk.StringVar(self, 'N/A')
        tk.Label(control_panel, text='Oven Temp (\u00b0C)').grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_temp_oven).grid(column=1, row=control_panel_row)
        control_panel_row += 1
        
        # Board temperature display label.
        self.str_temp_ref = tk.StringVar(self, 'N/A')
        tk.Label(control_panel, text='Board Temp (\u00b0C)').grid(column=0, row=control_panel_row)
        tk.Label(control_panel, textvariable=self.str_temp_ref).grid(column=1, row=control_panel_row)
        control_panel_row += 1

        # Error display.
        self.error_display = tk.Text(control_panel)
        self.error_display.grid(column=0, columnspan=3, row=control_panel_row)
        control_panel_row += 1

        ## Temperature Graph ##
        frame_graph = tk.Frame(self)
        frame_graph.pack(side=tk.LEFT)
        figure, self.plot_axis = matplotlib.pyplot.subplots(4)
        self.canvas = matplotlib.backends.backend_tkagg.FigureCanvasTkAgg(figure, master=frame_graph)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH)

    def refresh_ports(self):
        menu = self.menu_port['menu']
        menu.delete(0, 'end')
        serial_devices = reflowoven.available_serial_ports()
        for description in serial_devices.values():
            menu.add_command(label=description,
                             command=lambda value=description: self.str_port.set(value))
        if not self.str_port.get() in serial_devices.values():
            self.str_port.set('')

    # def open(self):
    #     file_types = [('Comma Seperated Values', '*.csv'), ('All Files', '*')]
    #     filename = tk.filedialog.askopenfilename(filetypes=file_types)
    #     self.str_profile.set(filename)
    #     profile = reflowoven.profile_load(filename)
    #     self.plot_target_time = [time for time,_ in profile]
    #     self.plot_target_temp = [temp for _,temp in profile]
    #     self.update_plot()

    def connect(self):
        action = self.button_connect['text']
        if action == 'CONNECT':
            desc_target = self.str_port.get()
            for port, desc in reflowoven.available_serial_ports().items():
                if desc == desc_target:
                    baudrate = int(self.str_baud.get())
                    if port and baudrate > 0:
                        if self.oven.connect(port, baudrate):
                            print('Connection successful.')
                        else:
                            print('Error: Failed to connect to reflow oven.')
                    else:
                        print('Error: Invalid serial port ({}) and/or baudrate ({}) selected.'.format(repr(port), baudrate))
                    break
            if desc != desc_target:
                print('Error: unable to find device matching description {}.'.format(desc_target))
        elif action == 'DISCONNECT':
            self.oven.disconnect()
        else:
            raise Exception('Unexpected connect button text encountered ({}).'.format(repr(action)))
        self.update()

    def start(self):
        action = self.button_start['text']
        if action == 'START':
            def get(string_var, entry):
                value = 0.0
                if is_float(entry.get()):
                    value = float(entry.get())
                elif is_float(string_var.get()):
                    value = float(string_var.get())
                return value

            ramp_rate   = get(self.str_ramp_rate,   self.entry_ramp_rate)
            soak_time   = get(self.str_soak_time,   self.entry_soak_time)
            soak_temp   = get(self.str_soak_temp,   self.entry_soak_temp)
            reflow_time = get(self.str_reflow_time, self.entry_reflow_time)
            reflow_temp = get(self.str_reflow_temp, self.entry_reflow_temp)
            ramp_p      = get(self.str_ramp_p,      self.entry_ramp_p)
            ramp_i      = get(self.str_ramp_i,      self.entry_ramp_i)
            ramp_d      = get(self.str_ramp_d,      self.entry_ramp_d)
            ramp_i_max  = get(self.str_ramp_i_max,  self.entry_ramp_i_max)
            fixed_p     = get(self.str_fixed_p,     self.entry_fixed_p)
            fixed_i     = get(self.str_fixed_i,     self.entry_fixed_i)
            fixed_d     = get(self.str_fixed_d,     self.entry_fixed_d)
            fixed_i_max = get(self.str_fixed_i_max, self.entry_fixed_i_max)

            reflow_profile = self.oven.ReflowProfile(ramp_rate, soak_time, soak_temp,
                                              reflow_time, reflow_temp, ramp_p,
                                              ramp_i, ramp_d, ramp_i_max, fixed_p,
                                              fixed_i, fixed_d, fixed_i_max)
            self.oven.cmd_run(reflow_profile)

            self.reflow_start_time = time.time()
            self.plot_time  = []
            self.plot_temp  = []
            self.plot_p     = []
            self.plot_i     = []
            self.plot_d     = []
            self.plot_power = []
            self.update_plot()
        elif action == 'STOP':
            self.oven.cmd_idle()
        else:
            raise Exception(f'Unexpected start button text encountered ({action}).')

    def update(self):
        success, oven_state = self.oven.query_all()
        if success == False: # Not connected.
            self.oven.disconnect()
            self.button_connect['text'] = 'CONNECT'
            self.button_start['state'] = 'disabled'
            self.button_start['text'] = 'START'
            self.label_state['bg'] = 'red'
            self.label_state['text'] = 'DISCONNECTED'
            self.str_ramp_rate.set('N/A')
            self.str_soak_time.set('N/A')
            self.str_soak_temp.set('N/A')
            self.str_reflow_time.set('N/A')
            self.str_reflow_temp.set('N/A')
            self.str_ramp_p.set('N/A')
            self.str_ramp_i.set('N/A')
            self.str_ramp_d.set('N/A')
            self.str_ramp_i_max.set('N/A')
            self.str_fixed_p.set('N/A')
            self.str_fixed_i.set('N/A')
            self.str_fixed_d.set('N/A')
            self.str_fixed_i_max.set('N/A')
            self.str_current_p = tk.StringVar(self, 'N/A')
            self.str_current_i = tk.StringVar(self, 'N/A')
            self.str_current_d = tk.StringVar(self, 'N/A')
            self.str_current_pow = tk.StringVar(self, 'N/A')
            self.str_temp_oven.set('N/A')
            self.str_temp_ref.set('N/A')
        else:
            error_text = '\n'.join(self.oven.error2str(oven_state.errors))
            self.error_display.delete('1.0', tk.END)
            self.error_display.insert(tk.INSERT, error_text)

            temp_oven = str(oven_state.temp_oven)
            temp_ref  = str(oven_state.temp_ref)
            if ((oven_state.errors & self.oven.ERROR_TC_COM) or
                    (oven_state.errors & self.oven.ERROR_TC_FLT)):
                temp_oven = 'ERROR'
                temp_ref  = 'ERROR'
            elif ((oven_state.errors & self.oven.ERROR_TC_OPN) or
                    (oven_state.errors & self.oven.ERROR_TC_GND) or
                    (oven_state.errors & self.oven.ERROR_TC_VCC)):
                temp_oven = 'ERROR'

            if (oven_state.state > self.oven.OVEN_STATE_IDLE) and is_float(temp_oven):
                self.plot_time.append(time.time() - self.reflow_start_time)
                self.plot_temp.append(oven_state.temp_oven)
                self.plot_p.append(oven_state.p_current)
                self.plot_i.append(oven_state.i_current)
                self.plot_d.append(oven_state.d_current)
                self.plot_power.append(oven_state.power_current)
                self.update_plot()
            
            
            self.button_connect['text'] = 'DISCONNECT'
            self.button_start['state'] = 'normal'
            if oven_state.state > self.oven.OVEN_STATE_IDLE:
                self.button_start['text'] = 'STOP'
            else:
                self.button_start['text'] = 'START'
            self.label_state['bg'] = 'green'
            self.label_state['text'] = self.oven.state2str(oven_state.state)
            self.str_ramp_rate.set(str(oven_state.ramp_rate))
            self.str_soak_time.set(str(oven_state.soak_time))
            self.str_soak_temp.set(str(oven_state.soak_temp))
            self.str_reflow_time.set(str(oven_state.reflow_time))
            self.str_reflow_temp.set(str(oven_state.reflow_temp))
            self.str_ramp_p.set(str(oven_state.ramp_p))
            self.str_ramp_i.set(str(oven_state.ramp_i))
            self.str_ramp_d.set(str(oven_state.ramp_d))
            self.str_ramp_i_max.set(str(oven_state.ramp_i_max))
            self.str_fixed_p.set(str(oven_state.fixed_p))
            self.str_fixed_i.set(str(oven_state.fixed_i))
            self.str_fixed_d.set(str(oven_state.fixed_d))
            self.str_fixed_i_max.set(str(oven_state.fixed_i_max))
            self.str_current_p.set(str(oven_state.p_current))
            self.str_current_i.set(str(oven_state.i_current))
            self.str_current_d.set(str(oven_state.d_current))
            self.str_current_pow.set(str(oven_state.power_current))
            self.str_temp_oven.set(temp_oven)
            self.str_temp_ref.set(temp_ref)

            self.after(500, self.update)

    def update_plot(self):
        self.plot_axis[0].clear()
        self.plot_axis[1].clear()
        self.plot_axis[2].clear()
        self.plot_axis[3].clear()
        self.plot_axis[0].plot(self.plot_time, self.plot_temp)
        self.plot_axis[1].plot(self.plot_time, self.plot_p)
        self.plot_axis[2].plot(self.plot_time, self.plot_i)
        self.plot_axis[3].plot(self.plot_time, self.plot_d)
        # self.plot_axis.plot(self.plot_time, self.plot_power)
        self.canvas.draw()

if __name__ == '__main__':
    root = tk.Tk()
    app = ReflowOvenApp(root)
    app.mainloop()