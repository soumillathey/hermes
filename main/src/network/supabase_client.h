/**
 * supabase_client.h
 * Shared Supabase connection constants (URL, API key) and auth token extern.
 * Included by all network modules that communicate with Supabase.
 */

#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <Arduino.h>

#define SUPABASE_BASE_URL "https://mjrpqoinwkssmzlimwaz.supabase.co"
#define SUPABASE_ANON_KEY "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1qcnBxb2lud2tzc216bGltd2F6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODIwOTU0NzksImV4cCI6MjA5NzY3MTQ3OX0.C_1b-1dlWvgnRn9xRHhinadsfP5sHjFxQC4rcN6cruw"

extern String auth_token;

#endif // SUPABASE_CLIENT_H
