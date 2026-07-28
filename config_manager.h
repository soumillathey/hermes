#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// Global Stored Parameters
extern String wifi_ssid;
extern String wifi_password;
extern int supabase_center_id;
extern double supabase_weight_threshold;
extern String supabase_email;
extern String supabase_password;
extern int supabase_profile_id;

// Function Declarations
void loadSettings();
void saveSettings(String ssid, String pass, int centerId, double minWeight, 
                  String sbEmail, String sbPass);

#endif // CONFIG_MANAGER_H
