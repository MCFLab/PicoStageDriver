"""Controller layer: connects the GUI Panel to the PicoStage backend.

This module wires UI events (button presses, periodic updates) to
actions on the low-level PicoStage class. It also routes logging
messages to the panel's communication area.
"""

from stage_driver_view import Panel
from picostage import PicoStage
import logging
import logging_texthandler
from time import sleep


class Controller:
    """High-level controller that mediates between the Panel and PicoStage.

    The controller binds UI callbacks, handles parameter transfers and
    status polling. Methods named for UI actions accept an ``event``
    parameter because they are bound to Tk events; the parameter is
    unused but accepted for compatibility.
    """

    def __init__(self, panel: Panel):
        """Initialize controller, attach logger and detect devices."""
        self.panel = panel

        # Attach text widget to logger so controller/stage logs are visible
        logging_texthandler.TextHandler(self.panel.comm_print_line, logger_name="StageDriver")
        self.logger = logging.getLogger("StageDriver")
        self.logger.setLevel(logging.ERROR)

        # Create PicoStage instance (hardware abstraction)
        self.stages = PicoStage('ASRL9::INSTR', self.logger)
        self.stages.set_motor_command(-1, "StatusClear", None)
        self.stages.get_error()
        self.check_num_devices()

        # bind the buttons from the panel to controller methods
        self.panel.bind_button('getParams', self.get_params)
        self.panel.bind_button('setParams', self.set_params)
        self.panel.bind_button('loadParams', self.load_params_from_file)
        self.panel.bind_button('saveParams', self.save_params_to_file)
        self.panel.bind_button('configMotor', self.config_motor)
        self.panel.bind_button('clearMotor', self.clear_motor)
        self.panel.bind_button('enableMotor', self.enable_motor)
        self.panel.bind_button('disableMotor', self.disable_motor)
        self.panel.bind_button('moveToPos', self.move_to_pos)
        self.panel.bind_button('setEncPositions', self.set_enc_position)
        self.panel.bind_button('setAllPositions', self.set_all_position)
        self.panel.bind_button('moveAtVel', self.move_at_vel)
        self.panel.bind_button('getRemote', self.get_remote)
        self.panel.bind_button('setRemote', self.set_remote)
        self.panel.bind_button('enableRemote', self.enable_remote)
        self.panel.bind_button('disableRemote', self.disable_remote)
        self.panel.bind_button('checkError', self.check_error)

        # communication and status callbacks
        self.panel.comm_associate(self.send_command)
        self.panel.status_motors_associate(self.check_status)

    def run(self):
        """Start the GUI mainloop."""
        self.panel.mainloop()

    def check_num_devices(self):
        """Query connected device count and update the panel display."""
        dev_num = self.stages.get_pico_command("NumberofDevices")
        if dev_num is not None:
            self.panel.set_num_motors(dev_num)


    def get_params(self, event):
        """Read parameter groups from the hardware and populate the panel."""
        motor = self.panel.get_motor_selected()
        for id in ["Curr", "Mode", "Homing", "Rate", "Switch", "Lim", "Enc"]:
            short_names = list(self.panel.get_panel_params(id).keys())
            full_names = [id + s for s in short_names]
            params = self.stages.get_motor_parameter_array(motor, full_names)
            params_short_names = {k[len(id):]: v for k, v in params.items()}
            if params:
                self.panel.set_panel_params(id, params_short_names)

    def set_params(self, event):
        """Collect parameter values from the panel and send to the hardware."""
        motor = self.panel.get_motor_selected()
        for id in ["Curr", "Mode", "Homing", "Rate", "Switch", "Lim", "Enc"]:
            params = self.panel.get_panel_params(id)
            if params:
                params_long_names = {id + k: v for k, v in params.items()}
                self.stages.set_motor_parameter_array(motor, params_long_names)

    def load_params_from_file(self, event):
        """Load parameter file on the stage hardware using panel filename."""
        file_name = self.panel.get_fileName()
        self.stages.load_parameters_from_file(file_name)

    def save_params_to_file(self, event):
        """Save current parameters to the filename provided by the panel."""
        file_name = self.panel.get_fileName()
        self.stages.save_parameters_to_file(file_name)

    def config_motor(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_motor_command(motor, "Config", None)

    def clear_motor(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_motor_command(motor, "StatusClear", None)

    def enable_motor(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_motor_status(motor, "Enabled", 1)

    def disable_motor(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_motor_status(motor, "Enabled", 0)

    def move_to_pos(self, event):
        motor = self.panel.get_motor_selected()
        pos = self.panel.get_motor_position()
        self.stages.set_motor_command(motor, "MoveToPosition", pos)

    def move_at_vel(self, event):
        motor = self.panel.get_motor_selected()
        rel_vel = self.panel.get_motor_velocity()
        set_vel = self.stages.get_motor_parameter(motor, "RateSetVelocity")
        if set_vel is not None:
            self.stages.set_motor_command(motor, "MoveAtVelocity", set_vel * rel_vel / 100)

    def get_remote(self, event):
        """Read remote control parameters and update the panel controls."""
        motor = self.panel.get_motor_selected()
        joystick_max = self.stages.get_remote_parameter(motor, "JoystickMax")
        joystick_dir = self.stages.get_remote_parameter(motor, "JoystickDirection")
        encoder_step = self.stages.get_remote_parameter(motor, "EncoderStepSize")
        encoder_dir = self.stages.get_remote_parameter(motor, "EncoderDirection")
        self.panel.set_joystick_dir(joystick_dir)
        self.panel.set_joystick_max(joystick_max)
        self.panel.set_encoder_dir(encoder_dir)
        self.panel.set_encoder_step(encoder_step)

    def set_remote(self, event):
        """Collect remote parameter values from the panel and write to hardware."""
        motor = self.panel.get_motor_selected()
        joystick_dir = self.panel.get_joystick_dir()
        joystick_max = self.panel.get_joystick_max()
        encoder_dir = self.panel.get_encoder_dir()
        encoder_step = self.panel.get_encoder_step()
        self.stages.set_remote_parameter(motor, "JoystickMax", joystick_max)
        self.stages.set_remote_parameter(motor, "JoystickDirection", joystick_dir)
        self.stages.set_remote_parameter(motor, "EncoderStepSize", encoder_step)
        self.stages.set_remote_parameter(motor, "EncoderDirection", encoder_dir)

    def enable_remote(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_remote_parameter(motor, "RemoteEnabled", 1)

    def disable_remote(self, event):
        motor = self.panel.get_motor_selected()
        self.stages.set_remote_parameter(motor, "RemoteEnabled", 0)

    def set_enc_position(self, event):
        motor = self.panel.get_motor_selected()
        enc = self.panel.get_setpos_value()
        self.stages.set_motor_status(motor, "EncoderPosition", enc)

    def set_all_position(self, event):
        """Set multiple position/status fields atomically to a given value.

        This temporarily disables the motor to avoid movement while the
        reported positions are updated.
        """
        motor = self.panel.get_motor_selected()
        enc = self.panel.get_setpos_value()
        # turn off to prevent movement while setting values
        is_enabled = self.stages.get_motor_status(motor, "Enabled")
        self.stages.set_motor_status(motor, "Enabled", 0)
        self.stages.set_motor_status(motor, "TargetPosition", enc)
        self.stages.set_motor_status(motor, "ActualPosition", enc)
        self.stages.set_motor_status(motor, "Enabled", is_enabled)
        # give motor time to engage
        sleep(0.1)
        self.stages.set_motor_status(motor, "EncoderPosition", enc)

    def check_status(self):
        """Poll motor status bits and update the status panel display."""
        motor = self.panel.get_motor_selected()
        motor_type = self.stages.get_motor_parameter(motor, "TypeDevice")
        if motor_type is None or motor_type == 0:
            return
        status_byte = self.stages.get_motor_command(motor, "GetStatus")
        motor_pos = self.stages.get_motor_status(motor, "ActualPosition")
        motor_target = self.stages.get_motor_status(motor, "TargetPosition")
        enc_pos = self.stages.get_motor_status(motor, "EncoderPosition")
        temp_C = self.stages.get_motor_status(motor, "Temperature")
        self.panel.status_motors_update(status_byte, motor_pos, motor_target, enc_pos, temp_C)

    def check_error(self, event):
        result = self.stages.get_error()
        if result is not None:
            self.panel.comm_print_line(result)

    def send_command(self, text):
        """Send a raw command to the PicoStage and print the response."""
        resp = self.stages.send_direct_command(text)
        self.panel.comm_print_line(resp)

