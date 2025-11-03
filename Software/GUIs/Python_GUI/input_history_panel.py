"""A small input panel with history and a multi-line output area.

This widget is used for entering free-form commands/text. It keeps a
simple in-memory history list and exposes a callback `on_submit` which
is invoked with the submitted text when the user presses Return.
"""

import tkinter as tk
from tkinter import ttk


class InputHistoryPanel(tk.Frame):
    """Input panel with a Combobox history and a Text output box.

    Parameters
    - root: parent widget
    - on_submit: optional callable(text) invoked when the user submits
      input (presses Return)
    """

    def __init__(self, root, on_submit=None, **kwargs):
        super().__init__(root, **kwargs)
        self.root = root

        self.on_submit = on_submit
        # simple list-based history and index for navigation
        self.history = []
        self.history_index = -1

        # Combobox for input + dropdown history
        self.input_var = tk.StringVar()
        self.combobox = ttk.Combobox(root, textvariable=self.input_var)
        self.combobox.pack(fill=tk.X, padx=5, pady=5)
        self.combobox.bind("<Return>", self.submit_input)
        self.combobox.bind("<Up>", self.navigate_up)
        self.combobox.bind("<Down>", self.navigate_down)

        # Output Text widget (multi-line)
        self.output_text = tk.Text(root, height=10, wrap=tk.WORD)
        self.output_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

    def update_combobox(self):
        """Refresh combobox dropdown values from the history list."""
        self.combobox['values'] = self.history

    def navigate_up(self, event):
        """Navigate up in the input history and place the value in the combobox."""
        if self.history:
            self.history_index = max(0, self.history_index - 1)
            self.input_var.set(self.history[self.history_index])
            self.combobox.icursor(tk.END)  # move cursor to end
            return "break"

    def navigate_down(self, event):
        """Navigate down in the input history."""
        if self.history:
            self.history_index = min(len(self.history) - 1, self.history_index + 1)
            self.input_var.set(self.history[self.history_index])
            self.combobox.icursor(tk.END)  # move cursor to end
        return "break"

    def submit_input(self, event=None):
        """Handle input submission: append to history, call callback and update output."""
        text = self.input_var.get().strip()
        if text:
            self.history.append(text)
            self.history_index = len(self.history)
            self.update_combobox()

            self.append_output(text)
            self.input_var.set("")

            if self.on_submit:
                self.on_submit(text)

    def get_input_history(self):
        """Return a copy of the input history list."""
        return self.history.copy()

    def set_output_lines(self, lines):
        """Set the entire output Text widget to the provided lines (list of str)."""
        self.output_text.delete("1.0", tk.END)
        self.output_text.insert(tk.END, "\n".join(lines))
        self.output_text.see(tk.END)

    def append_output(self, line):
        """Append a single line to the output Text widget and scroll to end."""
        self.output_text.insert(tk.END, line + "\n")
        self.output_text.see(tk.END)
