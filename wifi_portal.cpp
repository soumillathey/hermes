#include "wifi_portal.h"
#include "config_manager.h"
#include "html_pages.h"
#include "ota_updater.h"

SystemState currentState = STATE_INIT;
WebServer server(80);

static const char *AP_SSID_PREFIX = "Gluvok_WeighTrix_";
static const char *AP_PASSWORD = "";

void startAPMode() {
  currentState = STATE_AP_MODE;

  String mac = WiFi.macAddress();
  String uniqueSuffix = mac.substring(mac.length() - 5);
  uniqueSuffix.replace(":", "");
  String apSsid = String(AP_SSID_PREFIX) + uniqueSuffix;

  Serial.printf("Configuring Access Point: SSID = '%s'\n", apSsid.c_str());

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), AP_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP Active. Configuration Portal URL: http://");
  Serial.println(myIP);

  setupWebServer();
}

bool connectToWiFi() {
  Serial.printf("Connecting to Network: '%s'\n", wifi_ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connection Established!");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nConnection Timeout. Check credentials or network signal.");
    return false;
  }
}

void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Triggering reconnect sequence...");
    Serial.printf("Current connection status: %d (WL_CONNECTED = 3)\n", WiFi.status());
  }
}

void setupWebServer() {
  static bool serverInitialized = false;
  if (serverInitialized) {
    server.begin();
    return;
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() { server.send(404, "text/plain", "Page Not Found"); });

  server.begin();
  serverInitialized = true;
  Serial.println("HTTP Web Server running.");
}

void handleRoot() {
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
  Serial.println("Config Portal root URL accessed.");
}

void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("center_id") && server.hasArg("min_weight") &&
      server.hasArg("sb_email") && server.hasArg("sb_password")) {
    
    String input_ssid = server.arg("ssid");
    input_ssid.trim();
    String input_pass = server.arg("password");
    input_pass.trim();
    int input_centerId = server.arg("center_id").toInt();
    double input_minWeight = server.arg("min_weight").toDouble();
    
    String input_sbEmail = server.arg("sb_email");
    input_sbEmail.trim();
    String input_sbPass = server.arg("sb_password");
    input_sbPass.trim();

    saveSettings(input_ssid, input_pass, input_centerId, input_minWeight,
                 input_sbEmail, input_sbPass);

    wifi_ssid = input_ssid;
    wifi_password = input_pass;
    supabase_center_id = input_centerId;
    supabase_weight_threshold = input_minWeight;
    supabase_email = input_sbEmail;
    supabase_password = input_sbPass;

    String successHtml = String(SAVE_HTML);
    successHtml.replace("%SSID%", input_ssid);
    server.send(200, "text/html", successHtml);

    Serial.println("Configuration updated. Connecting to Wi-Fi station...");
    delay(1000);
    
    WiFi.softAPdisconnect(true);

    if (connectToWiFi()) {
      currentState = STATE_CONNECTED_NORMAL;
      Serial.println("Gluvok by Lathey Weigh Trix is now in NORMAL operational mode.");
      checkForUpdates();
    } else {
      Serial.println("Could not connect to Wi-Fi. Reverting to AP Config Mode.");
      startAPMode();
    }
  } else {
    server.send(400, "text/plain", "Bad Request: Missing configuration parameters");
    Serial.println("Received invalid form submission.");
  }
}
