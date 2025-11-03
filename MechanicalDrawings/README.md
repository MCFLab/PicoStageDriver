# Mechanical Drawings

## Motor Controller

For the motor controller, we used an aluminum enclosure (Hammond 1455U2801, see BOM). On the front, we need holes for the USB cable to the Pico, the power switch, and two LEDs. On the back we mounted two rows of 9-pin Sub-D connectors for the motors and encoder/limit switches, the power connector jack, and the connector for the cable to the remote controller. We used a waterjet to cut the holes, using the "Motor Controller Front Panel" and "Motor Controller Back Panel" design files (supplied as Fusion 360 and STEP files). The cable connector is screwed to a 3D-printed adapter piece "RJ45Adapter", which we then glued to the inside of the back plate of the case.

## Remote Controller

The components for the remote controller are mounted in a 3D-printed case, consisting of a top shell and a bottom plate. For modularity, we designed the top shell as an assembly, which contains the shell and inserts for the joystick, the display, and motor controller cable connector. All of the shell components are attached with small #2 self-tapping screws. The encoders and the sensitivity potentiometer have threads and nuts and are fitted into the corresponding holes.

Note that the mounting holes for these components can differ between manufacturers and the dimensions might need adjustments.

We provide the Autodesk Inventor files and STEP files for the remote controller enclosure:

- Inventor:
  - "Top Assembly" (incorporating "RemoteControllerTop", "RJ45Insert", "JoystickInsert", and "DisplayInsert")
  - "RemoteControllerBottom"
- STEP
  - "Top Assembly"
  - "RemoteControllerBottom"
