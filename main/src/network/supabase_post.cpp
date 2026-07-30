#include "supabase_post.h"
#include "supabase_auth.h"
#include "../config/config_manager.h"
#include "../led/rgb_led.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static const String supabase_url = "https://mjrpqoinwkssmzlimwaz.supabase.co";
static const String supabase_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1qcnBxb2lud2tzc216bGltd2F6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODIwOTU0NzksImV4cCI6MjA5NzY3MTQ3OX0.C_1b-1dlWvgnRn9xRHhinadsfP5sHjFxQC4rcN6cruw";
static const String supabase_table = "weighments";

void postToSupabase(double weightValue) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Supabase] POST skipped: No Wi-Fi network connection.");
    return;
  }
  if (auth_token.length() == 0 && !loginToSupabase()) return;

  triggerRGBGreen(5000); // Turn RGB LED GREEN for 5 seconds when sending data to Supabase
  String fullUrl = supabase_url + "/rest/v1/" + supabase_table;
  Serial.printf("[Supabase] Starting POST request to URL: %s\n", fullUrl.c_str());

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  uint32_t randomNum = 1000 + (esp_random() % 9000);
  String jsonPayload = "{\"weight\":" + String(weightValue, 3) +
                       ",\"vehicle_number\":\"MH12AB" + String(randomNum) + "\"" +
                       ",\"rate_id\":1,\"center_id\":" + String(supabase_center_id);
  if (supabase_profile_id != -1) {
    jsonPayload += ",\"profile_id\":" + String(supabase_profile_id);
  }
  jsonPayload += ",\"customer_id\":1}";

  Serial.printf("[Supabase] Payload: %s\n", jsonPayload.c_str());

  HTTPClient http;
  if (http.begin(secureClient, fullUrl)) {
    http.setTimeout(15000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabase_key);
    http.addHeader("Authorization", "Bearer " + auth_token);
    http.addHeader("Prefer", "return=minimal");
    http.addHeader("Connection", "close");

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode == 200 || httpResponseCode == 201) {
      Serial.printf("[Supabase] POST success. Response Code: %d.\n", httpResponseCode);
      Serial.println("[RGB] Triggering GREEN LED for 5 seconds after 201 Success!");
      triggerRGBGreen(5000); // Turn RGB LED GREEN for 5 seconds on successful POST
    } else if (httpResponseCode == 401 || httpResponseCode == 403) {
      auth_token = ""; // Refresh token on retry
      if (loginToSupabase()) postToSupabase(weightValue);
    } else {
      Serial.printf("[Supabase] POST error: %d\n", httpResponseCode);
    }
    http.end();
  }
}
