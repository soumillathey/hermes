/**
 * wifi_ap.cpp
 * Access Point setup, WebServer route registration, and config page serving.
 */

#include "wifi_portal.h"
#include "wifi_handlers.h"
#include "../config/config_manager.h"
#include "../ui/config_page.h"
#include "../led/rgb_led.h"
#include "../led/seven_segment.h"
#include <WiFi.h>

SystemState currentState = STATE_INIT;
WebServer server(80);
const unsigned long WIFI_CHECK_INTERVAL = 20000;

static void handleRoot() {
  String html = String(CONFIG_HTML);
  html.replace("%SSID%",       wifi_ssid);
  html.replace("%PASSWORD%",   wifi_password);
  html.replace("%SB_EMAIL%",   supabase_email);
  html.replace("%SB_PASSWORD%",supabase_password);
  html.replace("%CENTER_ID%",  String(supabase_center_id));
  html.replace("%MIN_WEIGHT%", String(supabase_weight_threshold, 1));
  html.replace("%MAC%",        WiFi.macAddress());
  server.send(200, "text/html", html);
}

void setupWebServer() {
  server.on("/",     HTTP_GET,  handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() { server.send(404, "text/plain", "Not Found"); });
  server.begin();
}

void startAPMode() {
  currentState = STATE_AP_MODE;
  setRGBRed();
  displayErrorCode(SEG_CODE_AP_MODE);

  String mac = WiFi.macAddress();
  String suffix = mac.substring(mac.length() - 5);
  suffix.replace(":", "");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(("Gluvok_WeighTrix_" + suffix).c_str(), "");
  setupWebServer();
}
