#include "supabase_auth.h"
#include "../config/config_manager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

static const String supabase_url = "https://mjrpqoinwkssmzlimwaz.supabase.co";
static const String supabase_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1qcnBxb2lud2tzc216bGltd2F6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODIwOTU0NzksImV4cCI6MjA5NzY3MTQ3OX0.C_1b-1dlWvgnRn9xRHhinadsfP5sHjFxQC4rcN6cruw";

String auth_token = "";

static bool queryTableForProfileId(String tableName) {
  String url = supabase_url + "/rest/v1/" + tableName + "?select=id";

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (http.begin(secureClient, url)) {
    http.setTimeout(10000);
    http.addHeader("apikey", supabase_key);
    http.addHeader("Authorization", "Bearer " + auth_token);
    http.addHeader("Connection", "close");

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String response = http.getString();
      int idIdx = response.indexOf("\"id\":");
      if (idIdx != -1) {
        int start = idIdx + 5;
        while (start < response.length() && (response[start] == ' ' || response[start] == ':')) start++;
        int end = start;
        while (end < response.length() && isDigit(response[end])) end++;
        
        supabase_profile_id = response.substring(start, end).toInt();
        Serial.printf("[Profile] Resolved Profile ID from '%s': %d\n", tableName.c_str(), supabase_profile_id);
        http.end();
        return true;
      }
    }
    http.end();
  }
  return false;
}

bool fetchProfileId() {
  if (auth_token.length() == 0) return false;
  return queryTableForProfileId("profiles") || queryTableForProfileId("operators");
}

bool loginToSupabase() {
  if (supabase_email.length() == 0 || supabase_password.length() == 0) return false;

  String loginUrl = supabase_url + "/auth/v1/token?grant_type=password";
  Serial.println("[Auth] Attempting login to Supabase...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (http.begin(secureClient, loginUrl)) {
    http.setTimeout(15000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabase_key);
    http.addHeader("Connection", "close");

    String authPayload = "{\"email\":\"" + supabase_email + "\",\"password\":\"" + supabase_password + "\"}";
    int httpResponseCode = http.POST(authPayload);

    if (httpResponseCode == 200 || httpResponseCode == 201) {
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
  }
  return false;
}
