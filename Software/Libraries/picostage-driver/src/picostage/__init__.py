"""picostage package

Lightweight wrapper package exposing the Pico stage driver client.

This package exposes a single high-level class, :class:`PicoStage`, used to
connect to and operate a Pico stage driver over a VISA serial connection.
"""

from .stage_driver import PicoStage

__all__ = ["PicoStage"]