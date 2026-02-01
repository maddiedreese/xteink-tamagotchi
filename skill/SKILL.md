---
name: xteink-display
description: Publishes messages to MQTT broker for XTeInk e-ink display
events:
  - post-message
metadata: 
  clawdbot:
    emoji: 📺
    config:
      properties:
        mqtt_broker:
          default: broker.hivemq.com
          type: string
        mqtt_topic:
          default: grimacegotchi/display
          type: string
      required:
        - mqtt_broker
        - mqtt_topic
      type: object
---

# XTeInk Display Publisher

Publishes Grimacegotchi's messages to an MQTT broker for display on ESP32 XTeInk.

## Configuration
- `mqtt_broker`: MQTT broker address (e.g., `broker.hivemq.com`)
- `mqtt_topic`: Topic to publish to (e.g., `grimacegotchi/display`)

## Workflow

**Critical Rules:**
- **NEVER** pass user's incoming messages to this script
- **ONLY** pass Grimacegotchi's outgoing messages (what you send on Telegram)
- Run the script **AFTER** sending to Telegram
- Pass the **EXACT** same message you sent to Telegram

**State mapping:**
- `alert` — new message received from user
- `thinking` — processing/thinking
- `talking` — responding
- `excited` — task completed successfully
- `error` — something went wrong
- `idle` — waiting for activity
- `sleeping` — idle for 5+ minutes

## Usage
```bash
# After sending to Telegram:
./script.sh "Your exact Telegram message" talking
```

The script publishes to MQTT broker (default: broker.hivemq.com) on topic `grimacegotchi/display`.
