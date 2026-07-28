#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

enum SystemState { STATE_INIT, STATE_AP_MODE, STATE_CONNECTED_NORMAL };

extern SystemState currentState;
extern WebServer server;

const unsigned long WIFI_CHECK_INTERVAL = 20000; // Wi-Fi check interval (20s)

void startAPMode();
bool connectToWiFi();
void autoReconnectWiFi();
void setupWebServer();
void handleRoot();
void handleSave();

#endif // WIFI_PORTAL_H
