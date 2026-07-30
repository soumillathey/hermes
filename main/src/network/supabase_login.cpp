/**
 * supabase_login.cpp
 * Supabase JWT authentication — handles login and access token acquisition.
 */

#include "supabase_auth.h"
#include "supabase_client.h"
#include "../config/config_manager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

String auth_token = ""; // Definition (declared extern in supabase_auth.h)

bool loginToSupabase() {
  if (supabase_email.length() == 0 || supabase_password.length() == 0) return false;

  String loginUrl = String(SUPABASE_BASE_URL) + "/auth/v1/token?grant_type=password";
  Serial.println("[Auth] Attempting login to Supabase...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (!http.begin(secureClient, loginUrl)) return false;

  http.setTimeout(15000);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Connection", "close");

  String payload = "{\"email\":\"" + supabase_email + "\",\"password\":\"" + supabase_password + "\"}";
  int code = http.POST(payload);

  if (code == 200 || code == 201) {
    String response = http.getString();
    int index = response.indexOf("\"access_token\":\"");
    if (index != -1) {
      int start = index + 16;
      int end = response.indexOf("\"", start);
      auth_token = response.substring(start, end);
      Serial.println("[Auth] Login successful. Access Token acquired.");
      http.end();
      fetchProfileId();
      return true;
    }
  }

  http.end();
  return false;
}
