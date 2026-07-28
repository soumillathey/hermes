#include "wifi_portal.h"
#include "../config/config_manager.h"
#include "../ota/ota_updater.h"
#include "../ui/config_page.h"
#include "../ui/save_page.h"
#include <WiFi.h>

SystemState currentState = STATE_INIT;
WebServer server(80);
const unsigned long WIFI_CHECK_INTERVAL = 20000;

static void handleRoot() {
  String html = String(CONFIG_HTML);
  html.reserve(sizeof(CONFIG_HTML) + 128);

  html.replace("%SSID%", wifi_ssid);
  html.replace("%PASSWORD%", wifi_password);
  html.replace("%SB_EMAIL%", supabase_email);
  html.replace("%SB_PASSWORD%", supabase_password);
  html.replace("%CENTER_ID%", String(supabase_center_id));
  html.replace("%MIN_WEIGHT%", String(supabase_weight_threshold, 1));
  html.replace("%MAC%", WiFi.macAddress());

  server.send(200, "text/html", html);
}

static void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("center_id") && server.hasArg("min_weight") &&
      server.hasArg("sb_email") && server.hasArg("sb_password")) {
    
    wifi_ssid = server.arg("ssid"); wifi_ssid.trim();
    wifi_password = server.arg("password"); wifi_password.trim();
    supabase_center_id = server.arg("center_id").toInt();
    supabase_weight_threshold = server.arg("min_weight").toDouble();
    supabase_email = server.arg("sb_email"); supabase_email.trim();
    supabase_password = server.arg("sb_password"); supabase_password.trim();

    saveSettings(wifi_ssid, wifi_password, supabase_center_id, supabase_weight_threshold, supabase_email, supabase_password);

    String successHtml = String(SAVE_HTML);
    successHtml.replace("%SSID%", wifi_ssid);
    server.send(200, "text/html", successHtml);

    delay(1000);
    WiFi.softAPdisconnect(true);

    if (connectToWiFi()) {
      currentState = STATE_CONNECTED_NORMAL;
      checkForUpdates();
    } else {
      startAPMode();
    }
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() { server.send(404, "text/plain", "Not Found"); });
  server.begin();
}

void startAPMode() {
  currentState = STATE_AP_MODE;
  String mac = WiFi.macAddress();
  String uniqueSuffix = mac.substring(mac.length() - 5);
  uniqueSuffix.replace(":", "");
  String apSsid = "Gluvok_WeighTrix_" + uniqueSuffix;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), "");
  setupWebServer();
}

bool connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
  return (WiFi.status() == WL_CONNECTED);
}

void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[WiFi] Reconnecting... Status: %d\n", WiFi.status());
  }
}
