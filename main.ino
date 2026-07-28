/**
 * ESP32 Wi-Fi Configuration Portal & Cloud Firestore Data Logger
 *
 * Features:
 *  - ESP32 Access Point (AP) mode with a styled configuration portal.
 *  - Premium Glassmorphic Web UI to input credentials and Cloud Firestore
 * parameters.
 *  - Permanent settings storage using ESP32 Preferences (NVS Flash).
 *  - Non-blocking loop structure for Wi-Fi reconnection and client server
 * logic.
 *  - Automated random mock data generation every 10 seconds.
 *  - Direct HTTPS REST POST requests to Google Cloud Firestore database.
 */

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// --- Configuration Constants ---
const char *AP_SSID_PREFIX =
    "Gluvok_WeighTrix_";      // Suffix will be generated from MAC address
const char *AP_PASSWORD = ""; // Open network for configuration simplicity
const int WEB_SERVER_PORT = 80;
const unsigned long DATA_POST_INTERVAL = 10000;  // 10 seconds in milliseconds
const unsigned long WIFI_CHECK_INTERVAL = 20000; // 20 seconds in milliseconds

// --- Global Objects ---
WebServer server(WEB_SERVER_PORT);
Preferences preferences;
HardwareSerial Indicator(2);

// --- Weight Indicator State & Stability Settings ---
double supabase_weight_threshold = 50.0;     // Configurable minimum weight to trigger a weighment (default: 50.0)
const double STABILITY_TOLERANCE = 2.0;      // Allowed fluctuation range
const unsigned long STABILITY_DURATION = 10000; // Required time to stabilize (ms)

enum ScaleState {
  SCALE_IDLE,
  SCALE_STABILIZING,
  SCALE_STABLE_RECORDED
};
ScaleState scaleState = SCALE_IDLE;

unsigned long stableStartTime = 0;
double currentStableWeightCandidate = 0.0;
String digitBuffer = "";
bool isNegative = false;

// --- System State Enum ---
enum SystemState { STATE_INIT, STATE_AP_MODE, STATE_CONNECTED_NORMAL };
SystemState currentState = STATE_INIT;

// --- Stored Parameters ---
String wifi_ssid = "";
String wifi_password = "";
int supabase_center_id = 1;  // Default center ID
String supabase_email = "";
String supabase_password = "";
int supabase_profile_id = -1; // Dynamically resolved operator ID

// --- Supabase System Constants ---
const String supabase_url = "https://mjrpqoinwkssmzlimwaz.supabase.co";
const String supabase_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1qcnBxb2lud2tzc216bGltd2F6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODIwOTU0NzksImV4cCI6MjA5NzY3MTQ3OX0.C_1b-1dlWvgnRn9xRHhinadsfP5sHjFxQC4rcN6cruw";
const String supabase_table = "weighments";

// --- Stored Token Cache ---
String auth_token = "";

// --- Other Supabase Foreign Key IDs (Must exist in database) ---
const int supabase_rate_id     = 1;
const int supabase_customer_id = 1;

// --- Timer Tracking Variables ---
unsigned long lastDataPostTime = 0;
unsigned long lastWifiCheckTime = 0;

// --- OTA Firmware Configuration ---
const String CURRENT_VERSION = "1.0.2";
const String OTA_VERSION_URL = "https://raw.githubusercontent.com/SoumilLathey/gluvok-hardware-ota/main/firmware/version.json";
const unsigned long OTA_CHECK_INTERVAL = 3600000; // Check every 1 hour (in ms)
unsigned long lastOtaCheckTime = 0;

