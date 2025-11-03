"""Simple logging handler that routes log records to a UI text callback.

The handler expects a callable `text_widget` that accepts a single
string argument. This keeps the handler independent of the concrete
widget type used by the GUI (it could be a function that appends to a
Text widget, for example).
"""

import logging


# --- Custom TextHandler ---
class TextHandler(logging.Handler):
    """Logging handler that forwards formatted messages to a callable.

    Parameters
    - text_widget: callable(msg: str) used to display/append text in the UI
    - logger_name: name of the logger to attach this handler to
    """

    def __init__(self, text_widget, logger_name="StageDriver"):
        super().__init__()
        self.text_widget = text_widget
        self.logger = logging.getLogger(logger_name)
        # Use a simple message-only formatter by default
        self.setFormatter(logging.Formatter('%(message)s'))
        self.logger.addHandler(self)
        self.logger.setLevel(logging.DEBUG)

    def emit(self, record):
        """Format and send a log record to the configured text widget/callback."""
        msg = self.format(record)
        # calling the provided callback with the message keeps the handler
        # decoupled from a concrete Tk widget instance.
        self.text_widget(msg)



