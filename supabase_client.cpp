#include "supabase_client.h"
#include "config_manager.h"
#include <WiFi.h>

// Supabase System Constants
static const String supabase_url = "https://mjrpqoinwkssmzlimwaz.supabase.co";
static const String supabase_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1qcnBxb2lud2tzc216bGltd2F6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODIwOTU0NzksImV4cCI6MjA5NzY3MTQ3OX0.C_1b-1dlWvgnRn9xRHhinadsfP5sHjFxQC4rcN6cruw";
static const String supabase_table = "weighments";

// Foreign Key Constants
static const int supabase_rate_id     = 1;
static const int supabase_customer_id = 1;

String auth_token = "";

bool queryTableForProfileId(String tableName) {
  String url = supabase_url;
  if (!url.endsWith("/")) {
    url += "/";
  }
  url += "rest/v1/" + tableName + "?select=id";

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
        while (start < response.length() && (response[start] == ' ' || response[start] == ':')) {
          start++;
        }
        int end = start;
        while (end < response.length() && isDigit(response[end])) {
          end++;
        }
        String idStr = response.substring(start, end);
        supabase_profile_id = idStr.toInt();
        Serial.printf("[Profile] Resolved Operator Profile ID from table '%s': %d\n", tableName.c_str(), supabase_profile_id);
        http.end();
        return true;
      }
    }
    http.end();
  }
  return false;
}

bool fetchProfileId() {
  if (supabase_url.length() == 0 || supabase_key.length() == 0 || auth_token.length() == 0) {
    return false;
  }

  // Try "profiles" table first
  if (queryTableForProfileId("profiles")) {
    return true;
  }
  
  // Try "operators" table as fallback
  if (queryTableForProfileId("operators")) {
    return true;
  }

  Serial.println("[Profile] Error: Could not resolve Profile ID from either 'profiles' or 'operators' tables.");
  return false;
}

bool loginToSupabase() {
  if (supabase_url.length() == 0 || supabase_key.length() == 0 ||
      supabase_email.length() == 0 || supabase_password.length() == 0) {
    Serial.println("[Auth] Login skipped: Supabase URL, Key, Email, or Password is not configured.");
    return false;
  }

  String loginUrl = supabase_url;
  if (!loginUrl.endsWith("/")) {
    loginUrl += "/";
  }
  loginUrl += "auth/v1/token?grant_type=password";

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
      } else {
        Serial.println("[Auth] Login error: access_token not found in response.");
      }
    } else {
      Serial.printf("[Auth] Login failed. HTTP code: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.printf("[Auth] Response Payload: %s\n", response.c_str());
    }
    http.end();
  } else {
    Serial.println("[Auth] Connection to auth endpoint failed.");
  }
  return false;
}

void postToSupabase(double weightValue) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Supabase] POST skipped: No Wi-Fi network connection.");
    return;
  }

  if (supabase_url.length() == 0 || supabase_table.length() == 0 || supabase_key.length() == 0) {
    Serial.println("[Supabase] POST skipped: Supabase URL, Key, or Table is not configured.");
    return;
  }

  if (auth_token.length() == 0) {
    if (!loginToSupabase()) {
      Serial.println("[Supabase] POST skipped: User authentication failed.");
      return;
    }
  }

  String fullUrl = supabase_url;
  if (!fullUrl.endsWith("/")) {
    fullUrl += "/";
  }
  fullUrl += "rest/v1/" + supabase_table;

  Serial.printf("[Supabase] Starting POST request to URL: %s\n", fullUrl.c_str());

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  uint32_t randomNum = 1000 + (esp_random() % 9000);
  String vehicleNum = "MH12AB" + String(randomNum);

  String jsonPayload = "{";
  jsonPayload += "\"weight\":" + String(weightValue, 3) + ",";
  jsonPayload += "\"vehicle_number\":\"" + vehicleNum + "\",";
  jsonPayload += "\"rate_id\":" + String(supabase_rate_id) + ",";
  jsonPayload += "\"center_id\":" + String(supabase_center_id) + ",";
  if (supabase_profile_id != -1) {
    jsonPayload += "\"profile_id\":" + String(supabase_profile_id) + ",";
  }
  jsonPayload += "\"customer_id\":" + String(supabase_customer_id);
  jsonPayload += "}";

  Serial.printf("[Supabase] Payload: %s\n", jsonPayload.c_str());

  bool requestCompleted = false;
  int retryAttempts = 0;

  while (!requestCompleted && retryAttempts < 2) {
    HTTPClient http;
    if (http.begin(secureClient, fullUrl)) {
      http.setTimeout(15000);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("apikey", supabase_key);

      String authHeader = "Bearer " + auth_token;
      http.addHeader("Authorization", authHeader);
      http.addHeader("Prefer", "return=minimal"); 
      http.addHeader("Connection", "close"); 

      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
        if (httpResponseCode == 201 || httpResponseCode == 200) {
          Serial.printf("[Supabase] POST success. Response Code: %d\n", httpResponseCode);
          requestCompleted = true;
        } else if (httpResponseCode == 401 || httpResponseCode == 403) {
          Serial.printf("[Supabase] Auth failed (Code %d). Retrying login...\n", httpResponseCode);
          auth_token = "";
          retryAttempts++;
          if (retryAttempts < 2) {
            if (!loginToSupabase()) {
              Serial.println("[Supabase] POST failed: Token refresh failed.");
              requestCompleted = true;
            }
          }
        } else {
          Serial.printf("[Supabase] POST failed. Response Code: %d\n", httpResponseCode);
          String response = http.getString();
          Serial.printf("[Supabase] Response Payload: %s\n", response.c_str());
          requestCompleted = true;
        }
      } else {
        Serial.printf("[Supabase] POST failed. Error Code: %d (%s)\n", 
                      httpResponseCode, http.errorToString(httpResponseCode).c_str());
        requestCompleted = true;
      }
      http.end();
    } else {
      Serial.println("[Supabase] Network connection to host failed.");
      requestCompleted = true;
    }
  }
}
