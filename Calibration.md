# Calibration

The settings of the motor parameters can be adjusted and calibrated
using the finished stage driver. For programming and debugging we
utilized the TMC Landungbrücke, which is a USB interface that comes with
the TMC5240-EVAL-KIT (not the TMC5240-EVAL). This interface allows full
control of the TMC board with the TMC-IDE software, which proved helpful
for understanding the TMC features. For adapting our stage driver to
specific motors, it is not required.

## Motor settings

### Mode parameters

MP_MMIC (“ModeMicroStep”) determines the number of microsteps per full
step and corresponds to the MRES field in the CHOPCONF register (number
of microsteps per full step is 2\^MRES). It ranges from 0 (256
microsteps per full step) to 8 (no microstepping). If an encoder is
present, this should be set such that the motor step size approximates
the encoder step size (fine tuning can be done with MP_ECON).

MP_MINV ("ModeInvDir") is a flag to invert (1) the motor direction. It
corresponds to the SHAFT field in the GCONF register.

MP_MTOF ("ModeTOff") corresponds to the TOFF field in the CHOPCONF
register. This parameter is used to enable or disable the motor. A value
of 0 disables the motor, a value between 1 and 15 enables it. For our
motor configuration, any non-zero value will enable the motor, we chose
a default value of 5.

MP_MSGE ("ModeSGEnable") is a flag to enable (1) or disable (0) the
StallGuard2 feature of the TMC. It corresponds to the SG_STOP field in
the RAMP_STAT register. MP_MTCT ("ModeTCT") determines the minimum
velocity at which the stall guard feature is enabled (it will not work
at low speeds) and corresponds to the TCOOLTHRS register. The threshold
velocity could be set slightly below the usual operating velocity $v$.
The value is then set as
$\text{TCOOLTHRS} \approx 1.1*2^{24}*2^{- \text{MRES}}/v$ , where $v$ is
the set velocity in MS/sec and MRES is the microstep factor (see above).
MP_MSGT ("ModeSGT") is a gain factor that determines the sensitivity for
stall guard detection and corresponds to the SGT field in the COOLCONF
register. The range is -64 to +63 (0 is default); higher values make
StallGuard less sensitive and requires more torque to indicate a stall.
This value should be adjusted empirically by running the motor at the
set speed and manually blocking motor rotation until a stall is
registered. Note that for a motor stage with a fine pitch spindle,
especially when the motor is geared, the StallGuard mechanism may not
reliably detect a stall. In this case, setting up encoder deviation
monitoring is more reliable.

### Motor currents

See the section “Current setting” in the TMC manual for a quick
configuration guide. In short, the parameters listed here determine the
current applied during motor motion (“run”) and standstill (“hold”). The
overall current range can be set by hardware configuration through a
resistor attached to the IREF on the TMC chip. On our eval board, we can
choose from a set of discrete built-in resistors (see the TMC Eval data
sheet). We pull IREF_R2 and IREF_R3 on the TMC connector to high (3.3V)
to set the highest current range (the max range is then limited by the
following scale parameters).

MP_CRAN ("CurrRange") corresponds to the field CURRENT_RANGE of register
DRV_CONF and determines the overall range of currents. Values are 0 -\>
max current of 1A, 1 -\> 2A, 2 or 3 -\> 3A.

MP_CSCA ("CurrScaler") corresponds to the GLOBAL_SCALER register and
linearly adjusts the overall currents. The range is 32 to 255 (max);
however, the manual recommends values \>128.

