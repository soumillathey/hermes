#ifndef SUPABASE_AUTH_H
#define SUPABASE_AUTH_H

#include <Arduino.h>

extern String auth_token;

bool loginToSupabase();
bool fetchProfileId();

#endif // SUPABASE_AUTH_H
