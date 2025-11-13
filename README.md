# Welcome to the Pico Stage Driver Project

The **Pico Stage Driver Project** is an open-source development of a low-cost, open-source controller design that drives several stepper motors and implements important safety features, such as monitoring of mechanical limit switches, stall detection, and protective software limits. If rotational encoders in the motors or linear encoders on the stages are available, the controller can monitor the stage position and correct it in a feedback loop. The stages can be controlled with serial commands via USB, or optionally from a remote control box that incorporates a joystick for real-time multi-axis speed control, rotary dials for fine axis positioning, and a display to indicate the stage positions. To ease adoption, we provide driver libraries in Python and C, a driver for the commonly used microscopy control software µManager, and an example graphical user interface for calibration and testing.

## Content

A paper describing the design and implementation is published here:

Mathias S. Fischer, David Grass, and Martin C. Fischer, "Modular multi-axis stepper motor driver with remote control for use in microscopy," [arXiv:2511.09456](https://doi.org/10.48550/arXiv.2511.09456) (2025).

Here we provide the [Bill of Materials](BOM.md), [Assembly Instructions](Assembly.md), and more detailed [Calibration Instructions](Calibration.md) than contained in the publication.

The [Mechanical Drawings](MechanicalDrawings/README.md) and [Software](Software/README.md) are in their respective subdirectories.

## Contributing

We welcome your contributions with [pull-requests](https://github.com/MCFLab/PicoStageDriver/pulls) and [issues suggesting further improvement](https://github.com/MCFLab/PicoStageDriver/issues)!

## License

This work is licensed under the
[GNU General Public License Version 3](https://www.gnu.org/licenses/gpl-3.0.en.html).
