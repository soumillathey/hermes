#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WebServer.h>

enum SystemState { STATE_INIT, STATE_AP_MODE, STATE_CONNECTED_NORMAL };

extern SystemState currentState;
extern WebServer server;
extern const unsigned long WIFI_CHECK_INTERVAL;

void startAPMode();
bool connectToWiFi();
void autoReconnectWiFi();
void setupWebServer();

#endif // WIFI_PORTAL_H
