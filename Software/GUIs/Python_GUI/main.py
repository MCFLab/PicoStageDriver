"""Small application entry point for the Stage Driver GUI.

This module creates the top-level Panel and Controller and starts the
Tk mainloop via Controller.run().
"""

from stage_driver_controller import Controller
from stage_driver_view import Panel


def main() -> None:
    """Create the UI panel and controller, then run the GUI loop."""
    panel = Panel()
    controller = Controller(panel)
    controller.run()


if __name__ == "__main__":
    main()
