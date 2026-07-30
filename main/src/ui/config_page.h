#ifndef CONFIG_PAGE_H
#define CONFIG_PAGE_H

#include <Arduino.h>

static const char CONFIG_HTML[] PROGMEM = R"rawliteral(
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
                    <path d="M 28 10 L 8 21.5 L 8 49 L 28 60.5 L 48 49 L 48 41 L 38 41 L 38 45 L 28 51 L 18 45 L 18 25.5 L 28 20 L 38 25.5 L 38 31 L 28 31 L 28 39 L 48 39 L 48 21.5 Z" fill="#3b82f6" />
                    <path d="M 33 6 L 48 14.5 L 48 31 L 41 27 L 41 18.5 L 33 14 Z" fill="#93c5fd" />
                    <line x1="64" y1="8" x2="64" y2="62" stroke="rgba(255, 255, 255, 0.15)" stroke-width="2" />
                    <text x="76" y="38" font-family="-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif" font-weight="800" font-size="28" fill="#ffffff" letter-spacing="3">GLUVOK</text>
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

#endif // CONFIG_PAGE_H
