/**
 * XTeInk Tamagotchi - E-Ink Display Firmware
 *
 * Displays Tamagotchi/Clawdbot/MoltBot activity on an e-ink display via MQTT.
 * Subscribes to MQTT topic for real-time push notifications.
 *
 * GitHub: https://github.com/maddiedreese/xteink-tamagotchi
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#include "sprites.h"

// ============================================================================
// CONFIGURATION - Customize these for your setup
// ============================================================================

// MQTT Broker (using free public broker - no auth needed)
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// MQTT Topic - change this to something unique for your setup!
// Format: tamagotchi/<your-unique-id>/display
const char* MQTT_TOPIC = "tamagotchi/demo/display";

// Client ID for MQTT (should be unique per device)
const char* MQTT_CLIENT_ID = "tamagotchi-xteink";

// WiFi Setup AP name (shown when device needs WiFi configuration)
const char* AP_NAME = "Tamagotchi-Setup";

// ============================================================================
// HARDWARE PINS (XTeInk X4 specific - do not change)
// ============================================================================

#define EPD_SCLK  8
#define EPD_MOSI  10
#define EPD_CS    21
#define EPD_DC    4
#define EPD_RST   5
#define EPD_BUSY  6
#define BATTERY_PIN 0

// ============================================================================
// DISPLAY SETUP
// ============================================================================

GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
    GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 800

// ============================================================================
// STATE
// ============================================================================

enum Mood {
    MOOD_SLEEPING,
    MOOD_IDLE,
    MOOD_ALERT,
    MOOD_THINKING,
    MOOD_TALKING,
    MOOD_WORKING,
    MOOD_EXCITED,
    MOOD_ERROR
};

struct DisplayState {
    Mood mood = MOOD_IDLE;
    String lastActivity = "Waiting for messages...";
    String lastMessage = "";
    unsigned long lastEventTime = 0;
    int batteryPercent = 100;
    bool needsRedraw = true;
};

DisplayState state;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 5000;

// WiFi check interval (3 minutes)
const unsigned long WIFI_CHECK_INTERVAL = 180000;
unsigned long lastWiFiCheck = 0;

// Idle timeout (5 minutes)
const unsigned long IDLE_TIMEOUT = 300000;

// Minimum time between display refreshes
const unsigned long MIN_REFRESH_INTERVAL = 2000;
unsigned long lastDisplayUpdate = 0;

// ============================================================================
// SPRITE DATA
// ============================================================================

struct SpriteData {
    const unsigned char* data;
    int width;
    int height;
};

SpriteData getSprite(Mood mood) {
    SpriteData sprite;
    switch (mood) {
        case MOOD_SLEEPING:
            sprite.data = sprite_sleeping;
            sprite.width = SPRITE_SLEEPING_WIDTH;
            sprite.height = SPRITE_SLEEPING_HEIGHT;
            break;
        case MOOD_IDLE:
            sprite.data = sprite_idle;
            sprite.width = SPRITE_IDLE_WIDTH;
            sprite.height = SPRITE_IDLE_HEIGHT;
            break;
        case MOOD_ALERT:
            sprite.data = sprite_alert;
            sprite.width = SPRITE_ALERT_WIDTH;
            sprite.height = SPRITE_ALERT_HEIGHT;
            break;
        case MOOD_THINKING:
            sprite.data = sprite_thinking;
            sprite.width = SPRITE_THINKING_WIDTH;
            sprite.height = SPRITE_THINKING_HEIGHT;
            break;
        case MOOD_TALKING:
            sprite.data = sprite_talking;
            sprite.width = SPRITE_TALKING_WIDTH;
            sprite.height = SPRITE_TALKING_HEIGHT;
            break;
        case MOOD_WORKING:
            sprite.data = sprite_working;
            sprite.width = SPRITE_WORKING_WIDTH;
            sprite.height = SPRITE_WORKING_HEIGHT;
            break;
        case MOOD_EXCITED:
            sprite.data = sprite_excited;
            sprite.width = SPRITE_EXCITED_WIDTH;
            sprite.height = SPRITE_EXCITED_HEIGHT;
            break;
        case MOOD_ERROR:
            sprite.data = sprite_error;
            sprite.width = SPRITE_ERROR_WIDTH;
            sprite.height = SPRITE_ERROR_HEIGHT;
            break;
        default:
            sprite.data = sprite_idle;
            sprite.width = SPRITE_IDLE_WIDTH;
            sprite.height = SPRITE_IDLE_HEIGHT;
            break;
    }
    return sprite;
}

const char* getMoodName(Mood mood) {
    switch (mood) {
        case MOOD_SLEEPING: return "SLEEPING";
        case MOOD_IDLE:     return "IDLE";
        case MOOD_ALERT:    return "ALERT";
        case MOOD_THINKING: return "THINKING";
        case MOOD_TALKING:  return "TALKING";
        case MOOD_WORKING:  return "WORKING";
        case MOOD_EXCITED:  return "EXCITED";
        case MOOD_ERROR:    return "ERROR";
        default:            return "UNKNOWN";
    }
}

// ============================================================================
// BATTERY
// ============================================================================

int readBatteryPercent() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = raw * 2.0 * 3.3 / 4095.0;
    int percent = (int)((voltage - 3.0) / 1.2 * 100);
    return constrain(percent, 0, 100);
}

// ============================================================================
// DISPLAY UPDATE
// ============================================================================

void drawWrappedText(const String& text, int x, int y, int charsPerLine, int lineHeight, int maxLines) {
    String remaining = text;
    int currentY = y;
    int linesDrawn = 0;

    while (remaining.length() > 0 && linesDrawn < maxLines) {
        int breakPoint = charsPerLine;
        if (breakPoint > (int)remaining.length()) {
            breakPoint = remaining.length();
        }

        if (breakPoint < (int)remaining.length()) {
            for (int i = breakPoint; i > breakPoint / 2; i--) {
                if (remaining.charAt(i) == ' ') {
                    breakPoint = i;
                    break;
                }
            }
        }

        String line = remaining.substring(0, breakPoint);
        remaining = remaining.substring(breakPoint);
        remaining.trim();

        if (linesDrawn == maxLines - 1 && remaining.length() > 0) {
            if (line.length() > 3) {
                line = line.substring(0, line.length() - 3) + "...";
            }
        }

        display.setCursor(x, currentY);
        display.print(line);
        currentY += lineHeight;
        linesDrawn++;
    }
}

void updateDisplay(bool force = false) {
    if (!state.needsRedraw && !force) return;

    unsigned long now = millis();
    if (!force && (now - lastDisplayUpdate) < MIN_REFRESH_INTERVAL) {
        return;
    }
    lastDisplayUpdate = now;

    Serial.println("Updating display...");

    SpriteData sprite = getSprite(state.mood);

    display.setPartialWindow(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        // Sprite centered at top
        int spriteX = (DISPLAY_WIDTH - sprite.width) / 2;
        int spriteY = 50;
        display.drawBitmap(spriteX, spriteY, sprite.data,
                          sprite.width, sprite.height, GxEPD_BLACK);

        // Text below sprite
        int textY = spriteY + sprite.height + 50;

        // Mood name
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(50, textY);
        display.print(getMoodName(state.mood));

        // Divider
        textY += 20;
        display.drawLine(30, textY, DISPLAY_WIDTH - 30, textY, GxEPD_BLACK);

        // Activity
        textY += 40;
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(30, textY);
        String activity = state.lastActivity;
        if (activity.length() > 35) {
            activity = activity.substring(0, 32) + "...";
        }
        display.print(activity);

        // Last message - word wrapped, up to 288 chars
        if (state.lastMessage.length() > 0) {
            textY += 45;
            display.setFont(&FreeMonoBold12pt7b);

            String msg = state.lastMessage;
            if (msg.length() > 288) {
                msg = msg.substring(0, 285) + "...";
            }

            const int CHARS_PER_LINE = 28;
            const int LINE_HEIGHT = 24;
            const int MAX_LINES = 15;
            const int LEFT_MARGIN = 50;

            drawWrappedText(msg, LEFT_MARGIN, textY, CHARS_PER_LINE, LINE_HEIGHT, MAX_LINES);
        }

        // Bottom status bar
        display.setFont(&FreeMonoBold9pt7b);
        int bottomY = DISPLAY_HEIGHT - 30;

        display.setCursor(30, bottomY);
        display.print("Bat:");
        display.print(state.batteryPercent);
        display.print("%");

        display.setCursor(180, bottomY);
        display.print("WiFi:");
        display.print(WiFi.status() == WL_CONNECTED ? "OK" : "--");

        display.setCursor(300, bottomY);
        display.print("MQTT:");
        display.print(mqttClient.connected() ? "OK" : "--");

    } while (display.nextPage());

    state.needsRedraw = false;
    Serial.println("Display updated");
}

// ============================================================================
// MQTT CALLBACK
// ============================================================================

Mood parseMood(const char* moodStr) {
    if (!moodStr) return MOOD_IDLE;
    if (strcmp(moodStr, "sleeping") == 0) return MOOD_SLEEPING;
    if (strcmp(moodStr, "idle") == 0) return MOOD_IDLE;
    if (strcmp(moodStr, "alert") == 0) return MOOD_ALERT;
    if (strcmp(moodStr, "thinking") == 0) return MOOD_THINKING;
    if (strcmp(moodStr, "talking") == 0) return MOOD_TALKING;
    if (strcmp(moodStr, "working") == 0) return MOOD_WORKING;
    if (strcmp(moodStr, "excited") == 0) return MOOD_EXCITED;
    if (strcmp(moodStr, "error") == 0) return MOOD_ERROR;
    return MOOD_IDLE;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT message on ");
    Serial.print(topic);
    Serial.print(": ");

    char* json = (char*)malloc(length + 1);
    if (!json) {
        Serial.println("malloc failed");
        return;
    }
    memcpy(json, payload, length);
    json[length] = '\0';
    Serial.println(json);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    free(json);

    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    const char* moodStr = doc["state"] | doc["mood"];
    if (moodStr) {
        state.mood = parseMood(moodStr);
    }

    const char* message = doc["message"];
    if (message && strlen(message) > 0) {
        state.lastMessage = String(message);
        state.lastActivity = "New message received";
    }

    const char* activity = doc["activity"];
    if (activity) {
        state.lastActivity = String(activity);
    }

    state.lastEventTime = millis();
    state.needsRedraw = true;

    Serial.printf("State updated: mood=%s, msg=%s\n",
                  getMoodName(state.mood),
                  state.lastMessage.c_str());
}

// ============================================================================
// MQTT CONNECTION
// ============================================================================

bool mqttConnect() {
    Serial.print("Connecting to MQTT broker...");

    if (mqttClient.connect(MQTT_CLIENT_ID)) {
        Serial.println(" connected!");
        mqttClient.subscribe(MQTT_TOPIC);
        Serial.print("Subscribed to: ");
        Serial.println(MQTT_TOPIC);

        state.lastActivity = "Connected to MQTT";
        state.needsRedraw = true;
        return true;
    }

    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
    return false;
}

// ============================================================================
// SETUP
// ============================================================================

void showMessage(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr) {
    display.setPartialWindow(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(30, 350);
        display.print(line1);
        if (line2) {
            display.setCursor(30, 400);
            display.print(line2);
        }
        if (line3) {
            display.setFont(&FreeMonoBold12pt7b);
            display.setCursor(30, 460);
            display.print(line3);
        }
    } while (display.nextPage());
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\nTamagotchi - XTeInk X4 Display");
    Serial.println("================================");

    // Initialize SPI
    SPI.begin(EPD_SCLK, -1, EPD_MOSI, EPD_CS);

    // Initialize display
    display.init(115200);
    display.setRotation(3);
    display.setTextWrap(false);

    showMessage("Tamagotchi", "", "Starting...");

    // WiFi Manager
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(180);

    showMessage("WiFi Setup", "", "Connect to: Tamagotchi-Setup");

    if (!wifiManager.autoConnect(AP_NAME)) {
        Serial.println("WiFi failed");
        showMessage("WiFi Failed", "", "Restarting...");
        delay(3000);
        ESP.restart();
    }

    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    showMessage("Tamagotchi", "WiFi OK!", "Connecting to MQTT...");

    // Setup MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(1024);

    mqttConnect();

    // Initial display
    state.lastEventTime = millis();
    state.batteryPercent = readBatteryPercent();
    state.needsRedraw = true;
    updateDisplay(true);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Handle MQTT
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }

    // Update battery every 60 seconds
    static unsigned long lastBatteryCheck = 0;
    if (millis() - lastBatteryCheck > 60000) {
        int newBattery = readBatteryPercent();
        if (newBattery != state.batteryPercent) {
            state.batteryPercent = newBattery;
            state.needsRedraw = true;
        }
        lastBatteryCheck = millis();
    }

    // Check WiFi every 3 minutes
    if (millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected - starting config portal");
            showMessage("WiFi Lost", "", "Connect to: Tamagotchi-Setup");

            WiFiManager wifiManager;
            wifiManager.setConfigPortalTimeout(180);

            if (wifiManager.startConfigPortal(AP_NAME)) {
                Serial.println("WiFi reconnected!");
                showMessage("Tamagotchi", "WiFi OK!", "Reconnecting MQTT...");
                mqttConnect();
                state.needsRedraw = true;
            } else {
                Serial.println("Config portal timed out");
                showMessage("WiFi Failed", "", "Will retry...");
            }
            updateDisplay(true);
        }
    }

    // Idle timeout - show sleeping after 5 minutes
    unsigned long idleTime = millis() - state.lastEventTime;
    if (idleTime > IDLE_TIMEOUT) {
        if (state.mood != MOOD_SLEEPING) {
            state.mood = MOOD_SLEEPING;
            state.lastActivity = "Zzz...";
            state.needsRedraw = true;
        }
    }

    // Update display if needed
    updateDisplay();

    delay(10);
}