// --- HTML Templates ---
// Beautiful Glassmorphic Configuration Page (Dark Theme)
const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gluvok by Lathey Weigh Trix Portal</title>
    <style>
        :root {
            --bg-color: #030712;
            --card-bg: rgba(17, 24, 39, 0.7);
            --text-color: #9ca3af;
            --text-title: #ffffff;
            --accent-primary: #3b82f6;
            --accent-secondary: #60a5fa;
            --accent-glow: rgba(59, 130, 246, 0.4);
            --border-color: rgba(255, 255, 255, 0.05);
            --input-bg: rgba(31, 41, 55, 0.85);
            --success-color: #10b981;
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }
        body {
            background: linear-gradient(135deg, #0b1528 0%, #030712 100%);
            color: var(--text-color);
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            width: 100%;
            max-width: 460px;
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 24px;
            padding: 35px 30px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.6), inset 0 1px 0 rgba(255, 255, 255, 0.05);
            animation: slideUp 0.6s cubic-bezier(0.16, 1, 0.3, 1);
        }
        @keyframes slideUp {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .header {
            text-align: center;
            margin-bottom: 28px;
        }
        .status-badge {
            display: inline-flex;
            align-items: center;
            background: rgba(59, 130, 246, 0.12);
            color: #93c5fd;
            padding: 5px 12px;
            border-radius: 20px;
            font-size: 11px;
            font-weight: 600;
            margin-bottom: 16px;
            border: 1px solid rgba(59, 130, 246, 0.25);
            text-transform: uppercase;
            letter-spacing: 0.8px;
        }
        .status-dot {
            width: 7px;
            height: 7px;
            background-color: #3b82f6;
            border-radius: 50%;
            margin-right: 8px;
            animation: pulse 1.8s infinite;
        }
        @keyframes pulse {
            0% { transform: scale(0.95); opacity: 0.5; box-shadow: 0 0 0 0 rgba(59, 130, 246, 0.7); }
            70% { transform: scale(1.1); opacity: 1; box-shadow: 0 0 0 5px rgba(59, 130, 246, 0); }
            100% { transform: scale(0.95); opacity: 0.5; box-shadow: 0 0 0 0 rgba(59, 130, 246, 0); }
        }
        .logo-container {
            display: flex;
            justify-content: center;
            align-items: center;
            margin-bottom: 18px;
        }
        .logo-svg {
            max-width: 280px;
            height: auto;
        }
        .header p {
            font-size: 13.5px;
            color: #7b7e85;
            line-height: 1.4;
        }
        .section-title {
            font-size: 12px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 1px;
            color: var(--accent-primary);
            margin: 24px 0 12px 0;
            border-bottom: 1px solid rgba(255, 107, 107, 0.2);
            padding-bottom: 6px;
        }
        .form-group {
            margin-bottom: 18px;
        }
        .form-group label {
            display: block;
            margin-bottom: 7px;
            font-size: 11px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            color: #8c8f96;
        }
        .form-group input {
            width: 100%;
            padding: 11px 14px;
            background: var(--input-bg);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 10px;
            color: var(--text-title);
            font-size: 14.5px;
            transition: all 0.25s ease;
            outline: none;
        }
        .form-group input:focus {
            border-color: var(--accent-primary);
            box-shadow: 0 0 10px var(--accent-glow);
            background: rgba(10, 10, 15, 0.95);
        }
        .btn-submit {
            width: 100%;
            padding: 13px;
            background: linear-gradient(135deg, var(--accent-primary) 0%, var(--accent-secondary) 100%);
            border: none;
            border-radius: 10px;
            color: #ffffff;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.25s ease;
            box-shadow: 0 4px 15px rgba(255, 107, 107, 0.3);
            letter-spacing: 0.5px;
            margin-top: 8px;
        }
        .btn-submit:hover {
            transform: translateY(-1.5px);
            box-shadow: 0 6px 20px rgba(255, 107, 107, 0.45);
            filter: brightness(1.1);
        }
        .btn-submit:active {
            transform: translateY(0);
        }
        .footer {
            margin-top: 25px;
            text-align: center;
            font-size: 11px;
            color: #5b5c61;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="status-badge">
                <span class="status-dot"></span>
                <span>Config Mode Active</span>
            </div>
            <div class="logo-container">
                <svg class="logo-svg" viewBox="0 0 300 70" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <!-- Hexagon Left/Bottom G (Vibrant Blue #3b82f6) -->
                    <path d="M 28 10 L 8 21.5 L 8 49 L 28 60.5 L 48 49 L 48 41 L 38 41 L 38 45 L 28 51 L 18 45 L 18 25.5 L 28 20 L 38 25.5 L 38 31 L 28 31 L 28 39 L 48 39 L 48 21.5 Z" fill="#3b82f6" />
                    <!-- Hexagon Top Right L (Light Blue #93c5fd) -->
                    <path d="M 33 6 L 48 14.5 L 48 31 L 41 27 L 41 18.5 L 33 14 Z" fill="#93c5fd" />
                    <!-- Vertical Divider -->
                    <line x1="64" y1="8" x2="64" y2="62" stroke="rgba(255, 255, 255, 0.15)" stroke-width="2" />
                    <!-- Text GLUVOK -->
                    <text x="76" y="38" font-family="-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif" font-weight="800" font-size="28" fill="#ffffff" letter-spacing="3">GLUVOK</text>
                    <!-- Text BY LATHEY WEIGH TRIX -->
                    <text x="76" y="56" font-family="-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif" font-weight="600" font-size="9" fill="#93c5fd" letter-spacing="1.5">BY LATHEY WEIGH TRIX</text>
                </svg>
            </div>
            <p>Setup local Wi-Fi and Operator details</p>
        </div>
        <form action="/save" method="POST">
            <div class="section-title">Wi-Fi Settings</div>
            <div class="form-group">
                <label for="ssid">Wi-Fi Network SSID</label>
                <input type="text" id="ssid" name="ssid" placeholder="Wi-Fi SSID" required value="%SSID%">
            </div>
            <div class="form-group">
                <label for="password">Wi-Fi Password</label>
                <input type="password" id="password" name="password" placeholder="••••••••" value="%PASSWORD%">
            </div>

            <div class="section-title">Operator Login Credentials</div>
            <div class="form-group">
                <label for="sb_email">Operator Email</label>
                <input type="email" id="sb_email" name="sb_email" placeholder="operator@example.com" required value="%SB_EMAIL%">
            </div>
            <div class="form-group">
                <label for="sb_password">Operator Password</label>
                <input type="password" id="sb_password" name="sb_password" placeholder="••••••••" required value="%SB_PASSWORD%">
            </div>

            <div class="section-title">Scale Parameters</div>
            <div class="form-group">
                <label for="center_id">Center ID</label>
                <input type="number" id="center_id" name="center_id" placeholder="e.g. 1" required value="%CENTER_ID%">
            </div>
            <div class="form-group">
                <label for="min_weight">Min Weight Threshold (kg)</label>
                <input type="number" step="0.1" id="min_weight" name="min_weight" placeholder="e.g. 50.0" required value="%MIN_WEIGHT%">
            </div>
            <button type="submit" class="btn-submit">Apply Configuration</button>
        </form>
        <div class="footer">
            Device MAC: <span style="font-family: monospace; color: #ff8e8e;">%MAC%</span>
        </div>
    </div>
</body>
</html>
)rawliteral";

// Beautiful Saving/Rebooting Page
const char SAVE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Applying Configurations</title>
    <style>
        :root {
            --bg-color: #0b0c10;
            --card-bg: rgba(30, 30, 38, 0.7);
            --text-color: #c5c6c7;
            --text-title: #ffffff;
            --accent-primary: #00e676;
            --border-color: rgba(255, 255, 255, 0.08);
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
        }
        body {
            background: linear-gradient(135deg, #091a10 0%, #06060c 100%);
            color: var(--text-color);
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            width: 100%;
            max-width: 440px;
            backdrop-filter: blur(16px);
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 24px;
            padding: 40px 30px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.6);
            text-align: center;
        }
        .badge {
            background: rgba(0, 230, 118, 0.12);
            color: var(--accent-primary);
            padding: 6px 14px;
            border-radius: 20px;
            font-size: 11px;
            font-weight: 600;
            display: inline-block;
            margin-bottom: 20px;
            border: 1px solid rgba(0, 230, 118, 0.25);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        h1 {
            color: var(--text-title);
            font-size: 24px;
            font-weight: 700;
            margin-bottom: 12px;
            background: linear-gradient(135deg, #ffffff 40%, #a3ffcc 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        p {
            font-size: 14px;
            line-height: 1.5;
            color: #8a8f98;
            margin-bottom: 24px;
        }
        .spinner {
            width: 44px;
            height: 44px;
            border: 3px solid rgba(0, 230, 118, 0.1);
            border-top: 3px solid var(--accent-primary);
            border-radius: 50%;
            margin: 0 auto;
            animation: spin 0.8s linear infinite;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="badge">Success</div>
        <h1>Settings Saved</h1>
        <p>The configuration has been written to flash. Gluvok by Lathey Weigh Trix will now connect to <b>%SSID%</b>.</p>
        <div class="spinner"></div>
    </div>
</body>
</html>
)rawliteral";

// --- Forward Declarations ---
void loadSettings();
bool fetchProfileId();
bool queryTableForProfileId(String tableName);
void saveSettings(String ssid, String pass, int centerId, double minWeight, 
                  String sbEmail, String sbPass);
void startAPMode();
bool connectToWiFi();
void setupWebServer();
void handleRoot();
void handleSave();
bool loginToSupabase();
void postToSupabase(double weightValue);
void autoReconnectWiFi();
void processNewWeight(double parsedWeight);
void checkForUpdates();

// --- Main Setup Function ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==============================================");
  Serial.println("Gluvok by Lathey Weigh Trix Starting...");
  Serial.printf("Device MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.println("==============================================");

  // Initialize random seed
  randomSeed(analogRead(0));

  // Initialize UART2 for Weight Indicator
  Indicator.begin(1200, SERIAL_8N1, 16, 17);
  Serial.println("Indicator listening on UART2 (pins 16/RX, 17/TX) at 1200 baud...");

  // Load configured settings from non-volatile flash storage
  loadSettings();

  // Always force AP Config Mode on boot so operator configuration is required on every startup
  Serial.println("Forcing AP Config Mode on startup.");
  startAPMode();

  lastDataPostTime = millis();
  lastWifiCheckTime = millis();
}

// --- Main Loop Function ---
void loop() {
  // Always handle incoming configuration portal clients
  server.handleClient();

  // Read weight indicator stream
  while (Indicator.available()) {
    char c = (char)Indicator.read();
    
    if (c == '-') {
      isNegative = true;
      digitBuffer = "";
    } else if (isDigit(c)) {
      digitBuffer += c;
    } else {
      if (digitBuffer.length() >= 5 && digitBuffer.length() <= 6) {
        double parsedVal = digitBuffer.toDouble();
        if (isNegative) {
          parsedVal = -parsedVal;
        }
        processNewWeight(parsedVal);
      }
      digitBuffer = "";
      isNegative = false;
    }
  }

  // If in normal operational mode
  if (currentState == STATE_CONNECTED_NORMAL) {
    unsigned long currentMillis = millis();

    // Check Wi-Fi state & handle auto reconnection at regular intervals
    if (currentMillis - lastWifiCheckTime >= WIFI_CHECK_INTERVAL) {
      lastWifiCheckTime = currentMillis;
      autoReconnectWiFi();
    }

    // Periodically check for remote OTA firmware updates
    if (currentMillis - lastOtaCheckTime >= OTA_CHECK_INTERVAL) {
      lastOtaCheckTime = currentMillis;
      checkForUpdates();
    }
  }

  // Avoid starving ESP32 background tasks
  delay(10);
}

// --- Load Settings from Preferences ---
void loadSettings() {
  preferences.begin("supabase-cfg", true); // Open namespace in read-only mode

  wifi_ssid = preferences.getString("ssid", "");
  wifi_password = preferences.getString("password", "");
  supabase_center_id = preferences.getInt("center_id", 1);
  supabase_weight_threshold = preferences.getDouble("min_weight", 50.0);
  
  supabase_email = preferences.getString("sb_email", "");
  supabase_password = preferences.getString("sb_pass", "");

  preferences.end();

  Serial.println("Configurations loaded from NVS:");
  Serial.printf(" -> SSID: %s\n", wifi_ssid.c_str());
  Serial.printf(" -> Supabase URL: %s\n", supabase_url.c_str());
  Serial.printf(" -> Supabase Table: %s\n", supabase_table.c_str());
  Serial.printf(" -> Operator Email: %s\n", supabase_email.c_str());
  Serial.printf(" -> Center ID: %d\n", supabase_center_id);
  Serial.printf(" -> Min Weight Threshold: %.1f\n", supabase_weight_threshold);
}

// --- Save Settings to Preferences ---
void saveSettings(String ssid, String pass, int centerId, double minWeight, 
                  String sbEmail, String sbPass) {
  preferences.begin("supabase-cfg", false); // Open namespace in read/write mode

  preferences.putString("ssid", ssid);
  preferences.putString("password", pass);
  preferences.putInt("center_id", centerId);
  preferences.putDouble("min_weight", minWeight);
  
  preferences.putString("sb_email", sbEmail);
  preferences.putString("sb_pass", sbPass);

  preferences.end();
  Serial.println("New configurations written to Flash Memory.");
}

// --- Start ESP32 Access Point ---
void startAPMode() {
  currentState = STATE_AP_MODE;

  // Generate unique SSID suffix using MAC address to avoid conflicts
  String mac = WiFi.macAddress();
  String uniqueSuffix = mac.substring(mac.length() - 5);
  uniqueSuffix.replace(":", "");
  String apSsid = String(AP_SSID_PREFIX) + uniqueSuffix;

  Serial.printf("Configuring Access Point: SSID = '%s'\n", apSsid.c_str());

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), AP_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP Active. Configuration Portal URL: http://");
  Serial.println(myIP);

  // Setup routes and start Server
  setupWebServer();
}

// --- Connect to Wi-Fi Station ---
bool connectToWiFi() {
  Serial.printf("Connecting to Network: '%s'\n", wifi_ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true); // Tell ESP32 driver to auto-reconnect if dropped
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  // Wait for Wi-Fi connection with a timeout (15 seconds)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connection Established!");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println(
        "\nConnection Timeout. Check credentials or network signal.");
    return false;
  }
}

// --- Auto Reconnect Logic ---
void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Triggering reconnect sequence...");
    // Let driver attempt background reconnection
    // If it stays disconnected, print status
    Serial.printf("Current connection status: %d (WL_CONNECTED = 3)\n",
                  WiFi.status());
  }
}

