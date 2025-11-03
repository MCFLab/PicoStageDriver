"""Pico stage driver client

This module contains a thin client for communicating with a Pico stage driver
device via VISA (pyvisa). The main public surface is the :class:`PicoStage`
class which exposes methods to query and set parameters, send direct
commands and save/load parameter sets to/from JSON files.

Notes:
- This client is not thread-safe by itself; it uses an internal lock for VISA
    access (``visa_lock``) to serialize communication.
- The module expects pyvisa to be available and a VISA resource string to be
    provided when constructing :class:`PicoStage`.
"""

import pyvisa
import sys
import re
import threading
import json
from .command_table import table
from typing import List, Dict

class InstrumentError(Exception):
    """Raised when an unexpected or fatal instrument condition is detected.

    Used when the connected device does not present the expected ID or when
    initialization cannot continue.
    """
    pass


class PicoStage:
    """Client for a Pico TMC stage driver.

    The class handles opening a VISA resource manager and a device resource,
    performing basic reads and writes and providing convenience wrappers for
    parameter access. Typical usage:

    >>> ps = PicoStage('ASRL3::INSTR', logger)
    >>> pos = ps.get_motor_parameter(0, 'ActualPosition')
    >>> ps.set_motor_parameter(0, 'TargetPosition', 1000)

    Attributes:
        logger: a logging-like object used for informational and error output.
        visa_lock: threading.Lock used to serialize VISA queries.

    Implementation notes:
        - Parameter names used by external callers are mapped to the Pico
          parameter IDs using the mapping imported from ``command_table.table``.
    """
    
    def __init__(self, address, logger):
        """ Connects to the stage driver

        Opens the resource manager, then the device. Terminates if the device cannot 
        be found. Sets the communication attributes. Then checks the ID response and
        throws an exception if it's the wrong device.
        Arguments:
          address: a VISA Resource ID, like 'ASRL3::INSTR'
        """
        self.logger = logger
        self.logger.info("Initializing instrument.")
        try:
            self._rm = pyvisa.ResourceManager("@py")
        except Exception as e:
            self.logger.error("Could not open resource manager.")
            print(e)
            sys.exit(1)
        try:
            self._inst = self._rm.open_resource(address)
        except Exception as e:
            self._rm.close()
            self.logger.info("Could not open instrument")
            print(e)
            sys.exit(1)
        self.logger.info(f"Instrument: {self._inst}")
        self._inst.read_termination = "\n"
        self._inst.write_termination = "\n"
        self._inst.baud_rate = 9600
        self.logger.info("Requesting ID from instrument")
        resp = self._inst.query("*IDN?").rstrip("\r\n")
        if not resp.startswith("Stage Driver Pico"):
            self._inst.close()
            self._rm.close()
            raise InstrumentError(
                f"Wrong ID response. Expected 'Stage Driver Pico', got '{resp}'.")
        self.visa_lock = threading.Lock()
        # prepare lookup dictionary
        self.pico_to_ext = table
        self.ext_to_pico = {v: k for k, v in table.items()}


    def __del__(self):
        # backup: close on deletion
        self.close()


    def close(self):
        """ Closes the stage driver

        Closes the instrument (if open) and the resource manager (if open).
        The deletes the attributes (just in case).
        """
        if hasattr(self, "_inst"):
            self.logger.info("Closing instrument.")
            self._inst.close()
            delattr(self, "_inst")
        else:
            self.logger.info("Instrument not open. Do nothing.")
        if hasattr(self, "_rm"):
            self.logger.info("Closing resource manager.")
            self._rm.close()
            delattr(self, "_rm")
        else:
            self.logger.info("Resource manager not open. Do nothing.")


    def get_error(self):
        """Query the instrument for a pending error message.

        Returns the raw error message string if an error exists, otherwise
        returns None. The method logs the check at INFO level.
        """
        self.logger.info("Checking for errors.")
        with self.visa_lock:
            resp = self._inst.query("GPC_EMSG").rstrip("\r\n")
        if not resp.startswith("PC_EMSG=No errors"):
            return resp


    def get_motor_parameter_array(self, motor, params: List[str]) -> Dict[str, int]:
        return self.get_parameter_array('MP', motor, params)

    def set_motor_parameter_array(self, motor, params: Dict[str, int], value=None) -> None:
        return self.set_parameter_array('MP', motor, params, value)

    def get_remote_parameter_array(self, motor, params: List[str]) -> Dict[str, int]:
        return self.get_parameter_array('RP', motor, params)

    def set_remote_parameter_array(self, motor, params: Dict[str, int], value=None) -> None:
        return self.set_parameter_array('RP', motor, params, value)

    def get_motor_parameter(self, motor, params):
        return self.get_parameter('MP', motor, params)

    def set_motor_parameter(self, motor, params, value=None):
        return self.set_parameter('MP', motor, params, value)

    def get_remote_parameter(self, motor, params):
        return self.get_parameter('RP', motor, params)

    def set_remote_parameter(self, motor, params, value=None):
        return self.set_parameter('RP', motor, params, value)

    def get_motor_status(self, motor, params):
        return self.get_parameter('MS', motor, params)

    def set_motor_status(self, motor, params, value=None):
        return self.set_parameter('MS', motor, params, value)

    def get_motor_command(self, motor, params):
        return self.get_parameter('MC', motor, params)

    def set_motor_command(self, motor, params, value=None):
        return self.set_parameter('MC', motor, params, value)

    def get_pico_command(self, params):
        return self.get_parameter('PC', None, params)

    def set_pico_command(self, params, value=None):
        return self.set_parameter('PC', None, params, value)




    def send_direct_command(self, command):
        """ Sends a command to the device and returns the response

        Arguments:
            command: a string with the command
        Returns the response
        """
        self.logger.info(f"Sending command '{command}' to the device.")
        with self.visa_lock:
            return self._inst.query(command).rstrip("\r\n")

    def save_parameters_to_file(self, filename):
        """Save readable parameters for all motors to a JSON file.

        The saved JSON contains one object per motor, keyed as ``motor0``,
        ``motor1`` etc. Only parameters that successfully read are included.

        Args:
            filename: path to write the JSON file to.
        """

        param_list = self.get_pico_names("MP_")
        param_list.extend(self.get_pico_names("RP_"))
        param_list_ext = [self.pico_to_ext[k] for k in param_list]
        all_data = {}
        for motor in range(4):
            motor_data = {}
            for param in param_list_ext: 
                try:
                    param_val = self.get_value(None, motor, param)
                except Exception as e:
                    # skip identifier if get_value fails
                    self.logger.info(f"Warning: could not get {param!r} for motor{motor}: {e}")
                    continue
                if param_val is not None:
                    motor_data[param] = param_val
            all_data[f'motor{motor}'] = motor_data
        with open(filename, "w") as f:
            json.dump(all_data, f, indent=4)


    def load_parameters_from_file(self, filename):
        """Load parameter values from a file and apply them to the device.

        The file format is the same as written by :meth:`save_parameters_to_file`.

        Args:
            filename: path to the JSON file to read.
        """

        with open(filename, "r") as f:
            all_data = json.load(f)
        for label, data in all_data.items():
            # loop through motors
            match = re.fullmatch(r'motor(\d+)', label)
            if match:
                motor = int(match.group(1))
            else:
                self.logger.error(f"Illegal label {label} in file.")
                return
            for param, val in data.items():
                try:
                    self.set_value(None, motor, param, val)
                except Exception as e:
                    # skip identifier if set_value fails
                    self.logger.info(f"Warning: could not set {param!r} for motor{motor}: {e}")
                    continue
                            


