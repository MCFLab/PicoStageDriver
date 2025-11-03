"""Lookup table mapping Pico parameter IDs to external friendly names.

The driver communicates using short Pico parameter identifiers (e.g.
"MP_CSCA"). This module provides a dictionary mapping those identifiers to
more descriptive names used by the rest of the codebase.

The mapping is intentionally simple and static.
"""

table = {
    "MP_CSCA": "CurrScaler",
    "MP_CRAN": "CurrRange",
    "MP_CRUN": "CurrRun",
    "MP_CHOL": "CurrHold",
    "MP_MMIC": "ModeMicroStep",
    "MP_MINV": "ModeInvDir",
    "MP_MTOF": "ModeTOff",
    "MP_MSGE": "ModeSGEnable",
    "MP_MSGT": "ModeSGT",
    "MP_MTCT": "ModeTCT",
    "MP_HMOD": "HomingMode",
    "MP_HDIR": "HomingDirection",
    "MP_HVEL": "HomingVelocity",
    "MP_HSST": "HomingSoftStop",
    "MP_HNEV": "HomingIndexEvent",
    "MP_RSEV": "RateSetVelocity",
    "MP_RMXV": "RateMaxVelocity",
    "MP_RSEA": "RateSetAcc",
    "MP_RMXA": "RateMaxAcc",
    "MP_ECON": "EncConstant",
    "MP_EDEV": "EncDeviation",
    "MP_ETOL": "EncLoopTolerance",
    "MP_EMAX": "EncLoopMax",
    "MP_ERST": "EncResetXafterCL",
    "MP_SLEN": "SwitchLeftEnable",
    "MP_SREN": "SwitchRightEnable",
    "MP_SLPO": "SwitchLeftPolarity",
    "MP_SRPO": "SwitchRightPolarity",
    "MP_SSWP": "SwitchSwap",
    "MP_LENC": "LimEncoder",
    "MP_LLEN": "LimLeftEnable",
    "MP_LREN": "LimRightEnable",
    "MP_LLPS": "LimLeftPosition",
    "MP_LRPS": "LimRightPosition",
    "MP_TDEV": "TypeDevice",
    "MP_TAXI": "TypeAxis",

    "RP_ENAB": "RemoteEnabled",
    "RP_JDIR": "JoystickDirection",
    "RP_JMAX": "JoystickMax",
    "RP_EDIR": "EncoderDirection",
    "RP_ESTP": "EncoderStepSize", 

    "MS_XACT": "ActualPosition",
    "MS_XTAR": "TargetPosition",
    "MS_XENC": "EncoderPosition",
    "MS_VELO": "TargetVelocity",
    "MS_ACCE": "TargetAcc",
    "MS_ENAB": "Enabled",
    "MS_TEMP": "Temperature", # get only
    "MS_PULL": "LastPullInTries", # get only

    "MC_HOME": "FindHome", # set only
    "MC_CONF": "Config", # set only
    "MC_SCLR": "StatusClear", # set only
    "MC_MPOS": "MoveToPosition", # set only
    "MC_MVEL": "MoveAtVelocity", # set only
    "MC_POSR": "HasPositionReached", # get only
    "MC_STAT": "GetStatus", # get only

    "PC_IDST": "IDString", # get only
    "PC_NDEV": "NumberofDevices", # get only
    "PC_VERS": "Version", # get only
    "PC_SAFL": "SaveToFlash" # set only
}