MP_CRUN ("CurrRun") and MP_CHOL (CurrHold") correspond to the fields
IRUN and IHOLD in register IHOLD_IRUN, respectively. It linearly scales
the run and hold currents in the range of 0 to 32 (max). IRUN between 16
and 31 is recommended for best microstep performance.

### Speed settings

MP_RSEV ("RateSetVelocity") is the set velocity for positioning
commands, and MP_RMXV ("RateMaxVelocity") is a maximum allowed velocity
for positioning or velocity commands. MP_RSEA ("RateSetAcc") and MP_RMXA
("RateMaxAcc") are the same but for acceleration. Units are MS/sec and
MS/sec^2^, respectively.

### Homing settings

HMOD ("HomingMode") sets the mode used to search for the reference
position. A value of 0 disables homing, 1 uses the limit switches, and 2
uses the encoder index position. Note that since we did not have an
encoder with an index, the “2” mode is untested. MP_HDIR
("HomingDirection") sets the homing direction: -1 searches in the
negative direction, i.e. for the negative (“left”) limit switch or the
encoder index, +1 towards in the positive direction (right limit switch)
or index. Note that to use the limit switch search, the switch in the
corresponding direction needs to be anabled. MP_HVEL ("HomingVelocity")
determines the velocity for home searches, MP_HSST ("HomingSoftStop") is
a flag whether the motor should do a soft stop (1) at the set
acceleration or a hard stop (0) when a homing position is reached. In
the soft stop case, the homing reference is adjusted to account for the
slow deceleration. MP_HNEV ("HomingIndexEvent") corresponds to the field
pos_neg_edge of register ENCMODE and determines what kind of polarity to
use for the index search.

### Switch settings

MP_SLEN ("SwitchLeftEnable") and MP_SREN ("SwitchRightEnable") are flags
to indicate the presence (1) or absence (0) of left/right limit
switches. MP_SLPO ("SwitchLeftPolarity") and MP_SRPO
("SwitchRightPolarity ") are flags to indicate the polarity of the
left/right switches (0: non-inverted, a high level on the switch stops
the motor; 1: inverted, a low level on the switch stops the motor).
MP_SSWP ("SwitchSwap"), if set to 1, swaps the role of the left and
right switch. These parameters correspond to the STOP_L_ENABLE,
STOP_R_ENABLE, POL_STOP_L, POL_STOP_R, and SWAP_LR fields of the SW_MODE
register, respectively.

### Limits settings

MP_LLEN ("LimLeftEnable") and MP_LREN ("LimRightEnable") are flags to
indicate whether the left/right software (“virtual”) limits are enabled.
These parameters correspond to the EN_VIRTUAL_STOP_L and
EN_VIRTUAL_STOP_R fields of the SW_MODE register, respectively. The
actual left and right limits are determined by MP_LLPS
("LimLeftPosition") and MP_LRPS ("LimRightPosition"), which correspond
to the VIRTUAL_STOP_L and VIRTUAL_STOP_R registers. MP_LENC
("LimEncoder") determines whether the motor position (0) or the encoder
position (1) is used to determine the virtual limits. This corresponds
to the VIRTUAL_STOP_ENC field of the SW_MODE register.

### Encoder settings

MP_ECON ("EncConstant") is the encoder constant, which in turn
determines the number of steps the TMC adds for every encoder count
received. This parameter corresponds to the ENC_CONST register. The
constant reflects the ratio $r$ = microsteps / encoder count. The ratio
does not have to be an integer but needs to be sent to the register as a
signed integer. The TMC has a specific way to encode the fraction $r$ as
the ENC_CONST, which is not the usual floating point representation. For
a detailed description, please refer to the TMC datasheet. Here we
utilize the decimal representation. To calculate the encoder constant we
split $r$ into its integer part $r_{i}$ and fractional part $r_{f}$.
ENC_CONST can then calculated as:

$$
{\mathrm{ENC}}_{-}{\mathrm{CONST}} =
\begin{cases}
2^{16} \ r_i + 10{,}000 \ r_f & \text{if } r > 0 \\
2^{16} \ (2^{16} - r_i - 1) + 10{,}000\ (1 - r_f) & \text{if } r < 0
\end{cases}
$$

Furthermore, if $r$ is negative, this constant needs to be converted
into a 32-bit signed format, which can be achieved by:

$${{\mathrm{ENC}}_{-}{\mathrm{CONST}}}_{\text{signed}} = {\mathrm{ENC}}_{-}{\mathrm{CONST}} - 2^{32}$$

Since this method is less-than-intuitive, we provided a helper python
function for this conversion in the repository.

MP_EDEV ("EncDeviation") is the maximum allowed deviation between
encoder reading and motor position before a following error is raised.
Set this to 0 to disable deviation check. This parameter corresponds to
the ENC_DEVIATION register.

MP_EMAX ("EncLoopMax") sets the position operation mode. A value of 1
(one try to get to the desired position) corresponds to an open-loop
mode. A value \>1 sets the “open loop with pull-ins” mode and sets the
maximum number of pull-in tries. A value of 0 sets the “closed-loop”
mode, where the motor continuously readjusts according to the encoder
position.

MP_ETOL ("EncLoopTolerance") sets the tolerance window for “Open-loop
with pull ins” and “Closed-loop” operation modes.

If there is a large deviation between the TMC motor position and the
encoder position, a small incremental movement would result in a first
step of the size of this deviation before it is pulled back to the
target position. To avoid this, we can set MP_ERST ("EncResetXafterCL")
to 1, for which the actual and target position register of the TMC are
set to the encoder position after a pull-in or closed loop motion
completes.

### Device and axis types

MP_TDEV (TypeDevice) determines whether a TMC board of the respective
board number is actually attached to the Pico. A value of 0 indicates no
TMC is attached – any attempt to send commands to this board will result
in an error. A value of 1 simulates a board – positioning and velocity
commands still work, but don’t control a motor. This mode is useful for
debugging. Finally, a value of 2 indicates a TMC board present.

MP_TAXI ("TypeAxis") is intended for use with µManager and serves to
identify what kind of axis the channel corresponds to. The values are: 0
is an undefined axis, 1 is an X-axis, 2 a Y-axis, 3 a Z-Axis, and 4 an
auxiliary axis. When set, µManager can automatically assign the channels
to the corresponding devices (XY-Stage, Z-Stage, or Aux-Stage).