// --- Set Web Server Routes ---
void setupWebServer() {
  static bool serverInitialized = false;
  if (serverInitialized) {
    server.begin();
    return;
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() { server.send(404, "text/plain", "Page Not Found"); });

  server.begin();
  serverInitialized = true;
  Serial.println("HTTP Web Server running.");
}

// --- Serves Configuration Web Portal ---
void handleRoot() {
  String html = String(CONFIG_HTML);

  // Bind stored configurations to input values
  html.replace("%SSID%", wifi_ssid);
  html.replace("%PASSWORD%", wifi_password);
  
  html.replace("%SB_EMAIL%", supabase_email);
  html.replace("%SB_PASSWORD%", supabase_password);
  
  html.replace("%CENTER_ID%", String(supabase_center_id));
  html.replace("%MIN_WEIGHT%", String(supabase_weight_threshold, 1));
  html.replace("%MAC%", WiFi.macAddress());

  server.send(200, "text/html", html);
  Serial.println("Config Portal root URL accessed.");
}

// --- Processes Form Submission ---
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("center_id") && server.hasArg("min_weight") &&
      server.hasArg("sb_email") && server.hasArg("sb_password")) {
    
    String input_ssid = server.arg("ssid");
    String input_pass = server.arg("password");
    int input_centerId = server.arg("center_id").toInt();
    double input_minWeight = server.arg("min_weight").toDouble();
    
    String input_sbEmail = server.arg("sb_email");
    String input_sbPass = server.arg("sb_password");

    // Write parameters to Flash Memory
    saveSettings(input_ssid, input_pass, input_centerId, input_minWeight,
                 input_sbEmail, input_sbPass);

    // Update global variables so we connect using new parameters
    wifi_ssid = input_ssid;
    wifi_password = input_pass;
    supabase_center_id = input_centerId;
    supabase_weight_threshold = input_minWeight;
    supabase_email = input_sbEmail;
    supabase_password = input_sbPass;

    // Send visual Success Page back to user
    String successHtml = String(SAVE_HTML);
    successHtml.replace("%SSID%", input_ssid);
    server.send(200, "text/html", successHtml);

    Serial.println("Configuration updated. Connecting to Wi-Fi station...");
    delay(1000); // Give the client browser time to receive the success page HTML
    
    // Disable AP and try to connect as Station
    WiFi.softAPdisconnect(true);

    if (connectToWiFi()) {
      currentState = STATE_CONNECTED_NORMAL;
      Serial.println("Gluvok by Lathey Weigh Trix is now in NORMAL operational mode.");
      
      // Perform initial OTA firmware check
      checkForUpdates();
    } else {
      Serial.println("Could not connect to Wi-Fi. Reverting to AP Config Mode.");
      startAPMode();
    }
  } else {
    server.send(400, "text/plain", "Bad Request: Missing configuration parameters");
    Serial.println("Received invalid form submission.");
  }
}

