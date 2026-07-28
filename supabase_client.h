#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

extern String auth_token;

bool loginToSupabase();
bool fetchProfileId();
void postToSupabase(double weightValue);

#endif // SUPABASE_CLIENT_H
