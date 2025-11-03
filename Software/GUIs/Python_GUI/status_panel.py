"""Status display panel showing motor LEDs, positions and temperature.

This panel contains a vertical column of indicator LEDs, numeric
displays for Actual/Target/Encoder positions, deviation and
temperature. It optionally polls a provided ``on_update`` callable at a
period defined by ``interval``.
"""

import tkinter as tk
from panel_controls import NumDisplay


class StatusPanel(tk.Frame):
    """Panel that displays status LEDs and numeric readouts for a motor.

    Parameters
    - root: parent widget
    - interval: polling interval in milliseconds used when auto_update is True
    - auto_update: if truthy, the panel will call `self.on_update` periodically
    - on_update: optional callable invoked before the automatic redisplay
    """

    def __init__(self, root, interval=500, auto_update=0, on_update=None):
        super().__init__(root)
        self.on_update = on_update
        self.leds = []
        self.numLeds = 12
        self.create_leds()
        self.auto_update = auto_update
        self.update_interval = interval
        if auto_update:
            self.update()

        self.led_frame.pack(anchor='w', padx=10)
        self.xact = NumDisplay(self, 'XAct')
        self.xact.pack(anchor='w', padx=10)
        self.xtarget = NumDisplay(self, 'XTarget')
        self.xtarget.pack(anchor='w', padx=10)
        self.enc = NumDisplay(self, 'Enc')
        self.enc.pack(anchor='w', padx=10)
        self.dev = NumDisplay(self, 'Deviation')
        self.dev.pack(anchor='w', padx=10)
        self.temp = NumDisplay(self, 'Temp')
        self.temp.pack(anchor='w', padx=10)


    def create_leds(self):
        """Create a column of small oval LEDs with textual labels."""
        self.led_frame = tk.LabelFrame(self)
        for i in reversed(range(self.numLeds)):
            labels = ["stop_L", "stop_R", "virt_L", "virt_R", "SG_stat", "SG_evnt", "encDev",
                      "latch_L", "latch_R", "isMov", "atPos", "enabled"]
            row = i  # Top to bottom: high bit to bit 0
            canvas = tk.Canvas(self.led_frame, width=15, height=15, highlightthickness=0)
            canvas.grid(row=row, column=0, pady=0)
            led = canvas.create_oval(3, 3, 12, 12, fill='grey')
            self.leds.append((canvas, led))
            # Label to the right of the LED
            label = tk.Label(self.led_frame, text=f"{self.numLeds-1-i}: {labels[self.numLeds-1-i]}", font=("Arial", 8))
            label.grid(row=row, column=1, padx=(5, 0), sticky='w')

    def update_leds(self, byte):
        """Update LED colors based on the bits of ``byte`` (LSB bit 0)."""
        for i in range(self.numLeds):
            canvas, led = self.leds[i]
            bit_on = (byte >> i) & 1
            color = 'lime' if bit_on else 'grey'
            canvas.itemconfig(led, fill=color)

    def update_positions(self, xact, xtarget, enc):
        """Update numeric displays for actual, target and encoder positions.

        Also computes and displays deviation (encoder - actual).
        """
        self.xact.numInt.set(xact)
        self.xtarget.numInt.set(xtarget)
        self.enc.numInt.set(enc)
        self.dev.numInt.set(enc - xact)

    def update_status(self, temp):
        """Update the temperature display."""
        self.temp.numInt.set(temp)

    def update(self):
        """Optional periodic update hook.

        If ``auto_update`` is enabled, this method re-schedules itself
        after ``update_interval`` milliseconds. The method also calls
        ``self.on_update`` if provided so the controller can poll
        hardware before the view refreshes.
        """
        if self.on_update:
            self.on_update()
        # Call this method again after the interval time (in milliseconds)
        if self.auto_update:
            self.after(self.update_interval, self.update)

