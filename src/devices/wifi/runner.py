"""runner.py — Serialized nmcli subprocess execution with simulation mode."""
from __future__ import annotations

import logging
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

from src.devices.wifi.activity import is_nmcli_busy, nmcli_lock

logger = logging.getLogger(__name__)

SIMULATION_STATE: dict[str, Any] = {
    "enabled": False,
    "active_ssid": "Simulated_Office_5G",
    "hotspot_active": False,
}


def is_nmcli_available() -> bool:
    """Check if nmcli binary is present on system PATH."""
    return shutil.which("nmcli") is not None


def get_wifi_interface() -> str:
    """Detect primary wireless network interface."""
    net_path = Path("/sys/class/net")
    if net_path.is_dir():
        for iface in net_path.iterdir():
            if (iface / "wireless").is_dir() or iface.name.startswith(("wl", "wlan")):
                return iface.name
    return "wlan0"


def run_nmcli(args: list[str], timeout: float = 15.0, warn: bool = True) -> tuple[int, str]:
    """Execute nmcli command with exclusive locking."""
    if not is_nmcli_available() or SIMULATION_STATE["enabled"]:
        return 0, "Simulation OK"
    pfx = ["sudo", "-n"] if getattr(os, "geteuid", lambda: 0)() != 0 and shutil.which("sudo") else []
    with nmcli_lock:
        try:
            res = subprocess.run(pfx + ["nmcli"] + args, capture_output=True, text=True, timeout=timeout, check=False)
            out = (res.stdout or res.stderr).strip()
            if res.returncode != 0 and warn:
                logger.warning("nmcli error (%d): %s", res.returncode, out)
            return res.returncode, out
        except subprocess.TimeoutExpired:
            return 124, "nmcli command timed out"
        except (subprocess.SubprocessError, OSError) as e:
            return 1, str(e)


__all__ = ["SIMULATION_STATE", "get_wifi_interface", "is_nmcli_available", "is_nmcli_busy", "run_nmcli"]
