/**
 * wifi_handlers.cpp
 * Handles the /save POST form submission from the config portal.
 * Parses credentials, saves to NVS, and transitions to STA mode.
 */

#include "wifi_portal.h"
#include "wifi_handlers.h"
#include "../config/config_manager.h"
#include "../ota/ota_updater.h"
#include "../ui/save_page.h"
#include <WiFi.h>

void handleSave() {
  if (!server.hasArg("ssid") || !server.hasArg("center_id") ||
      !server.hasArg("min_weight") || !server.hasArg("sb_email") ||
      !server.hasArg("sb_password")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  wifi_ssid      = server.arg("ssid");        wifi_ssid.trim();
  wifi_password  = server.arg("password");    wifi_password.trim();
  supabase_center_id        = server.arg("center_id").toInt();
  supabase_weight_threshold = server.arg("min_weight").toDouble();
  supabase_email    = server.arg("sb_email");    supabase_email.trim();
  supabase_password = server.arg("sb_password"); supabase_password.trim();

  saveSettings(wifi_ssid, wifi_password, supabase_center_id,
               supabase_weight_threshold, supabase_email, supabase_password);

  String successHtml = String(SAVE_HTML);
  successHtml.replace("%SSID%", wifi_ssid);
  server.send(200, "text/html", successHtml);

  delay(1000);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  if (connectToWiFi()) {
    currentState = STATE_CONNECTED_NORMAL;
    checkForUpdates();
  } else {
    startAPMode();
  }
}
