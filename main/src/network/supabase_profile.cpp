/**
 * supabase_profile.cpp
 * Resolves the operator's profile_id by querying Supabase REST tables.
 */

#include "supabase_auth.h"
#include "supabase_client.h"
#include "../config/config_manager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

static bool queryTableForProfileId(const String& tableName) {
  String url = String(SUPABASE_BASE_URL) + "/rest/v1/" + tableName + "?select=id";

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (!http.begin(secureClient, url)) return false;

  http.setTimeout(10000);
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", "Bearer " + auth_token);
  http.addHeader("Connection", "close");

  int code = http.GET();
  if (code == 200) {
    String response = http.getString();
    int idIdx = response.indexOf("\"id\":");
    if (idIdx != -1) {
      int start = idIdx + 5;
      while (start < response.length() && !isDigit(response[start])) start++;
      int end = start;
      while (end < response.length() && isDigit(response[end])) end++;
      supabase_profile_id = response.substring(start, end).toInt();
      Serial.printf("[Profile] Resolved Operator Profile ID from table '%s': %d\n",
                    tableName.c_str(), supabase_profile_id);
      http.end();
      return true;
    }
  }
  http.end();
  return false;
}

bool fetchProfileId() {
  if (auth_token.length() == 0) return false;
  return queryTableForProfileId("profiles") || queryTableForProfileId("operators");
}
