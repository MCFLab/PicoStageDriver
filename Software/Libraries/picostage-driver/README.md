# picostage-driver

Python client for the Pico TMC stage driver hardware.

This small package provides a thin wrapper around a Pico stage driver's
VISA protocol. It exposes a single high-level class, `PicoStage`, that
connects to the device, reads and writes parameters, sends direct commands,
and can save/load parameter sets to JSON files.

## Requirements

- Python 3.8+
- pyvisa (for VISA/serial communications)

Install the driver and dependencies in editable/development mode:

```powershell
# inside PowerShell (adjust python launcher as needed)
pip install -e .
pip install pyvisa
```

If your system uses the `py` launcher, substitute `py -3 -m pip` for `pip`.

## Quick usage

The following example shows a minimal usage pattern. Replace the VISA
resource string with the serial/USB port used by the Pico device.

```python
import logging
from picostage import PicoStage

logger = logging.getLogger("StageDriverLogger")
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.INFO)

address = 'ASRL3::INSTR'  # example VISA resource string
ps = PicoStage(address, logger)

# Enable a motor:
motor = 2
pos = ps.set_motor_status(motor, "Enabled", 1)

# Set a parameter or send a command (value may be None for commands):
ps.set_motor_parameter(0, 'EncLoopMax', 10)

# Save and load parameter sets to JSON
ps.save_parameters_to_file('params.json')
ps.load_parameters_from_file('params.json')

# Close when done
ps.close()
```

## Public API (high level)

- `PicoStage(address, logger)` — connect to the device at `address` and use
    `logger` for informational and error messages.
- `close()` — close instrument and resource manager handles.
- `send_direct_command(command)` — send a raw command string and return the
    response (trimmed).
- `get_*` / `set_*` convenience wrappers — the client exposes several
    convenience methods for reading and writing parameters. Examples:
  - `get_motor_parameter(motor, name)` / `set_motor_parameter(motor, name, value)`
  - `get_remote_parameter(motor, name)` / `set_remote_parameter(motor, name, value)`
  - `get_motor_status` / `set_motor_status`
  - `get_motor_command` / `set_motor_command`
  - `get_pico_command` / `set_pico_command`
- `save_parameters_to_file(filename)` / `load_parameters_from_file(filename)`
— save/load readable parameters for all motors in a JSON file.

For a full list of mapped parameter names, see `src/picostage/command_table.py`.

## Notes

- The client depends on `pyvisa` and a working VISA backend (for example
    `pyvisa-py` or National Instruments VISA libraries) to communicate with the
    device.
- This package deliberately keeps a thin API surface; the `PicoStage` class
    performs name mapping between short Pico IDs and friendlier external names
    (see `command_table.py`).
