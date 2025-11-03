# Command List

## Motor Parameters

| **Command** | **Get / Set** | **Description** | **Axis range** | **Set if remote** | **Parameter range** |
|--------|-----|------------------------------------|---------|-------|---------|
| MP_CSCA | G/S | **C**urrent **SCA**le: overall scale factor or all currents | valid | yes | 32..255 |
| MP_CRAN | G/S | **C**urrent **RAN**ge. 0-\>1A, 1-\>2A, 2-\> 3A, 3-\> 3A | valid | yes | 0..3 |
| MP_CRUN | G/S | **C**urrent **RUN**: scale factor for operating current | valid | yes | 0..31 |
| MP_CHOL | G/S | **C**urrent **HOL**d: scale factor for holding current | valid | yes | 0..31 |
| MP_MMIC | G/S | **M**ode **MIC**rosteps: step size in 2\^MMIC MS. 0-\> native 256 microsteps, 8-\> full step (no microstepping) | valid | yes | 0..8 |
| MP_MINV | G/S | **M**ode **INV**ert: inverts the axis direction, sets the shaft parameter | valid | yes | 0 or 1 |
| MP_MTOF | G/S | **M**ode **TOF**f: off time . 0-\> driver disabled, 5 -\> enabled | valid | yes | 0..10 |
| MP_MSGE | G/S | **M**ode **SG** **E**nable: flag to enable stallGuard2 | valid | yes | 0 or 1 |
| MP_MSGT | G/S | **M**ode **SG** **T**hreshold for stallguard | valid | yes | -64..63 |
| MP_MTCT | G/S | **M**ode **TC**ool **T**hreshold: lower limit for stallguard velocity | valid | yes | \>0 |
| MP_HMOD | G/S | **H**oming **MOD**e: 0-\>disabled, 1-\>limits, 2-\>index | valid | yes | 0..2 |
| MP_HDIR | G/S | **H**oming **DIR**ection: -1=neg, 1=pos | valid | yes | -1 or 1 |
| MP_HVEL | G/S | **H**oming **VEL**ocity: units are internal (\~4/3 microsteps/s) | valid | yes | 0 to VMXV |
| MP_HSST | G/S | **H**oming **S**oft **ST**op: 0-\>hard stop, 1-\>soft stop | valid | yes | 0 or 1 |
| MP_HNEV | G/S | **H**oming Index(**N**) **EV**ent: 0-\>event during pos N, 1-\> event on N rising edge, 2-\> falling, 3-\>any edge | valid | yes | 0..3 |
| MP_RSEV | G/S | **R**ate **SE**t **V**elocity: units are internal (\~4/3 microsteps/s) | valid | yes | 0 to VMXV |
| MP_RMXV | G/S | **R**ate **M**a**X** **V**elocity: units are internal (\~4/3 microsteps/s) | valid | yes | \>0 |
| MP_RSEA | G/S | **R**ate **SE**t **A**cc: internal units (\~(4/3)\^2 microsteps/s\^2) | valid | yes | 0 to VMXA |
| MP_RMXA | G/S | **R**ate **M**a**X** **A**cc: internal units (\~(4/3)\^2 microsteps/s\^2) | valid | yes | \>0 |
| MP_ECON | G/S | **E**ncoder **CON**stant: sets the encoder resolution, sign sets the direction (0 not present) | valid | yes | any |
| MP_EDEV | G/S | **E**ncoder **DEV**iation: max deviation between encoder and motor position before flag is raised. 0-\>disabled | valid | yes | \>0 |
| MP_ETOL | G/S | **E**ncoder **TOL**erance: tolerance window for closed-loop operation | valid | yes | \>0 |
| MP_EMAX | G/S | **E**ncoder **MAX** iterations : max number of pull-in tries. 0-\>Closed-loop, 1-\>Open-loop, \>1-\>Open-loop with pull-ins | valid | yes | \>=0 |
| MP_ERST | G/S | **E**ncoder **R**e**S**e**T** X after pull-ins | valid | yes | 0 or 1 |
| MP_SLEN | G/S | **S**witch **L**eft **EN**abled | valid | yes | 0 or 1 |
| MP_SREN | G/S | **S**witch **R**ight **EN**abled | valid | yes | 0 or 1 |
| MP_SLPO | G/S | **S**witch **L**eft **PO**larity: 1-\> high level stops motor; 0-\> low level stops | valid | yes | 0 or 1 |
| MP_SRPO | G/S | **S**witch **R**ight**PO**larity: 1-\> high level stops motor; 0-\> low level stops | valid | yes | 0 or 1 |
| MP_SSWP | G/S | **S**witch **SW**a**P**: 1-\>swap left and right role | valid | yes | 0 or 1 |
| MP_LENC | G/S | **L**imit **ENC**oder: 0-\>use XACT for the limits, 1-\>use encoder for limits | valid | yes | 0 or 1 |
| MP_LLEN | G/S | **L**imit **L**eft **EN**abled | valid | yes | 0 or 1 |
| MP_LREN | G/S | **L**imit **R**ight **EN**abled | valid | yes | 0 or 1 |
| MP_LLPS | G/S | **L**imit **L**eft **P**o**S**ition | valid | yes | any |
| MP_LRPS | G/S | **L**imit **R**ight **P**o**S**ition | valid | yes | any |
| MP_TDEV | G/S | **T**ype **DEV**ice: 0=None, 1=Sim, 2=TMC | valid | yes | 0..2 |
| MP_TAXI | G/S | **T**ype **AXI**s: 0=Undef, 1=X, 2=Y, 3=Z, 4=Aux | valid | yes | 0..4 |

