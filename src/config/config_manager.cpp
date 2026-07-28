#include "config_manager.h"
#include <Preferences.h>

String wifi_ssid = "";
String wifi_password = "";
int supabase_center_id = 1;
double supabase_weight_threshold = 50.0;
String supabase_email = "";
String supabase_password = "";
int supabase_profile_id = -1;

static Preferences preferences;

void loadSettings() {
  preferences.begin("supabase-cfg", true);

  wifi_ssid = preferences.getString("ssid", "");
  wifi_password = preferences.getString("password", "");
  supabase_center_id = preferences.getInt("center_id", 1);
  supabase_weight_threshold = preferences.getDouble("min_weight", 50.0);
  supabase_email = preferences.getString("sb_email", "");
  supabase_password = preferences.getString("sb_pass", "");

  preferences.end();

  Serial.println("Configurations loaded from NVS:");
  Serial.printf(" -> SSID: %s\n", wifi_ssid.c_str());
  Serial.printf(" -> Operator Email: %s\n", supabase_email.c_str());
  Serial.printf(" -> Center ID: %d\n", supabase_center_id);
  Serial.printf(" -> Min Weight Threshold: %.1f\n", supabase_weight_threshold);
}

void saveSettings(String ssid, String pass, int centerId, double minWeight, 
                  String sbEmail, String sbPass) {
  preferences.begin("supabase-cfg", false);

  preferences.putString("ssid", ssid);
  preferences.putString("password", pass);
  preferences.putInt("center_id", centerId);
  preferences.putDouble("min_weight", minWeight);
  preferences.putString("sb_email", sbEmail);
  preferences.putString("sb_pass", sbPass);

  preferences.end();
  Serial.println("New configurations written to Flash Memory.");
}
