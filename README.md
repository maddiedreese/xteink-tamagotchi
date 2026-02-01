# XTeInk Tamagotchi

Display your OpenClaw / Clawdbot / MoltBot AI assistant's activity on a portable e-ink display in real-time using a **Clawdbot skill**.

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![ESP32-C3](https://img.shields.io/badge/ESP32-C3-blue)](https://www.espressif.com/en/products/socs/esp32-c3)
[![MQTT](https://img.shields.io/badge/MQTT-broker.hivemq.com-orange)](https://www.hivemq.com)
[![Clawdbot Skill](https://img.shields.io/badge/Clawdbot-Skill-blue)](https://docs.clawd.bot)

---

## Table of Contents

1. [What is XTeInk Tamagotchi?](#what-is-xteink-tamagotchi)
2. [How It Works (Architecture)](#how-it-works-architecture)
3. [Hardware Requirements](#hardware-requirements)
4. [Software Prerequisites](#software-prerequisites)
5. [Complete Setup Guide](#complete-setup-guide)
   - [Step 1: Clone the Repository](#step-1-clone-the-repository)
   - [Step 2: Configure and Flash Firmware](#step-2-configure-and-flash-firmware)
   - [Step 3: Set Up WiFi](#step-3-set-up-wifi)
   - [Step 4: Install the Clawdbot Skill](#step-4-install-the-clawdbot-skill)
   - [Step 5: Configure the Skill](#step-5-configure-the-skill)
   - [Step 6: Configure Your Agent to Auto-Run the Skill](#step-6-configure-your-agent-to-auto-run-the-skill)
   - [Step 7: Use the Skill](#step-7-use-the-skill)
   - [Step 8: Test It](#step-8-test-it)
6. [Understanding the Components](#understanding-the-components)
   - [Firmware](#firmware)
   - [The Skill](#the-skill)
   - [MQTT Message Format](#mqtt-message-format)
7. [Customization](#customization)
   - [Custom Sprites](#custom-sprites)
   - [Configuration Options](#configuration-options)
8. [Troubleshooting](#troubleshooting)
9. [Project Structure](#project-structure)
10. [FAQ](#faq)
11. [Contributing](#contributing)
12. [License](#license)
13. [Acknowledgments](#acknowledgments)

---

## What is XTeInk Tamagotchi?

XTeInk Tamagotchi transforms an XTeInk X4 e-ink display into a companion for your AI assistant. When your OpenClaw/Clawdbot/MoltBot responds to messages, thinks, or works on tasks, the display updates in real-time to show:

| Feature | Description |
|---------|-------------|
| **Mood Sprites** | 8 different states (sleeping, idle, alert, thinking, talking, working, excited, error) |
| **Activity Status** | What the assistant is currently doing (e.g., "Processing...", "Responding...") |
| **Messages** | Up to 288 characters of the latest response |
| **System Status** | Battery percentage, WiFi connection, and MQTT connection status |

---

## How It Works (Architecture)

```
┌─────────────────────────────────────────────────────────────────┐
│                        YOUR COMPUTER                             │
│                                                                 │
│  ┌──────────────┐     ┌──────────────────┐     ┌─────────────┐  │
│  │  Clawdbot    │────▶│   Clawdbot       │────▶│ mosquitto_pub│  │
│  │  (main app)  │     │   Skill          │     │  (MQTT CLI)  │  │
│  │              │     │   (manual)       │     │              │  │
│  └──────────────┘     └──────────────────┘     └──────┬──────┘  │
│                                                        │         │
│                                                        │ MQTT    │
│                                                        ▼         │
│                                                broker.hivemq.com │
│                                                         │         │
└────────────────────────────────────────────────────────┼─────────┘
                                                         │
                              ┌──────────────────────────┼──────────┐
                              │                          │          │
                              ▼                          ▼          │
                    ┌─────────────────┐        ┌─────────────────┐  │
                    │   XTeInk X4     │        │   Other Users   │  │
                    │   (Display)     │        │   (Same topic)  │  │
                    │   ESP32-C3      │◀───────│   can subscribe │  │
                    │   Subscribes    │        │                 │  │
                    └─────────────────┘        └─────────────────┘  │
```

### Data Flow

1. **Clawdbot** processes a user message and generates a response
2. **You send the response** to Telegram (or your messaging platform)
3. **You run the skill** (`xteink-display` skill) with your message and state
4. **mosquitto_pub** publishes a JSON message to the MQTT broker
5. **The XTeInk** (subscribed to the same topic) receives the message
6. **The firmware** parses the JSON and updates the e-ink display

The skill approach gives you:
- ✅ Full control over when/what is displayed
- ✅ Works with any messaging platform (Telegram, Discord, Signal, etc.)
- ✅ No automatic publishing of sensitive/thought messages
- ✅ Can customize state (thinking, talking, excited, etc.) per message

---

## Hardware Requirements

| Item | Description |
|------|-------------|
| **XTeInk X4** | ESP32-C3 based 4.26" e-ink display (800×480 pixels) |
| **USB-C Cable** | To power and flash the device |
| **Computer** | macOS or Linux for development |

**Where to buy:** [XTeInk Store](https://www.xteink.com)

No Raspberry Pi, no additional batteries, no soldering required. The XTeInk X4 is a complete, self-contained device.

---

## Software Prerequisites

### Required Tools

```bash
# 1. PlatformIO CLI (for building and flashing firmware)
# Install via pip:
pip install platformio

# 2. MQTT client tools (for the skill to publish messages)
# macOS:
brew install mosquitto

# Linux (Debian/Ubuntu):
sudo apt install mosquitto-clients

# 3. Git (for cloning the repository)
# Already installed on most systems
```

### Verify Installation

```bash
# Check PlatformIO
pio --version

# Check MQTT client
which mosquitto_pub

# Check Git
git --version
```

---

## Complete Setup Guide

### Step 1: Clone the Repository

```bash
git clone https://github.com/maddiedreese/xteink-tamagotchi.git
cd xteink-tamagotchi
```

### Step 2: Configure and Flash Firmware

#### 2.1 Edit the Configuration

Open `firmware/src/main.cpp` and modify the MQTT topic:

```cpp
// Around line 20-25
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// CHANGE THIS to something unique for your setup!
const char* MQTT_TOPIC = "tamagotchi/your-unique-id/display";
```

**Choose a unique topic** to avoid conflicts with other users. Examples:
- `tamagotchi/alice-macbook/display`
- `tamagotchi/myname-2024/display`
- `tamagotchi/random123xyz/display`

#### 2.2 Connect the XTeInk

1. Connect your XTeInk X4 to your computer via USB-C
2. The device should appear as a serial device (e.g., `/dev/ttyUSB0` on Linux or `/dev/cu.usbserial-xxx` on macOS)

#### 2.3 Build and Flash

```bash
cd firmware
pio run -t upload
```

This will:
1. Download required libraries
2. Compile the firmware
3. Flash it to the XTeInk X4

---

### Step 3: Set Up WiFi

After flashing, the XTeInk will create a WiFi network called **"Tamagotchi-Setup"**.

1. **Connect to this network** from your phone or computer
2. **A captive portal should appear** automatically. If not, open a browser and navigate to `http://192.168.4.1`
3. **Select your WiFi network** from the list
4. **Enter your WiFi password**
5. **Click "Save"**

The device will reboot and connect to your WiFi. The display should show:
- "WiFi: OK" in the bottom status bar
- "MQTT: OK" once connected to the broker

> **Note:** The WiFi setup portal will automatically start every 3 minutes if WiFi connection is lost.

---

### Step 4: Install the Clawdbot Skill

The skill is a Clawdbot plugin that lets you manually publish messages to the e-ink display. Copy it to your skills directory:

```bash
# Copy the skill to your Clawdbot skills folder
mkdir -p ~/.clawdbot/skills
cp -r /path/to/xteink-tamagotchi/skill ~/.clawdbot/skills/xteink-display

# Or if using the main repo's skill location:
cp -r skill ~/.clawdbot/skills/xteink-display
```

The skill is now installed! You can verify it by checking:
```bash
ls ~/.clawdbot/skills/xteink-display/
# Should show: SKILL.md  script.sh
```

---

### Step 5: Configure the Skill

The skill needs your MQTT topic configuration. You can configure it in the skill's SKILL.md or pass parameters directly:

#### Option A: Edit SKILL.md

Open `~/.clawdbot/skills/xteink-display/SKILL.md` and update the metadata:

```yaml
metadata: 
  clawdbot:
    config:
      properties:
        mqtt_broker:
          default: broker.hivemq.com
          type: string
        mqtt_topic:
          default: tamagotchi/your-unique-id/display
          type: string
      required:
        - mqtt_broker
        - mqtt_topic
      type: object
```

#### Option B: Use Default Topic

The skill defaults to `grimacegotchi/display`. To use a custom topic, edit the script:

```bash
# Open the script
nano ~/.clawdbot/skills/xteink-display/script.sh

# Change the topic variable:
TOPIC="tamagotchi/your-unique-id/display"
```

> **Important:** Use the **same topic** you configured in the firmware!

---

### Step 7: Use the Skill

After sending a message to your AI assistant on Telegram (or any platform), run the skill to update the display:

```bash
# Navigate to the skill directory
cd ~/.clawdbot/skills/xteink-display

# Run the script with your message and state
./script.sh "Your message here" talking

# Examples for different states:
./script.sh "Processing your request..." thinking
./script.sh "Task completed!" excited
./script.sh "Oops, something went wrong" error
./script.sh "" idle
```

#### Available States

| State | When to Use |
|-------|-------------|
| `alert` | New message received from user |
| `thinking` | Processing/thinking about the request |
| `talking` | Responding to user |
| `excited` | Task completed successfully |
| `error` | Something went wrong |
| `idle` | Waiting for activity |
| `sleeping` | Idle for 5+ minutes |

---

### Step 6: Configure Your Agent to Auto-Run the Skill

**Important:** You need to configure your Clawdbot agent to run the skill **after every outgoing message**. This ensures the display updates automatically when you respond to users.

#### Option A: Add to Agent's Instructions (Recommended)

Add this to your agent's SOUL.md or AGENTS.md:

```markdown
## XTeInk Display

After every message you send to users (on Telegram, Discord, Signal, etc.), immediately run the XTeInk skill:

```bash
cd ~/.clawdbot/skills/xteink-display && ./script.sh "Your exact message here" talking
```

- Use `talking` when responding to users
- Use `excited` when tasks complete successfully
- Use `error` when something goes wrong
- Pass the **exact same message** you sent to the user
```

#### Option B: Add to Daily Notes Template

If you use daily notes or templates, add a reminder to run the skill after each response.

---

### Step 8: Test It

1. **Start Clawdbot:**
   ```bash
   clawdbot
   ```

2. **Send a message** to your AI assistant

3. **After responding**, run the skill:
   ```bash
   cd ~/.clawdbot/skills/xteink-display
   ./script.sh "Your response here" talking
   ```

4. **Watch your XTeInk display** — it should:
   - Show the "talking" sprite
   - Display your message
   - Show "Responding..." as the activity

---

## Understanding the Components

### Firmware

The firmware (`firmware/src/main.cpp`) runs on the ESP32-C3 and:

| Function | Description |
|----------|-------------|
| **WiFi Connection** | Connects to your WiFi network using WiFiManager |
| **MQTT Subscription** | Subscribes to your unique topic |
| **Display Rendering** | Draws sprites, text, and status bar |
| **State Management** | Tracks mood, message, and activity |
| **Battery Monitoring** | Reads and displays battery level |

**Key Configuration (in `firmware/src/main.cpp`):**
```cpp
const char* MQTT_SERVER = "broker.hivemq.com";  // MQTT broker
const int MQTT_PORT = 1883;                      // MQTT port
const char* MQTT_TOPIC = "tamagotchi/demo/display"; // Your topic
const char* AP_NAME = "Tamagotchi-Setup";        // WiFi setup AP name
```

---

### The Skill

The skill (`skill/script.sh`) is a Clawdbot skill that you manually invoke to publish messages to the e-ink display.

**How to use:**
```bash
./script.sh "Your message" talking
```

**What it does:**
1. **Receives your message and state** as command-line arguments
2. **Builds the JSON payload** (message, state, activity)
3. **Publishes to MQTT** using `mosquitto_pub`
4. **Updates the e-ink display** with your message and current state

**Key features:**
- Manual control - you decide when/what to publish
- Works with any messaging platform (Telegram, Discord, Signal, etc.)
- No automatic publishing of sensitive content
- Full control over states (alert, thinking, talking, excited, error, idle, sleeping)

**Configuration (in `skill/script.sh`):**
```bash
BROKER="broker.hivemq.com"       # MQTT broker
TOPIC="tamagotchi/demo/display"  # Your unique topic
MAX_CHARS=288                    # Max message length (display limit)
```

---

### MQTT Message Format

The skill publishes JSON messages in this format:

```json
{
  "message": "The text to display (up to 288 chars)",
  "state": "talking",
  "activity": "Responding..."
}
```

#### Valid State Values

| State | Meaning | Sprite Displayed |
|-------|---------|------------------|
| `sleeping` | Assistant is idle for 5+ minutes | 😴 Sleeping |
| `idle` | Waiting for messages | 🟢 Idle |
| `alert` | New user message received | ⚠️ Alert |
| `thinking` | Processing the request | 💭 Thinking |
| `talking` | Responding to user | 💬 Talking |
| `working` | Executing tasks | 🔧 Working |
| `excited` | Task completed successfully | 🎉 Excited |
| `error` | Something went wrong | ❌ Error |

---

## Customization

### Custom Sprites

You can replace the default sprites with your own designs. Here's our actual workflow:

#### Our Sprite Creation Workflow

1. **Generate with Nano Banana Pro**
   - Use Gemini 3 Pro Image (Nano Banana Pro) to generate the initial sprite design
   - Get a clean PNG with your character/design

2. **Remove Background with Canva Pro**
   - Upload to Canva
   - Use "Remove Background" tool to get a clean transparent background
   - Export as PNG

3. **Convert to Pixel Art with Piskel**
   - Import the PNG into Piskel (piskelapp.com)
   - Resize to 200×200 pixels
   - Use the pixel art tools to clean up and stylize
   - Export as PNG

4. **Run the Converter**
   ```bash
   # Install dependencies first
   pip install platformio
   python3 convert_sprites.py
   ```

5. **Copy Generated Headers**
   ```bash
   cp ~/clawdbot-tamagotchi/xteink-firmware/include/*.h firmware/include/
   ```

6. **Rebuild and Flash**
   ```bash
   cd firmware
   pio run -t upload
   ```

#### Sprite Requirements

| Requirement | Details |
|-------------|---------|
| Size | 200×200 pixels |
| Format | PNG with transparent background |
| Style | Pixel art (1-bit black and white works best) |

#### Sprite Names

```
sprites/
├── sleeping.png
├── idle.png
├── alert.png
├── thinking.png
├── talking.png
├── working.png
├── excited.png
└── error.png
```

---

### Configuration Options

#### Firmware (`firmware/src/main.cpp`)

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `MQTT_SERVER` | `const char*` | `broker.hivemq.com` | MQTT broker hostname |
| `MQTT_PORT` | `int` | `1883` | MQTT broker port |
| `MQTT_TOPIC` | `const char*` | `tamagotchi/demo/display` | MQTT topic to subscribe |
| `MQTT_CLIENT_ID` | `const char*` | `tamagotchi-xteink` | Unique client ID |
| `AP_NAME` | `const char*` | `Tamagotchi-Setup` | WiFi setup portal name |

#### Skill (script.sh Configuration)

| Option | Default | Description |
|--------|---------|-------------|
| `BROKER` | `broker.hivemq.com` | MQTT broker hostname |
| `TOPIC` | `tamagotchi/demo/display` | MQTT topic to publish |
| `MAX_CHARS` | `288` | Max message length (display limit) |

---

## Troubleshooting

### Display stuck on "Starting..."

**Cause:** WiFi captive portal is waiting for configuration.

**Solution:**
1. Connect to "Tamagotchi-Setup" WiFi network
2. Follow the setup wizard to enter your WiFi credentials

---

### Display shows "WiFi: --"

**Cause:** WiFi connection was lost.

**Solution:**
1. The device will create a setup portal every 3 minutes
2. Connect to "Tamagotchi-Setup" and reconfigure WiFi
3. Or wait for the auto-reconnect attempt

---

### Display shows "MQTT: --"

**Cause:** Cannot connect to MQTT broker.

**Solution:**
1. Check your internet connection
2. Verify the MQTT broker is online (`broker.hivemq.com` is a public broker)
3. The device will auto-reconnect every 5 seconds

---

### Skill not working

**Checklist:**

1. **Verify mosquitto_pub is installed:**
   ```bash
   which mosquitto_pub
   ```

2. **Check the topic matches:**
   - Firmware topic: `firmware/src/main.cpp` → `MQTT_TOPIC`
   - Skill topic: `skill/script.sh` → `TOPIC`

3. **Test manually:**
   ```bash
   mosquitto_pub -h broker.hivemq.com -t "tamagotchi/test/display" \
     -m '{"message":"Hello!","state":"talking","activity":"Testing"}'
   ```

4. **Check the skill script is executable:**
   ```bash
   chmod +x skill/script.sh
   ```

5. **Run with verbose output:**
   ```bash
   cd skill
   ./script.sh "Test message" talking
   ```

---

### Messages are truncated

**Cause:** Maximum message length is 288 characters (display limitation).

**Solution:** The skill automatically truncates messages. Longer messages will end with "..." on the display.

---

### Display doesn't update

**Solution:**
1. Check the display isn't in "sleeping" mode (wait or send a message)
2. Verify WiFi and MQTT show "OK" in the status bar
3. Press the reset button on the XTeInk

---

## Project Structure

```
xteink-tamagotchi/
├── firmware/                    # ESP32 PlatformIO project
│   ├── src/
│   │   └── main.cpp            # Main firmware (C++)
│   ├── include/
│   │   ├── sprites.h           # Sprite includes (auto-generated)
│   │   ├── sprite_*.h          # Individual sprite headers
│   │   └── FreeMonoBold*.h     # Font files
│   └── platformio.ini          # PlatformIO configuration
│
├── skill/                       # Clawdbot skill for publishing to display
│   ├── SKILL.md                # Skill metadata and configuration
│   └── script.sh               # Shell script for publishing to MQTT
│
├── sprites/                     # Source sprite images (200x200 PNG)
│   ├── sleeping.png
│   ├── idle.png
│   ├── alert.png
│   ├── thinking.png
│   ├── talking.png
│   ├── working.png
│   ├── excited.png
│   └── error.png
│
├── convert_sprites.py           # PNG → C header converter
├── README.md                    # This file
└── LICENSE                      # MIT License
```

---

## FAQ

### Can I use a different MQTT broker?

Yes! Change both:

1. **Firmware** (`firmware/src/main.cpp`):
   ```cpp
   const char* MQTT_SERVER = "your-broker.com";
   ```

2. **Hook** (environment variable):
   ```bash
   export TAMAGOTCHI_MQTT_BROKER="your-broker.com"
   ```

---

### Can I run the XTeInk without Clawdbot?

Yes! The XTeInk is just an MQTT subscriber. You can publish messages from any source:

```bash
mosquitto_pub -h broker.hivemq.com -t "tamagotchi/your-topic/display" \
  -m '{"message":"Hello!","state":"talking","activity":"Testing"}'
```

---

### How long does the battery last?

The XTeInk X4 has a built-in battery. With typical usage (a few messages per day), expect several days of operation. The display only consumes power when refreshing.

---

### Can I use this with other AI assistants?

Yes! The display works with any AI assistant that can publish MQTT messages. You'll need to:

1. Create a custom integration for your assistant
2. Publish JSON in the [correct format](#mqtt-message-format)
3. Subscribe to the same topic

---

## Contributing

Contributions are welcome! Here are some ways you can help:

- **Bug reports** — Found an issue? Open an issue!
- **Feature requests** — Have an idea? Let us know!
- **Pull requests** — Fix a bug or add a feature
- **Documentation** — Improve this README or add examples

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

## License

This project is licensed under the MIT License.

See the [LICENSE](LICENSE) file for full details.

---

## Acknowledgments

- **OpenClaw Team** — For the amazing AI assistant framework
- **Clawdbot Community** — For inspiration and feedback
- **ZinggJM** — For the [GxEPD2](https://github.com/ZinggJM/GxEPD2) e-ink library
- **HiveMQ** — For the free public MQTT broker
- **XTeInk** — For the excellent hardware

---

## Support

- **Issues:** [GitHub Issues](https://github.com/maddiedreese/xteink-tamagotchi/issues)
- **Discussions:** [GitHub Discussions](https://github.com/maddiedreese/xteink-tamagotchi/discussions)

---

Made with 🫡 for the OpenClaw/Clawdbot community
