

  /**
   * @file DefaultParams.h
   * @brief Contains default parameter arrays for stepper motor, safe motor, and remote control configurations.
   *
   * - For the definition, see Prameters.h
   */
  
  static int32_t defaultStepperMotorParams[] = {
    // CurrentParams
    128,      // "CSCA"; Scale: overall scale factor [0 (full scale, or 32..255)]
    1,        // "CRAN"; Range: 0->1A, 1->2A, 2-> 3A, 3-> 3A
    12,       // "CRUN"; Run: scale factor for operating current [0..31]
    8,        // "CHOL"; Hold: scale factor for holding current [0..31]
    // ModeParams
    3,        // "MMIC"; MICrosteps: step size in  2^MMIC MS (0-> native 256 MS, 8-> full step)
    0,        // "MINV"; INVert direction: inverts the axis direction, sets the shaft parameter
    5,        // "MTOF"; TOff: off time (0-> driver disabled, set to 5 otherwise)
    1,        // "MSGE"; SG Enable: flag to enable stallGuard2
    0,        // "MSGT"; SG Threshold for stallguard
    145,      // "MTCT"; TCoolThres (lower limit for stallguard velocity)
    // HomingParams
    1,        // "HMOD"; MODe: homing mode (0->disabled, 1->limits, 2->index)
    1,        // "HDIR"; DIRection: determines the homing direction (-1->neg, 1->pos)
    4000,     // "HVEL"; VELocity
    1,        // "HSST"; Use Soft STop
    1,        // "HNEV"; Index(N) event (0..3)
    // RateParams
    32000,    // "RSEV"; SEt Vel: units are internal (~4/3 microsteps/s)
    64000,    // "RMXV"; MaX Vel: units are internal (~4/3 microsteps/s)
    4000,     // "RSEA"; SEt Acc: internal units (~(4/3)^2 microsteps/s^2)
    5000,     // "RMXA"; MaX Acc: internal units (~(4/3)^2 microsteps/s^2)
    // EncoderParams
    71536,    // "ECON"; Constant: sets the encoder resolution, sign sets the direction (0 not present)
    500,      // "EDEV"; Deviation: max deviation between encoder and motor position before flag is raised
    5,        // "ETOL"; TOLerance: tolerance window for closed-loop operation
    1,        // "EMAX"; MAX iterations : max number of pull-in tries for closed-loop operation
    0,        // "ERST"; ReSeT X after closed-loop move
    // SwitchParams
    1,        // "SLEN"; Left ENabled 
    1,        // "SREN"; Right ENabled 
    1,        // "SLPO"; Left POlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
    1,        // "SRPO"; RightPOlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
    0,        // "SSWP"; Swap: 1->swap left and right role
    // LimitsParam
    0,        // "LENC"; ENCoder mode (use encoder, rather than XACT for the virtual limits)  
    0,        // "LLEN"; Left ENabled  
    0,        // "LREN"; Right ENabled 
    -100000,  // "LLPS"; Left PoSition 
    100000    // "LRPS"; Right PoSition 
  };

  static int32_t defaultSafeMotorParams[] = {
    // CurrentParams
    128,      // "CSCA"; Scale: overall scale factor [0 (full scale, or 32..255)]
    0,        // "CRAN"; Range: 0->1A, 1->2A, 2-> 3A, 3-> 3A
    0,        // "CRUN"; Run: scale factor for operating current [0..31]
    0,        // "CHOL"; Hold: scale factor for holding current [0..31]
    // ModeParams
    3,        // "MMIC"; MICrosteps: step size in  2^MMIC MS (0-> native 256 MS, 8-> full step)
    0,        // "MINV"; INVert direction: inverts the axis direction, sets the shaft parameter
    0,        // "MTOF"; TOff: off time (0-> driver disabled, set to 5 otherwise)
    0,        // "MSGE"; SG Enable: flag to enable stallGuard2
    0,        // "MSGT"; SG Threshold for stallguard
    0,        // "MTCT"; TCoolThres (lower limit for stallguard velocity)
    // HomingParams
    0,        // "HMOD"; MODe: homing mode (0->disabled, 1->limits, 2->index)
    1,        // "HDIR"; DIRection: determines the homing direction (-1->neg, 1->pos)
    0,        // "HVEL"; VELocity
    0,        // "HSST"; Use Soft STop
    1,        // "HNEV"; Index(N) event (0..3)
    // RateParams
    0,        // "RSEV"; SEt Vel: units are internal (~4/3 microsteps/s)
    0,        // "RMXV"; MaX Vel: units are internal (~4/3 microsteps/s)
    0,        // "RSEA"; SEt Acc: internal units (~(4/3)^2 microsteps/s^2)
    0,        // "RMXA"; MaX Acc: internal units (~(4/3)^2 microsteps/s^2)
    // EncoderParams
    0,        // "ECON"; Constant: sets the encoder resolution, sign sets the direction (0 not present)
    0,        // "EDEV"; Deviation: max deviation between encoder and motor position before flag is raised
    1,        // "ETOL"; TOLerance: tolerance window for closed-loop operation
    1,        // "EMAX"; MAX iterations : max number of pull-in tries for closed-loop operation
    0,        // "ERST"; ReSeT X after closed-loop move
    // SwitchParams
    0,        // "SLEN"; Left ENabled 
    0,        // "SREN"; Right ENabled 
    1,        // "SLPO"; Left POlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
    1,        // "SRPO"; RightPOlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
    0,        // "SSWP"; Swap: 1->swap left and right role
    // LimitsParam
    0,        // "LENC"; ENCoder mode (use encoder, rather than XACT for the virtual limits)  
    1,        // "LLEN"; Left ENabled  
    1,        // "LREN"; Right ENabled 
    -1000,    // "LLPS"; Left PoSition 
    1000      // "LRPS"; Right PoSition 
  };

  static int32_t defaultRemoteParams[] = {
      0,      // "ENAB"; remote enabled
      1,      // "JDIR"; joystick direction
      1000,   // "JMAX"; joystick max val
      1,      // "EDIR"; encoder direction
      10      // "ESTP"; encoder step size
  };
