# XTeInk Tamagotchi - E-Ink Display for OpenClaw

Display your OpenClaw / Clawdbot / MoltBot AI assistant's activity on a portable e-ink display in real-time.

![Hardware](https://img.shields.io/badge/Hardware-XTeInk_X4-blue)
![License](https://img.shields.io/badge/License-MIT-green)

## What is XTeInk Tamagotchi?

XTeInk Tamagotchi turns an XTeInk X4 e-ink display into a "Tamagotchi" for your AI assistant. When your OpenClaw/Clawdbot/MoltBot responds to messages, thinks, or works on tasks, the display updates in real-time to show:

- **Mood sprites** - 8 different states (sleeping, idle, alert, thinking, talking, working, excited, error)
- **Activity status** - What the assistant is currently doing
- **Messages** - Up to 288 characters of the latest response
- **System status** - Battery, WiFi, and MQTT connection status

## Hardware Required

- **XTeInk X4** - ESP32-C3 based 4.26" e-ink display (800x480)
  - Available from: [XTeInk Store](https://www.xteink.com)

That's it! No Raspberry Pi, no additional batteries, no soldering required.

## How It Works

```
┌─────────────┐     MQTT      ┌─────────────┐
│  OpenClaw   │ ──────────────▶ │  XTeInk X4  │
│  + Hook     │  (push-based) │  (display)  │
└─────────────┘               └─────────────┘
       │                             │
       │ publishes to                │ subscribes to
       ▼                             ▼
   broker.hivemq.com/tamagotchi/<your-id>/display
```

1. A hook in OpenClaw/Clawdbot captures assistant responses
2. The hook publishes messages to a public MQTT broker
3. The XTeInk subscribes to the same topic and displays updates instantly

## Complete Setup Guide

### Step 1: Install Prerequisites

**On your computer (macOS/Linux):**

```bash
# Install PlatformIO CLI
pip install platformio

# Install MQTT client (for the hook)
# macOS:
brew install mosquitto

# Linux:
sudo apt install mosquitto-clients
```

### Step 2: Choose Your Unique MQTT Topic

Since we use a public MQTT broker, you need a unique topic to avoid conflicts with other users.

Pick something unique like:
- `tamagotchi/alice-macbook/display`
- `tamagotchi/myname-2024/display`
- `tamagotchi/random123xyz/display`

**Remember this topic** - you'll use it in both the firmware and the hook.

### Step 3: Configure and Flash the Firmware

1. **Clone this repository:**
   ```bash
   git clone https://github.com/maddiedreese/xteink-tamagotchi.git
   cd xteink-tamagotchi
   ```

2. **Edit the firmware configuration:**

   Open `firmware/src/main.cpp` and find the CONFIGURATION section near the top:

   ```cpp
   // MQTT Topic - change this to something unique for your setup!
   const char* MQTT_TOPIC = "tamagotchi/demo/display";
   ```

   Change `tamagotchi/demo/display` to your unique topic from Step 2.

3. **Connect your XTeInk X4 via USB**

4. **Build and flash:**
   ```bash
   cd firmware
   pio run -t upload
   ```

5. **Connect to WiFi:**
   - The XTeInk will create a WiFi network called "Tamagotchi-Setup"
   - Connect to it with your phone or computer
   - A captive portal will appear - select your WiFi network and enter the password
   - The device will reboot and connect to your WiFi

6. **Verify connection:**
   - The display should show "Connected to MQTT"
   - The bottom status bar should show "WiFi: OK" and "MQTT: OK"

### Step 4: Install the OpenClaw/Clawdbot Hook

The hook sends messages from your AI assistant to the display.

1. **Find your hooks directory:**
   ```bash
   # For Clawdbot:
   ~/.clawdbot/hooks/

   # For OpenClaw:
   ~/.openclaw/hooks/
   ```

2. **Copy the hook:**
   ```bash
   cp -r hook ~/.clawdbot/hooks/xteink-display
   ```

3. **Set your MQTT topic:**

   Add to your shell profile (`~/.bashrc`, `~/.zshrc`, etc.):
   ```bash
   export TAMAGOTCHI_MQTT_TOPIC="tamagotchi/your-unique-id/display"
   ```

   Replace `your-unique-id` with the same topic you used in the firmware.

4. **Reload your shell:**
   ```bash
   source ~/.zshrc  # or ~/.bashrc
   ```

### Step 5: Test It!

1. Start an OpenClaw/Clawdbot session
2. Send a message
3. Watch your XTeInk display update with the response!

## Customization

### Custom Sprites

The display shows different sprites based on the AI's mood. You can customize these:

1. Create 200x200 pixel PNG images (black and white)
2. Name them: `sleeping.png`, `idle.png`, `alert.png`, `thinking.png`, `talking.png`, `working.png`, `excited.png`, `error.png`
3. Place them in the `sprites/` folder
4. Run the converter:
   ```bash
   python3 convert_sprites.py
   ```
5. Copy the generated headers to `firmware/include/`
6. Rebuild and flash the firmware

### Configuration Options

**Firmware (`firmware/src/main.cpp`):**
- `MQTT_SERVER` - MQTT broker address (default: broker.hivemq.com)
- `MQTT_PORT` - MQTT port (default: 1883)
- `MQTT_TOPIC` - Your unique topic
- `AP_NAME` - WiFi setup network name

**Hook (environment variables):**
- `TAMAGOTCHI_MQTT_BROKER` - MQTT broker (default: broker.hivemq.com)
- `TAMAGOTCHI_MQTT_TOPIC` - Your unique topic

## Troubleshooting

### Display stuck on "Starting..."
- The WiFi captive portal may be waiting for configuration
- Connect to the "Tamagotchi-Setup" WiFi network

### Display shows "WiFi: --"
- WiFi connection lost
- The device will automatically create the setup portal every 3 minutes
- Connect to "Tamagotchi-Setup" to reconfigure

### Display shows "MQTT: --"
- MQTT broker connection failed
- Check your internet connection
- The public broker may be temporarily unavailable - it will auto-reconnect

### Hook not publishing
- Verify `mosquitto_pub` is installed: `which mosquitto_pub`
- Check the topic matches between firmware and hook
- Test manually:
  ```bash
  mosquitto_pub -h broker.hivemq.com -t "tamagotchi/test/display" \
    -m '{"message":"Hello!","state":"talking","activity":"Testing"}'
  ```

### Messages truncated
- Maximum message length is 288 characters
- This is a display limitation to fit text on screen

## MQTT Message Format

Messages should be JSON with this structure:

```json
{
  "message": "The text to display (up to 288 chars)",
  "state": "talking",
  "activity": "Responding..."
}
```

**Valid states:**
- `sleeping` - Zzz...
- `idle` - Waiting
- `alert` - New input received
- `thinking` - Processing
- `talking` - Responding
- `working` - Executing tasks
- `excited` - Task completed
- `error` - Something went wrong

## Project Structure

```
xteink-tamagotchi/
├── firmware/               # ESP32 PlatformIO project
│   ├── src/main.cpp        # Main firmware code
│   ├── include/            # Sprite header files
│   └── platformio.ini      # Build configuration
├── hook/                   # OpenClaw/Clawdbot hook
│   ├── HOOK.md             # Hook metadata
│   └── handler.ts          # Hook implementation
├── sprites/                # Source sprite images (200x200 PNG)
├── convert_sprites.py      # PNG to C header converter
└── README.md               # This file
```

## Related Projects

- [OpenClaw](https://github.com/openclaw) - AI assistant framework
- [Clawdbot](https://github.com/clawdbot) - Clawdbot AI assistant
- [MoltBot](https://github.com/moltbot) - MoltBot AI assistant

## Contributing

Contributions welcome! Please feel free to submit issues and pull requests.

## License

MIT License - see LICENSE file for details.

## Acknowledgments

- Built for use with OpenClaw, Clawdbot, and MoltBot
- Uses the [GxEPD2](https://github.com/ZinggJM/GxEPD2) library for e-ink display
- MQTT provided by [HiveMQ](https://www.hivemq.com/) public broker
