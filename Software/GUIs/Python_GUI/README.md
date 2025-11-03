# Stage Driver GUI

Small Tkinter GUI used to interact with PicoStage hardware. The
repository contains a lightweight MVC split: `stage_driver_view` for the
UI, `stage_driver_controller` for logic and `picostage` (external) for
hardware access.

What is included

- `panel_controls.py`, `status_panel.py`, `input_history_panel.py` — UI
  widgets and small controls.
- `stage_driver_view.py` — the main application window (Panel).
- `stage_driver_controller.py` — controller that wires the Panel to the
  PicoStage backend.
- `logging_texthandler.py` — helper that routes logging messages to the
  UI.

Quick start (development)

1. Create or activate a Python environment (this project was developed
   with Python 3.8+).

2. Run the GUI (requires the PicoStage package and hardware or a mock):

```powershell
py -3 main.py
```

Notes

- The GUI code uses tkinter widgets; tests instantiate widgets headless
  (no mainloop) and should run on systems with tkinter available.
- If your environment does not have `py` launcher, use your `python`
  executable instead.