// --- Authenticate User via Supabase Auth API ---
bool loginToSupabase() {
  if (supabase_url.length() == 0 || supabase_key.length() == 0 ||
      supabase_email.length() == 0 || supabase_password.length() == 0) {
    Serial.println("[Auth] Login skipped: Supabase URL, Key, Email, or Password is not configured.");
    return false;
  }

  // Auth endpoint for email/password authentication
  String loginUrl = supabase_url;
  if (!loginUrl.endsWith("/")) {
    loginUrl += "/";
  }
  loginUrl += "auth/v1/token?grant_type=password";

  Serial.println("[Auth] Attempting login to Supabase...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure(); // Bypass certificate validation

  HTTPClient http;
  if (http.begin(secureClient, loginUrl)) {
    http.setTimeout(15000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabase_key); // Required for routing in Supabase Gateway
    http.addHeader("Connection", "close");

    // Construct body
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
        
        // Dynamically fetch the Operator Profile ID
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

// --- Helper to dynamically resolve profile_id from public tables ---
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

// --- Supabase REST POST ---
void postToSupabase(double weightValue) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Supabase] POST skipped: No Wi-Fi network connection.");
    return;
  }

  if (supabase_url.length() == 0 || supabase_table.length() == 0 ||
      supabase_key.length() == 0) {
    Serial.println("[Supabase] POST skipped: Supabase URL, Key, or Table is "
                   "not configured.");
    return;
  }

  // 1. Authenticate if token is missing
  if (auth_token.length() == 0) {
    if (!loginToSupabase()) {
      Serial.println("[Supabase] POST skipped: User authentication failed.");
      return;
    }
  }

  // Build target Supabase REST Endpoint URL
  String fullUrl = supabase_url;
  if (!fullUrl.endsWith("/")) {
    fullUrl += "/";
  }
  fullUrl += "rest/v1/" + supabase_table;

  Serial.printf("[Supabase] Starting POST request to URL: %s\n",
                fullUrl.c_str());

  WiFiClientSecure secureClient;
  secureClient.setInsecure(); // Bypass root Certificate validation

  // Generate mock vehicle number (max 10 characters due to varchar(10) database limit)
  String vehicleNum = "MH12AB" + String(random(1000, 9999));

  // Build Supabase JSON Payload
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
      http.setTimeout(15000); // 15 seconds timeout to prevent read timeouts on SSL handshake
      http.addHeader("Content-Type", "application/json");
      http.addHeader("apikey", supabase_key); // Required for Supabase routing

      String authHeader = "Bearer " + auth_token; // Use dynamic user JWT access_token
      http.addHeader("Authorization", authHeader);
      http.addHeader("Prefer", "return=minimal"); 
      http.addHeader("Connection", "close"); 

      // Make the POST call
      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
        if (httpResponseCode == 201 || httpResponseCode == 200) {
          Serial.printf("[Supabase] POST success. Response Code: %d\n", httpResponseCode);
          requestCompleted = true;
        } else if (httpResponseCode == 401 || httpResponseCode == 403) {
          // Token might be expired, clear cache and request a new one
          Serial.printf("[Supabase] Auth failed (Code %d). Retrying login...\n", httpResponseCode);
          auth_token = ""; // Clear token
          retryAttempts++;
          if (retryAttempts < 2) {
            if (!loginToSupabase()) {
              Serial.println("[Supabase] POST failed: Token refresh failed.");
              requestCompleted = true; // Break loop
            }
          }
        } else {
          Serial.printf("[Supabase] POST failed. Response Code: %d\n", httpResponseCode);
          String response = http.getString();
          Serial.printf("[Supabase] Response Payload: %s\n", response.c_str());
          requestCompleted = true; // Non-auth error, break loop
        }
      } else {
        Serial.printf("[Supabase] POST failed. Error Code: %d (%s)\n", 
                      httpResponseCode, http.errorToString(httpResponseCode).c_str());
        requestCompleted = true; // Connection error, break loop
      }
      http.end();
    } else {
      Serial.println("[Supabase] Network connection to host failed.");
      requestCompleted = true;
    }
  }
}

