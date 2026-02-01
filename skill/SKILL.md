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
          default: tamagotchi/your-unique-id/display
          type: string
      required:
        - mqtt_broker
        - mqtt_topic
      type: object
---

# XTeInk Display Publisher

Publishes messages to an MQTT broker for display on ESP32 XTeInk.

## Configuration
- `mqtt_broker`: MQTT broker address (e.g., `broker.hivemq.com`)
- `mqtt_topic`: Topic to publish to (e.g., `tamagotchi/your-unique-id/display`)

## Workflow

**Critical Rules:**
- **NEVER** pass user's incoming messages to this script
- **ONLY** pass the AI assistant's outgoing messages
- Run the script **AFTER** sending your response
- Pass the **EXACT** same message you sent

**State mapping:**
- `idle` — default, waiting for activity
- `alert` — new message received from user
- `thinking` — processing/thinking
- `talking` — responding
- `working` — using a tool
- `excited` — task completed successfully
- `error` — something went wrong
- `sleeping` — idle for 5+ minutes

## Usage
```bash
# After responding to a user:
./script.sh "Your response message here" talking
```

The script publishes to MQTT broker (default: broker.hivemq.com) on your configured topic.