Note that setting these parameters does not perform error or range
checks. Only when the parameters are used (most commonly with the config
command) will errors be generated.v

## Remote Parameters

| **Command** | **Get / Set** | **Description** | **Axis range** | **Set if remote** | **Parameter range** |
|--------|-----|------------------------------------|---------|-------|---------|
| RP_ENAB | G/S | **ENAB**led: 0-\>computer control, 1-\>remote control | -1 or active | yes | 0 or 1 |
| RP_JDIR | G/S | **J**oystick **DIR**ection | valid | yes | -1 or 1 |
| RP_JMAX | G/S | **J**oystick **MAX** velocity | valid | yes | 0 to VMXV |
| RP_EDIR | G/S | **E**ncoder **DIR**ection | valid | yes | -1 or 1 |
| RP_ESTP | G/S | **E**ncoder **ST**e**P** size | valid | yes | \>0 |

## Motor Status

| **Command** | **Get / Set** | **Description** | **Axis range** | **Set if remote** | **Value range** |
|--------|-----|------------------------------------|---------|-------|---------|
| MS_XACT | G/S | **XACT** register: Actual position | active | no | any |
| MS_XTAR | G/S | **XTAR** register: Target position | active | no | any |
| MS_XENC | G/S | **XENC** register: Encoder position | active | no | any |
| MS_VELO | G/S | **VELO** register: Current set velocity | active | no | 0 to VMXV |
| MS_ACCE | G/S | **ACCE** register: Current set acceleration/deceleration | active | no | 0 to VMXA |
| MS_ENAB | G/S | Motor **ENAB**le flag | -1 or active | yes | 0 or 1 |
| MS_TEMP | G | TMC IC **TEMP**erature in degrees C | active |  |  |
| MS_PULL | G | Most recent number of **PULL**-in tries | active |  |  |

## Motor Commands

| **Command** | **Get / Set** | **Description** | **Axis range** | **Set if remote** | **Value range** |
|--------|-----|------------------------------------|---------|-------|---------|
| MC_HOME | S | **HOME**s the motor. This can only be canceled by disabling the axis | active & enabled | no | No value |
| MC_CONF | S | **CONF**igures the axis | -1 or active | yes | No value |
| MC_SCLR | S | **S**tatus **CL**ea**R**: clears the error status registers | -1 or active | yes | No value |
| MC_MPOS | S | **M**ove axis to **POS**ition | active & enabled | no | any |
| MC_MVEL | S | **M**ove axis at **VEL**ocity | active & enabled | no | 0 to VMXV |
| MC_POSR | G | Is **POS**ition **R**eached? 0-\>No, 1-\>Yes | -1 or active |  |  |
| MC_STAT | G | Retrieve the **STAT**us flags (see below for format) | active |  |  |

### Status bits:

| **Status bit** | **Flag**                                |
|----------------|-----------------------------------------|
| 11             | Motor enabled                           |
| 10             | Position reached status (not event)     |
| 9              | Motor moving (velocity $\neq$ 0)        |
| 8              | Right (positive) latch available status |
| 7              | Left (negative) latch available status  |
| 6              | Encoder deviation warning               |
| 5              | Stallguard event occurred               |
| 4              | Stallguard status active                |
| 3              | Right (positive) virtual stop active    |
| 2              | Left (negative) virtual stop active     |
| 1              | Right (positive) limit stop active      |
| 0              | Left (negative) limit stop active       |

## Register commands

| **Command** | **Get / Set** | **Description** | **Axis range** | **Set if remote** | **Value range** |
|--------|-----|------------------------------------|---------|-------|---------|
| MC_DREG | G/S | **D**irectly gets or sets **REG**ister values in the TMC IC | active | yes | any|

Note: the command format here is SMC_DREG\<motor\>,\<reg\>,\<val\> and the get response is MC_DREG\<motor\>=\<val\>

## Pico commands (no axis qualifier)

| **Command** | **Get / Set** | **Description** | **Value range** |
|---------|-----|-------------------------------------------------|---------|
| \*IDN? | \- | Returns ID string “Stage Driver Pico” |  |
| PC_VERS | G | Returns the software **VERS**ion |  |
| PC_NDEV | G | Get **N**umber of possible **DEV**ices (MAXNUMMOTORS) |  |
| PC_EMSG | G | Returns **E**rror **M**e**S**sa**G**e |  |
| PC_SAFL | S | **SA**ve the configuration to **FL**ash memory. Returns "ERROR=0" if successful. | No value |