// --- Process New Weight & Track Scale Stability ---
void processNewWeight(double parsedWeight) {
  static double lastPrintedWeight = -9999.0;
  if (abs(parsedWeight - lastPrintedWeight) >= 1.0) {
    Serial.printf("[Scale] Parsed weight: %.3f (Threshold: %.1f)\n", parsedWeight, supabase_weight_threshold);
    lastPrintedWeight = parsedWeight;
  }

  // The session only closes when weight returns to zero (<= 0.0)
  if (parsedWeight <= 0.0) {
    if (scaleState != SCALE_IDLE) {
      scaleState = SCALE_IDLE;
      stableStartTime = 0;
      currentStableWeightCandidate = 0.0;
    }
    return;
  }

  // Weight is positive (> 0.0)
  if (scaleState == SCALE_IDLE) {
    // The session must begin when the threshold is crossed (>= supabase_weight_threshold)
    if (parsedWeight >= supabase_weight_threshold) {
      scaleState = SCALE_STABILIZING;
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    } else {
      // Ignore weights below the threshold when not in an active session
    }
  } 
  else if (scaleState == SCALE_STABILIZING) {
    // Session is active. Check stability of the weight.
    if (abs(parsedWeight - currentStableWeightCandidate) <= STABILITY_TOLERANCE) {
      unsigned long elapsed = millis() - stableStartTime;
      if (elapsed >= STABILITY_DURATION) {
        if (parsedWeight >= supabase_weight_threshold) {
          Serial.printf("[Scale] Weight stable reached: %.3f. Uploading to Supabase...\n", parsedWeight);
          scaleState = SCALE_STABLE_RECORDED;
          postToSupabase(parsedWeight);
        } else {
          // Stabilized below threshold, reset timer to prevent immediate upload if it rises again
          stableStartTime = millis();
          currentStableWeightCandidate = parsedWeight;
        }
      }
    } else {
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    }
  }
  else if (scaleState == SCALE_STABLE_RECORDED) {
    // Already recorded. Session remains active until weight returns to zero (<= 0.0), handled above.
  }
}

