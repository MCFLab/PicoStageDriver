# Software installation

## Building the Pico code

The motor and remote controller software can be built either with the
Arduino IDE or with Visual Studio Code and PlatformIO.

The Arduino IDE needs to be configured using the Earle Philhower
framework. This can be achieved by including
<https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json>
as an additional board manager URL in the Arduino IDE preference. In
addition, the driver for the display needs to be installed through the
library manger – search for “Adafruit SSD1306” and “Adafruit GFX” will
bring up the correct libraries for installation. After that, loading the
corresponding .ino file will also find the associated files. Once
“Raspberry Pi Pico 2” with the correct COM port number is selected, the
program is ready for building and flashing.

The VS code environment needs the PlatformIO extension to be installed.
Once installed, opening the folder with the platformio.ini file should
be all that is required. Make sure that the COM port listed in this file
reflects the correct port of the Pico.

## Libraries and GUI

### C library

The C library requires a VISA driver library (see the paper for links).
These are installer packages and should not require further
configuration. The library header file contains the function prototypes.

### Python library

The Python library also requires a VISA installation. We use the free
pyVISA and pyVISA-py packages. Since we use a serial connection,
pySerial is also required. We combined the Python library into a whl
file that contains a pre-built version of the Python package that can be
directly installed and which also includes the required dependencies. A
simple `pip install picostage-driver.whl` will do the trick.

### Python GUI and test scripts

The python GUI uses tkinter, which comes bundled with python
installations. For calibration, we wrote a Jupyter notebook with several
test scripts, which utilize the matplotlib, numpy, and pandas packages.

### µManager driver

Once validated by a few other groups, we intend to request a pull to incoporate
our driver in the µManager distribution.
For the time being,
to install the driver on a Win64 system, simply copy the file `mmgr_dal_PicoStageDriver.dll`
into the main µManager directory (e.g. `C:\Program Files\Micro-Manager-2.0`), adjust the
startup config file (see below), and start µManager.
Should this fail, a compilation is required (see the [instructions to set up the µManager environment](https://micro-manager.org/Building_MM_on_Windows)). The provided driver files
should be copied into a directory named
`PicoStageDriver\` in the `mmCoreAndDevices\DeviceAdapters\` directory
and the project should be built.

The µManager driver is based on the Arduino example by Nico Stuurman
(<https://micro-manager.org/Arduino>). This driver uses a Hub device, through
which the XY stage, the Z-stage, and an auxiliary Z-stage are
controlled. The stage driver can be set up with axis identifiers (see
"TypeAxis" below) that automatically identify the corresponding axes in
µManager. Ultimately, we will request inclusion of our driver into the
µManager repository, but for the time being, the compiled dll can be
copied into the program directory (both the dll and source are included
in the online repo). The hardware wizard will then recognize the
picostage driver and configure the device for inclusion. The axis
velocity and acceleration setting for each configures axis are read from
the controller Pico, but can be overridden with device parameters from
µManager. The only parameters not obtained from the controller are the
step sizes, which can be set as device properties, e.g.

```
Property,Pico-XYStage,StepSizeX [um],0.078125  
Property,Pico-XYStage,StepSizeY [um],0.078125  
Property,Pico-ZStage,StepSize [um],0.03125  
```

Note that requesting remote access from the remote controller will
prevent stage control from the computer, i.e. µManager. This behavior is
by design to prevent accidental (and undetected) motion during a
computer acquisition. For µManager to regain control of the stages, the
device parameter "IsRemoteControlled" can be re-set to 0.
