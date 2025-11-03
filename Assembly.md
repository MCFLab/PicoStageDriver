# Assembly Instructions

## Motor controller

### Enclosure prep

1. Lay out the TMC boards and the protoboard in the box. Make sure to
   leave room for the screw connectors sticking out from the TMC
   boards. Also leave enough room for the switch and LEDs in the front
   but remember that the USB cable sticks through the front panel to
   the Pico, so don’t mount the Pico too far back.

1. Mark the mounting holes with a pencil. Drill the holes, screw in the
   board spacers from the bottom (we used 4-40 spacers). Test to make
   sure the board fits on the spaces, but don’t’ mount them yet. Drill
   an extra hole for the GND to case connection later.

   <img src="media/Controller Layout.jpg" />

1. Glue on some rubber pads on the bottom so the screws don’t scratch
   up whatever the box is standing on.

1. With the tool of your choice, cut out the holes to mount the
   components in the front and back panels. We provide the design files
   for our layout for use with an automated tool (we used a waterjet),
   but a CNC mill, Dremel tool, or hand tools will work.

   <img src="media/Controller Front and Back Panels.jpg" style="width:50%" />

1. Make sure the USB cutout lines up with the connector of the Pico
   when mounted on the spacers.

### Power and motor layer

1. Prep the motor cable from the TMC board to the motor connector on
   the back of the box. As connectors we used sub-D connectors, but
   that depends on your stages/cables. Use wires that can carry the
   motor current (we used 20 gauge stranded). Twist the A1 and A2 wires
   and the B1 and B2 wires, solder one side to the motor connector in
   the back of the box. Cut to length, mount the connector, and route
   the cable in the box. The other end of the cables goes in the screw
   connector of the TMC boards. Make sure the pinouts match!

   <img src="media/Controller Motor layer.jpg" />

1. Mount the power jack (barrel connector), route the +V~M~ cable (we
   used a red 18 gauge wire) from the barrel to the switch (we used
   flat terminal connectors for the switch) and from there to the
   terminal strip. The GND cable goes to the terminal strip. At the
   terminal strip, branch out the wires.

1. From the terminal strip, one GND connection goes to the case for
   protection and shielding (we use a wire lug in a spare hole in the
   box).

1. One (thin) +V<sub>M</sub>/GND wire pair goes to the power led in the front.
   Use a resistor in series to limit the current. The resistor value
   depends on the supply voltage – we used a 4.7k resistor. Mount the
   LED (we used a mounting ring but still had to glue that in place).
   We put a connector in the wire pair so we could detach the LED when
   taking off the front panel.

1. Run a pair of twisted +V<sub>M</sub>/GND wires to each of the TMC boards. We
   ran the supply wires under the boards for convenience. The
   connectors on the board are screw connectors.

1. Mount the TMC boards on the spacers.

   <img src="media/Controller TMC Layer Image.jpg" />

### Pico protoboard

1. Here is the layout of the board:

   <img src="media/Controller Pico Layout.jpg" style="width:60%"/>

1. Solder two rows of 20-pin female headers for the Pico.

1. Solder in the male headers (squares in the layout above):

   - 4 rows of 8-pin headers for the TMC connectors.

   - Individual pins for the CS, LED, remote power, and UART.

1. Solder in the “load default” push button

1. Solder in a few hookup wires according to the layout diagram.

1. Mount the protoboard on the spacers. If you use metallic spacers,
   when mounting the Pico protoboard on them, make sure they don’t
   short any connections underneath. If you are close to a connection,
   use a plastic washer.

### Control layer

1. Prep the cables from the TMC board to the encoder / limit switch
   connectors on the back of the box (we used sub-D connectors, but
   that depends on your stages/cables). For the TMC encoder connector
   side, we used prefabricated 5-pin Dupont connector cables. For the
   limit switch connectors, we used a row of female headers. Solder the
   other end of the cables into the sub-D connector. Double-check the
   pinouts!