# Private functions

    def get_pico_names(self, pattern):
        """Return a list of Pico parameter IDs that contain ``pattern``.

        Example: ``get_pico_names('MP_')`` returns keys for motor parameters.
        """
        return [key for key in self.pico_to_ext if pattern in key]

    def get_param_names(self, pattern):
        """Return mapped external parameter names whose Pico IDs contain ``pattern``.

        The returned names are the values from the command table mapping.
        """
        return [self.pico_to_ext[key] for key in self.pico_to_ext if pattern in key]


    def get_parameter_array(self, ID, motor=None, params=None ):
        """Get multiple parameters and return a mapping of name->value.

        Args:
            ID: parameter type prefix (e.g. 'MP' for motor parameters).
            motor: motor index (or None for global parameters).
            params: list of external parameter names to read.

        Returns:
            dict mapping parameter names to integer values. If ``params`` is
            not a list, an error is logged and None is returned.
        """
        if isinstance(params, list): # handle list case
            retDict = {}
            for param in params:
                retDict[param] = self.get_value(ID, motor, param)
            return retDict
        else: # must be a list
            self.logger.error(f"params is not a list.")

    def get_parameter(self, ID, motor=None, param=None ):
        """Get a single parameter value by external name.

        Args:
            ID: parameter type prefix like 'MP', 'RP', 'MS', 'MC', 'PC'.
            motor: motor index or None.
            param: external parameter name (string).

        Returns:
            Integer value for the parameter or None on error.
        """
        if isinstance(param, str): # handle single value case 
            return self.get_value(ID, motor, param)
        else: # must be a single number
            self.logger.error(f"param is not a single string.")

    def get_value(self, ID, motor=None, param=None):
        """Translate an external parameter name and retrieve its value.

        Performs validation of ID/type and read-ability, then queries the
        device using :meth:`get_value_from_dev`.

        Returns an integer or None if the parameter is not readable/valid.
        """
        self.logger.info(f"Getting value {param} for motor {motor}.")
        pico_param = self.ext_to_pico[param]
        if ID is not None and not (ID+'_') in pico_param:
            self.logger.error(f"Value {param} not of type {ID}.")
            return
        if pico_param in [ "PC_SAFL", "MC_HOME", "MC_CONF", "MC_SCLR", "MC_MPOS", "MC_MVEL"]:
            self.logger.error(f"Value {param} not readable.")
            return
        return self.get_value_from_dev(motor, pico_param)

    def get_value_from_dev(self, motor=None, queryString=None):
        """Query the device using a raw Pico query string and return an int.

        The method composes a "G" (get) query using the Pico parameter string
        plus an optional motor index, sends it to the device and parses an
        integer from the response. If the response is malformed, an error is
        logged and None is returned.
        """
        query = f"G{queryString}{motor if motor is not None else ''}"
        with self.visa_lock:
            resp = self._inst.query(query).rstrip("\r\n").rstrip("\r\n")
        if resp.startswith(queryString):
            parts = resp.rsplit("=", 1)
            results = parts[1]
            numbers = re.findall(r"[-+]?\d+", results)
            if len(numbers)!=1:
                self.logger.error(f"Wrong response. Expected one numeric value, got {len(numbers)}.")
                return
            return int(numbers[0])
        else:
            self.log_error(resp)

    def set_parameter_array(self, ID, motor=None, params=None, value=None):
        """Set multiple parameter values from a dict mapping name->value.

        Args:
            ID: parameter type prefix like 'MP' or 'RP'.
            motor: motor index or None.
            params: dict of external names to values.
        """
        if isinstance(params, dict): # handle dict case
            for param, val in params.items():
                self.set_value(ID, motor, param, val)
        else:
            self.logger.error(f"params is not a dict.")

    def set_parameter(self, ID, motor=None, param=None, value=None):
        """Set a single parameter by external name.

        Args:
            ID: parameter type prefix like 'MP', 'RP', 'MS', 'MC', 'PC'.
            motor: motor index or None.
            param: external parameter name (string).
            value: value to write (or None for commands that don't take an arg).
        """
        if isinstance(param, str): # handle single value case 
            self.set_value(ID, motor, param, value)
        else:
            self.logger.error(f"param is not a single string.")

    def set_value(self, ID, motor=None, param=None, value=None):
        """Translate an external parameter name and write the value to device.

        Performs validation of ID/type and write-ability, then issues the
        command with :meth:`set_value_to_dev`.
        """
        self.logger.info(f"Setting value {param} for motor {motor}.")
        pico_param = self.ext_to_pico[param]
        if ID is not None and not (ID+'_') in pico_param:
            self.logger.error(f"Value {param} not of type {ID}.")
            return
        if pico_param in ["PC_NDEV", "PC_VERS", "MS_TEMP", "MS_PULL", "MC_POSR", "MC_STAT"]:
            self.logger.error(f"Value {param} not writeable.")
            return
        self.set_value_to_dev(motor, pico_param, value)


    def set_value_to_dev(self, motor=None, queryString=None, value=None):
        """Compose and send an S (set) command to the device.

        The method sends the set command and delegates response checking to
        :meth:`log_error` which will log non-zero error responses.
        """
        valStr = f"{',' + str(value) if value is not None else ''}"
        query = f"S{queryString}{motor if motor is not None else ''}{valStr}"
        with self.visa_lock:
            resp = self._inst.query(query).rstrip("\r\n").rstrip("\r\n")
        self.log_error(resp)


    def log_error(self, resp):
        """Inspect a raw response string and log any reported error.

        The device uses a simple "ERROR=N" prefix; when a non-zero error is
        present, an additional error string is obtained via :meth:`get_error`.
        """
        if resp.startswith("ERROR=0"):
            return
        else:
            self.logger.error(resp)
            errMsg = self.get_error()
            self.logger.error(errMsg)


     