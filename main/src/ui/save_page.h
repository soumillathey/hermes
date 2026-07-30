#ifndef SAVE_PAGE_H
#define SAVE_PAGE_H

#include <Arduino.h>

static const char SAVE_HTML[] PROGMEM = R"rawliteral(
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
        .icon {
            width: 56px;
            height: 56px;
            background: rgba(0, 230, 118, 0.12);
            border: 2px solid rgba(0, 230, 118, 0.4);
            border-radius: 50%;
            margin: 0 auto;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 26px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="badge">Success</div>
        <h1>Settings Saved</h1>
        <p>The configuration has been written to flash. Gluvok by Lathey Weigh Trix will now connect to <b>%SSID%</b>.</p>
        <div class="icon">&#10003;</div>
    </div>
</body>
</html>
)rawliteral";

#endif // SAVE_PAGE_H