// --- HTTP OTA Update Client ---
void checkForUpdates() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] Skipped: No Wi-Fi network connection.");
    return;
  }

  Serial.println("[OTA] Checking for updates...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure(); // Bypass Certificate validation for raw GitHub/HTTPS URLs

  HTTPClient http;
  if (http.begin(secureClient, OTA_VERSION_URL)) {
    http.setTimeout(10000);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("[OTA] Version info payload: " + payload);

      // Parse JSON manually to avoid additional library dependency
      int versionIdx = payload.indexOf("\"version\"");
      int urlIdx = payload.indexOf("\"url\"");

      if (versionIdx != -1 && urlIdx != -1) {
        // Extract remote version string
        int verStart = payload.indexOf(":", versionIdx) + 1;
        verStart = payload.indexOf("\"", verStart) + 1;
        int verEnd = payload.indexOf("\"", verStart);
        String remoteVersion = payload.substring(verStart, verEnd);
        remoteVersion.trim();

        // Extract binary URL
        int urlStart = payload.indexOf(":", urlIdx) + 1;
        urlStart = payload.indexOf("\"", urlStart) + 1;
        int urlEnd = payload.indexOf("\"", urlStart);
        String binUrl = payload.substring(urlStart, urlEnd);
        binUrl.trim();

        Serial.printf("[OTA] Current Version: %s, Remote Version: %s\n", CURRENT_VERSION.c_str(), remoteVersion.c_str());

        if (remoteVersion != CURRENT_VERSION && binUrl.length() > 0) {
          Serial.println("[OTA] New version found. Starting HTTP Update...");
          
          t_httpUpdate_return ret = httpUpdate.update(secureClient, binUrl);

          switch (ret) {
            case HTTP_UPDATE_FAILED:
              Serial.printf("[OTA] Update failed. Error (%d): %s\n", 
                            httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
              break;
            case HTTP_UPDATE_NO_UPDATES:
              Serial.println("[OTA] No updates available.");
              break;
            case HTTP_UPDATE_OK:
              Serial.println("[OTA] Update successful. Rebooting...");
              break;
          }
        } else {
          Serial.println("[OTA] Firmware is up to date.");
        }
      } else {
        Serial.println("[OTA] Parse error: JSON formatting mismatch.");
      }
    } else {
      Serial.printf("[OTA] Version check failed, HTTP Code: %d\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("[OTA] Connection to version check URL failed.");
  }
}

// (End of file)