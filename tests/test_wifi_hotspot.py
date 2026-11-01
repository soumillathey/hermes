"""test_wifi_hotspot.py — Unit tests for nmcli hotspot AP and watchdog lifecycle."""
from __future__ import annotations

import unittest
from unittest.mock import MagicMock, patch

from src.devices.wifi.activity import is_watchdog_suspended, suspend_watchdog
from src.devices.wifi.hotspot import (
    is_hotspot_active,
    start_emergency_hotspot,
    start_hotspot,
    stop_emergency_hotspot,
    stop_hotspot,
)
from src.devices.wifi.runner import SIMULATION_STATE
from src.devices.wifi.watchdog import (
    is_watchdog_running,
    start_wifi_watchdog,
    stop_wifi_watchdog,
)


class TestWiFiHotspot(unittest.TestCase):
    def setUp(self) -> None:
        SIMULATION_STATE["enabled"] = True
        SIMULATION_STATE["hotspot_active"] = False

    def tearDown(self) -> None:
        stop_wifi_watchdog()
        stop_hotspot()

    def test_hotspot_lifecycle_simulated(self) -> None:
        self.assertFalse(is_hotspot_active())
        self.assertTrue(start_hotspot("hermes", "12345678"))
        self.assertTrue(is_hotspot_active())
        self.assertTrue(stop_hotspot())
        self.assertTrue(stop_hotspot())
        self.assertFalse(is_hotspot_active())

    @patch("src.devices.wifi.portal_service.portal_service.start", return_value=True)
    @patch("src.devices.wifi.portal_service.portal_service.stop", return_value=None)
    def test_emergency_hotspot_lifecycle(self, mock_stop: MagicMock, mock_start: MagicMock) -> None:
        self.assertTrue(start_emergency_hotspot("hermes", "12345678"))
        self.assertTrue(is_hotspot_active())
        self.assertTrue(stop_emergency_hotspot())
        self.assertTrue(stop_emergency_hotspot())
        self.assertFalse(is_hotspot_active())

    def test_watchdog_suspension_and_lifecycle(self) -> None:
        suspend_watchdog(1.0)
        self.assertTrue(is_watchdog_suspended())
        self.assertTrue(start_wifi_watchdog(interval=0.1))
        self.assertTrue(is_watchdog_running())
        stop_wifi_watchdog()
        self.assertFalse(is_watchdog_running())


if __name__ == "__main__":
    unittest.main()
