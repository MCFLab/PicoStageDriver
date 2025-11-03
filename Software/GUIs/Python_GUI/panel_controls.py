"""UI control widgets used by the Stage Driver GUI.

This module provides small reusable ttk/tk widgets used by
`stage_driver_view.Panel`: numeric controls, displays, sliders and
parameter panels.

Widgets:
- NumCtrl: labeled Spinbox for integer selection
- NumDisplay: small labeled Entry bound to an IntVar
- SliderCtrl: labeled horizontal Scale with callbacks
- ParamPanel: a horizontal panel of NumDisplay controls
"""

import tkinter as tk
from tkinter import ttk

class NumCtrl(ttk.Frame):
    """A small labeled Spinbox control bound to an IntVar.

    Parameters
    - parent: parent widget
    - text: label text
    - minVal: minimum Spinbox value
    - maxVal: maximum Spinbox value
    - inc: increment step
    """

    def __init__(self, parent, text, minVal, maxVal, inc):
        super().__init__(parent)
        self.numInt = tk.IntVar(value=0)
        ttk.Label(self, text=text).pack(anchor='w')
        ttk.Spinbox(self, width=10,
                    from_=minVal,
                    to=maxVal,
                    increment=inc,
                    textvariable=self.numInt).pack(pady=1)


class NumDisplay(ttk.Frame):
    """A labeled Entry widget bound to an IntVar for displaying integers.

    Parameters
    - parent: parent widget
    - text: label text
    - default: initial integer value
    """

    def __init__(self, parent, text, default=0):
        super().__init__(parent)
        self.numInt = tk.IntVar(value=default)
        ttk.Label(self, text=text).pack(anchor='w')
        self.entry = tk.Entry(self, width=10, textvariable=self.numInt)
        self.entry.pack(pady=1)


class SliderCtrl(ttk.Frame):
    """A labeled horizontal Scale bound to an IntVar and callback.

    Parameters
    - parent: parent widget
    - text: label for the scale
    - minVal, maxVal: range for the scale
    - on_change: callback called by the Scale widget (string arg by Tk)
    """

    def __init__(self, parent, text, minVal, maxVal, on_change):
        super().__init__(parent)
        self.on_change = on_change
        self.velocity = tk.IntVar(value=0)
        self.scale = tk.Scale(self,
                              from_=minVal, to=maxVal,
                              orient=tk.HORIZONTAL,
                              variable=self.velocity,
                              resolution=1,
                              command=self.on_change,
                              length=200,
                              label=text
                              )
        self.scale.pack()

    def set_value(self, value):
        """Set the current value of the slider (int)."""
        self.velocity.set(value)

    def get_value(self):
        """Return the current integer value of the slider."""
        return self.velocity.get()

class ParamPanel(ttk.Frame):
    """Horizontal panel that contains multiple `NumDisplay` controls.

    Each parameter control is created as an attribute named after the
    supplied control name. The panel keeps the list of names in
    ``self.names`` so callers (for example `Panel.get_panel_params`) can
    iterate over them.
    """

    def __init__(self, parent, id, ctrlArr):
        super().__init__(parent)
        self.id = id
        self.names = ctrlArr
        frame = ttk.Frame(parent)
        label = ttk.Label(frame, text=f"{id}:")
        label.pack(side='left', anchor='s', padx=10)
        for i, name in enumerate(self.names):
            # remove optional 'parm' prefix to create a friendly label
            setattr(self, name, NumDisplay(frame, name.removeprefix("parm")))
            getattr(self, name).pack(side='left', anchor='s', padx=10)
        frame.pack(anchor='w', pady=5)
