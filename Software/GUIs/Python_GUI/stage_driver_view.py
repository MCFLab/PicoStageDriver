"""Top-level GUI panel definition for the Stage Driver application.

Defines the `Panel` class which composes the smaller controls and
exposes convenience getters/setters for the controller layer. The
Panel wires button events into callback names using `bind_button` to
keep the View and Controller loosely coupled.
"""

import tkinter as tk
from tkinter import ttk
from input_history_panel import InputHistoryPanel
from status_panel import StatusPanel
from panel_controls import *



####################################

class Panel(tk.Tk):
    """Main application window composed of motor, parameter, motion
    and communication sub-panels.

    The controller instantiates this class and registers callbacks for
    the various buttons via `bind_button`.
    """

    def __init__(self):
        super().__init__()
        self.title('Stage Driver Test Panel')

        # define the fonts
        text_font = ('Helvetica',10)
        style_panelFont = ttk.Style()
        style_panelFont.configure("Custom.TLabelframe.Label", font=("Helvetica", 10, "bold"))

        # define the frames

        # motors frame
        label = ttk.Label(text="Motors", style="Custom.TLabelframe.Label")
        motors_frame = ttk.LabelFrame(self, labelwidget=label)
        self.motors_numMotors =  NumDisplay(motors_frame, 'Num Motors')
        self.motors_motorSelect =  NumCtrl(motors_frame, 'Motor Select', 0, 3, 1)
        self.motors_configButton = ttk.Button(motors_frame, text='Config')
        self.motors_clearButton = ttk.Button(motors_frame, text='Clear')
        self.motors_enableButton = ttk.Button(motors_frame, text='Enable')
        self.motors_disableButton = ttk.Button(motors_frame, text='Disable')
        self.motors_numMotors.pack(side='left', anchor='s', padx=10)
        self.motors_motorSelect.pack(side='left', anchor='s', padx=10)
        self.motors_configButton.pack(side='left', anchor='s', padx=10)
        self.motors_clearButton.pack(side='left', anchor='s', padx=10)
        self.motors_enableButton.pack(side='left', anchor='s', padx=10)
        self.motors_disableButton.pack(side='left', anchor='s', padx=10)

        # parameter frame
        label = ttk.Label(text="Parameters", style="Custom.TLabelframe.Label")
        
        
        param_frame = ttk.LabelFrame(self, labelwidget=label)
        
        self.CurrPanel = ParamPanel(param_frame, "Curr", \
                                  ['Scaler', 'Range', 'Run', 'Hold'])
        self.ModePanel = ParamPanel(param_frame, "Mode", \
                                  ['MicroStep', 'InvDir', 'TOff', 'SGEnable', 'SGT', 'TCT'])
        self.HomingPanel = ParamPanel(param_frame, "Homing", \
                                  ['Mode', 'Direction', 'Velocity', 'SoftStop', 'IndexEvent'])
        self.RatePanel = ParamPanel(param_frame, "Rate", \
                                  ['SetVelocity', 'MaxVelocity', 'SetAcc', 'MaxAcc'])
        self.SwitchPanel = ParamPanel(param_frame, "Switch", \
                                  ['LeftEnable', 'RightEnable', 'LeftPolarity', 'RightPolarity', 'Swap'])
        self.LimPanel = ParamPanel(param_frame, "Lim", \
                                  ['Encoder', 'LeftEnable', 'RightEnable', 'LeftPosition', 'RightPosition'])
        self.EncPanel = ParamPanel(param_frame, "Enc", \
                                  ['Constant', 'Deviation', 'LoopTolerance', 'LoopMax', 'ResetXafterCL'])
        params_buttonFrame = ttk.Frame(param_frame)
        self.params_getButton = ttk.Button(params_buttonFrame, text='Get', width=5)
        self.params_getButton.pack(side='left', anchor='s', padx=10)
        self.params_setButton = ttk.Button(params_buttonFrame, text='Set', width=5)
        self.params_setButton.pack(side='left', anchor='s', padx=10)
        self.params_fileNameLabel = ttk.Label(params_buttonFrame, text='File name:', font=text_font)
        self.params_fileNameLabel.pack(side='left', anchor='s', padx=(50,0))
        self.params_fileNameText = tk.StringVar()
        self.params_fileNameEntry = ttk.Entry(params_buttonFrame, textvariable=self.params_fileNameText, width=20)
        self.params_fileNameEntry.pack(side='left', anchor='s', padx=(0,10))
        self.params_fileNameText.set('MotorConfig.json')
        self.params_loadButton = ttk.Button(params_buttonFrame, text='Load', width=5)
        self.params_loadButton.pack(side='left', anchor='s', padx=10)
        self.params_saveButton = ttk.Button(params_buttonFrame, text='Save', width=5)
        self.params_saveButton.pack(side='left', anchor='s', padx=10)        
        params_buttonFrame.pack(anchor='w', pady=5)


        # motion frame
        label = ttk.Label(text="Motion", style="Custom.TLabelframe.Label")
        motion_frame = ttk.LabelFrame(self, labelwidget=label)
        motion_vel_frame = ttk.Frame(motion_frame)
        motion_vel_label = ttk.Label(motion_vel_frame, text='Velocity Mode:', font=text_font)
        self.motion_vel_number =  NumDisplay(motion_vel_frame, 'Velocity [%]')
        self.motion_vel_number.entry.bind("<Return>", self.vel_number_to_slider)
        self.motion_vel_slider =  SliderCtrl(motion_vel_frame, 'Velocity [%]',
                                              -100, 100,
                                              self.vel_slider_to_number)
        self.motion_vel_setButton = ttk.Button(motion_vel_frame, text='Set')
        self.motion_vel_stopButton = ttk.Button(motion_vel_frame, text='Stop', command=self.vel_stop_motion)
        motion_vel_label.pack(side='left', anchor='s', padx=10)
        self.motion_vel_slider.pack(side='left', anchor='s', padx=10)
        self.motion_vel_number.pack(side='left', anchor='s', padx=10)
        self.motion_vel_setButton.pack(side='left', anchor='s', padx=10)
        self.motion_vel_stopButton.pack(side='left', anchor='s', padx=10)
        motion_pos_frame = ttk.Frame(motion_frame)
        motion_pos_label = ttk.Label(motion_pos_frame, text='Position Mode:', font=text_font)
        self.motion_pos =  NumDisplay(motion_pos_frame, 'Position')
        self.motion_pos_setButton = ttk.Button(motion_pos_frame, text='Set')
        self.motion_setpos =  NumDisplay(motion_pos_frame, 'Set pos')
        self.motion_enc_setButton = ttk.Button(motion_pos_frame, text='Set enc')
        self.motion_all_setButton = ttk.Button(motion_pos_frame, text='Set all')
        motion_pos_label.pack(side='left', anchor='s', padx=10)
        self.motion_pos.pack(side='left', anchor='s', padx=10)
        self.motion_pos_setButton.pack(side='left', anchor='s', padx=10)
        self.motion_setpos.pack(side='left', anchor='s', padx=(40,10))
        self.motion_enc_setButton.pack(side='left', anchor='s', padx=10)
        self.motion_all_setButton.pack(side='left', anchor='s', padx=10)
        motion_vel_frame.pack(anchor='w', pady=5) 
        motion_pos_frame.pack(anchor='w', pady=5) 

        # remote frame
        label = ttk.Label(text="Remote", style="Custom.TLabelframe.Label")
        remote_frame = ttk.LabelFrame(self, labelwidget=label)
        self.remote_joystickMax =  NumDisplay(remote_frame, 'Joystick Max', default=10000)
        self.remote_joystickDir =  NumDisplay(remote_frame, 'Joystick Dir', default=1)
        self.remote_encoderStep =  NumDisplay(remote_frame, 'Encoder Step', default=10)
        self.remote_encoderDir =  NumDisplay(remote_frame, 'Encoder Dir', default=1)
        self.remote_getButton = ttk.Button(remote_frame, text='Get', width=5)
        self.remote_setButton = ttk.Button(remote_frame, text='Set', width=5)
        self.remote_enableButton = ttk.Button(remote_frame, text='Enable', width=8)
        self.remote_disableButton = ttk.Button(remote_frame, text='Disable', width=8)
        self.remote_joystickMax.pack(side='left', anchor='s', padx=10)
        self.remote_joystickDir.pack(side='left', anchor='s', padx=10)
        self.remote_encoderStep.pack(side='left', anchor='s', padx=10)
        self.remote_encoderDir.pack(side='left', anchor='s', padx=10)
        self.remote_getButton.pack(side='left', anchor='s', padx=10)
        self.remote_setButton.pack(side='left', anchor='s', padx=10)
        self.remote_enableButton.pack(side='left', anchor='s', padx=(20,10))
        self.remote_disableButton.pack(side='left', anchor='s', padx=10)

        # communication frame
        label = ttk.Label(text="Communication", style="Custom.TLabelframe.Label")
        comm_frame = ttk.LabelFrame(self, labelwidget=label)
        self.comm_checkErrorButton = ttk.Button(comm_frame, text='Check Err')
        self.comm_checkErrorButton.pack(anchor='w', pady=5)
        self.app = InputHistoryPanel(comm_frame)

        # status frame
        label = ttk.Label(text="Status", style="Custom.TLabelframe.Label")
        status_frame = ttk.LabelFrame(self, width=900, labelwidget=label)
        self.status_motors = StatusPanel(status_frame, interval=500, auto_update=1)
        self.status_motors.pack(anchor='w', padx=10)

        # assemble the whole panel
        motors_frame.grid(column=0, row=0, sticky = "W", padx=5, pady=5) 
        param_frame.grid(column=0, row=1, sticky = "W", padx=5, pady=5)
        motion_frame.grid(column=0, row=2, sticky = "W", padx=5, pady=5)
        status_frame.grid(column=1, row=0, rowspan=3, sticky = "NW", padx=5, pady=5)
        remote_frame.grid(column=0, row=3, columnspan=2, sticky = "W", padx=5, pady=5)
        comm_frame.grid(column=0, row=4, columnspan=2, sticky = "W", padx=5, pady=5)

    # set up callbacks and such
    def bind_button(self, button_name, func):
        """Bind a controller callable to a named button or control.

        The `button_name` values are the canonical names used by the
        Controller when wiring events.
        """
        if button_name == 'getParams':
            self.params_getButton.bind('<Button>', func)
        elif button_name == 'setParams':
            self.params_setButton.bind('<Button>', func)
        elif button_name == 'loadParams':
            self.params_loadButton.bind('<Button>', func)
        elif button_name == 'saveParams':
            self.params_saveButton.bind('<Button>', func)
        elif button_name == 'configMotor':
            self.motors_configButton.bind('<Button>', func)
        elif button_name == 'clearMotor':
            self.motors_clearButton.bind('<Button>', func)
        elif button_name == 'enableMotor':
            self.motors_enableButton.bind('<Button>', func)
        elif button_name == 'disableMotor':
            self.motors_disableButton.bind('<Button>', func)
        elif button_name == 'moveToPos':
            self.motion_pos.entry.bind("<Return>", func)
            self.motion_pos_setButton.bind('<Button>', func)
        elif button_name == 'setEncPositions':
            self.motion_enc_setButton.bind('<Button>', func)
        elif button_name == 'setAllPositions':
            self.motion_all_setButton.bind('<Button>', func)
        elif button_name == 'moveAtVel':
            self.motion_vel_setButton.bind('<Button>', func)
            self.update_velocity = func
        elif button_name == 'getRemote':
            self.remote_getButton.bind('<Button>', func)
        elif button_name == 'setRemote':
            self.remote_setButton.bind('<Button>', func)
        elif button_name == 'enableRemote':
            self.remote_enableButton.bind('<Button>', func)
        elif button_name == 'disableRemote':
            self.remote_disableButton.bind('<Button>', func)
        elif button_name == 'checkError':
            self.comm_checkErrorButton.bind('<Button>', func)

    def comm_associate(self, func):
        """Associate a callable that will receive submitted communication text."""
        self.app.on_submit = func

    def comm_print_line(self, text):
        """Print a line to the communication output area (prefixed with '>')."""
        self.app.append_output(f"> {text}")

    def status_motors_associate(self, func):
        """Associate a callable that will be invoked on status updates."""
        self.status_motors.on_update = func

    def status_motors_update(self, byte, xact, xtarget, enc, temp):
        """Update the status panel with the provided values.

        - byte: status bits (int)
        - xact: actual position
        - xtarget: target position
        - enc: encoder position
        - temp: temperature Celsius
        """
        self.status_motors.update_leds(byte)
        self.status_motors.update_positions(xact, xtarget, enc)
        self.status_motors.update_status(temp)