1. Prep the TMC to Pico connectors. We used a 2-row male header for the
   TMC side. To reduce the number of wires, connect several of the pins
   right at the connector (see diagram below). There are a total of 8
   wires from the connector to the Pico protoboard. On the Pico side we
   used DuPont headers. Some of these are (again) prefab 5-pin
   connectorized cables, the others are individual connectors. You can
   put on Dupont connectors yourself with a crimp tool, but it’s easier
   to use a handful of connectorized hookup cables.

   <table>
       <tr>
           <td>
             <img src="media/Controller TMC connector wiring.jpg" />
           </td>
           <td>
             <img src="media/Controller TMC Connector.jpg" />
           </td>
       </tr>
   </table>

1. Prep the RJ-45 jack cables. One side gets soldered into the jack,
   the other gets Dupont connectors. Make sure the TX and RX connectors
   are mapped to lines that are twisted with ground. We used this
   assignment:

   <img src="media/Controller RJ-45 Wire Labels.png" style="width:25%" />

1. Mount the jack. We screwed the Jack into a plastic adapter and glued
   the adapter to the back plate.

   <img src="media/Controller RJ-45 mount.jpg" style="width:50%" />

1. For the Pico power LED, use a series resistor (\~1k) to limit the
   current. Use Dupont connectors on the Pico side.

1. Connect the LED (2 pins), the RJ-45 jack (4 pins).

1. Connect the TMC connectors (8 pins each). For the V<sub>dig</sub> connector,
   choose a pin from the 3.3V or 5V line of the Pico protoboard,
   depending on the required voltage of your encoder. When pushing in
   the TMC connectors, make sure all pins are well seated, since
   soldering tends to loosen the pins in the connector. Triple check
   the connections!

   <img src="media/Controller Pico Wired.jpg" />

1. If required (for larger motors) install a brake resistor (see TMC
   datasheet). **Careful**: At some point during software debugging of
   the SPI communication, the TMC chips ended up in a mode that
   continually activated the overvoltage transistor (visible by the red
   LED on the TMC board). This seriously overheated the brake resistor!
   This should no longer occur, but we never ended up installing the
   brake resistors.

   <img src="media/Controller Control layer.jpg" />

   <img src="media/Controller box complete open.jpg" />

Congratulations, you concluded the motor control assembly! We recommend
operating the controller for a while without lid (see our comment on the
brake resistor!) so you can see what’s going on in there. Also,
initially you might need access to the default load button (in the next
version, this button could be placed in a more accessible location,
maybe where it can be reached through the USB cutout).

## Remote controller

### Pico protoboard

1. Here is the layout of the board:

   <img src="media/Remote Pico Layout.jpg"
alt="A screenshot of a computer AI-generated content may be incorrect." />

1. Solder two rows of 20-pin female headers for the Pico.

1. Solder in the male headers (squares in the layout above):

   - 3 pins for the potentiometer (Sens, GND, and 3.3V)

   - 4 pins for the RJ-45 jack (GND, supply voltage for the pico, TX,
      and RX)

1. Solder in the protection diode for the Pico supply voltage as
   indicated.

1. Solder in a few hookup wires according to the layout diagram.

1. Solder in sets of 5-pin prefab Dupont connector wires for the
   devices listed. Note that the wires in the connectors can be removed
   and rearranged to adhere to a common color scheme (if desired).

   - 4 encoders

   - 1 joystick

   - 1 display (only uses 4 of the 5 wires)

1. Solder the wires to the pot, the other end should be Dupont
   connectors.

1. Prep the RJ-45 jack cables. Use the same assignment as above.

1. Connect all these. Note that you change the direction of the
   sensitivity pot (clockwise/anticlockwise) by swapping the GND and
   3.3V pins. Also, the TX from the motor controller needs to go in the
   RX of the remote controller and vice versa.

   <img src="media/Remote Pico Wired.jpg" />

1. Mount the RJ-45 jack, the display, the joystick, the encoders, and
   the pot.

1. 3D print the encoder knobs (or use other ones) and attach then to the encoders.

   <img src="media/Remote Box Assembled.jpg">

Congratulations, you concluded the remote controller assembly! You can now attach it to the motor controller via a standard ethernet cable connected to the two RJ-45 jacks. You can connect the remote controller to a USB port for debugging, but that is optional. During normal operation it is powered from the motor controller (no extra configuration required).