# getters and setter for the fron panel
    def set_num_motors(self, val):
        """Set the displayed number of motors."""
        self.motors_numMotors.numInt.set(val)

    def get_motor_selected(self):
        """Return the currently selected motor index (int)."""
        return self.motors_motorSelect.numInt.get()

    def get_motor_position(self):
        """Return the requested motor position from the UI.

        This also zeroes any active velocity control to avoid accidental
        motion when switching to position mode.
        """
        self.motion_vel_slider.set_value(0)
        self.motion_vel_number.numInt.set(0)
        return self.motion_pos.numInt.get()

    def get_setpos_value(self):
        """Return the 'Set pos' value from the motion controls."""
        return self.motion_setpos.numInt.get()

    def get_motor_velocity(self):
        """Return the velocity percentage set in the UI (int).

        If the numeric field was edited but not applied, we copy that
        value to the slider to keep both controls in sync.
        """
        vel = self.motion_vel_number.numInt.get()
        self.motion_vel_slider.set_value(vel)  # sync slider
        return vel

    def get_fileName(self):
        """Return the filename entered in the parameters area."""
        return self.params_fileNameText.get()
    def get_joystick_max(self):
        return self.remote_joystickMax.numInt.get()
    def get_joystick_dir(self):
        return self.remote_joystickDir.numInt.get()
    def get_encoder_step(self):
        return self.remote_encoderStep.numInt.get()
    def get_encoder_dir(self):
        return self.remote_encoderDir.numInt.get()
    def set_joystick_max(self, val):
        self.remote_joystickMax.numInt.set(val)
    def set_joystick_dir(self, val):
        self.remote_joystickDir.numInt.set(val)
    def set_encoder_step(self, val):
        self.remote_encoderStep.numInt.set(val)
    def set_encoder_dir(self, val):
        self.remote_encoderDir.numInt.set(val)

    def vel_slider_to_number(self, event):
        """Callback: slider moved -> update numeric field and notify controller."""
        self.motion_vel_number.numInt.set(self.motion_vel_slider.get_value())
        self.update_velocity(event)

    def vel_number_to_slider(self, event):
        """Callback: numeric entry changed -> update slider and notify controller."""
        self.motion_vel_slider.set_value(self.motion_vel_number.numInt.get())
        self.update_velocity(event)

    def vel_stop_motion(self):
        """Stop any velocity-based motion by zeroing controls and notifying controller."""
        self.motion_vel_slider.set_value(0)
        self.motion_vel_number.numInt.set(0)
        self.update_velocity(None)

    def get_panel_params(self, identifier):
        """ Retrieves the param values from the panel """
        panelName = f"{identifier}Panel"
        panel = getattr(self, panelName)
        keyArray = panel.names
        params = {}
        for name in keyArray:
            ctrl = getattr(panel, name)
            params[name] = ctrl.numInt.get()
        return params

    def set_panel_params(self, identifier, params):
        """ Displays the params on the panel """
        panelName = f"{identifier}Panel"
        panel = getattr(self, panelName)
        for name in params:
            ctrl = getattr(panel, name)
            ctrl.numInt.set(params[name])

