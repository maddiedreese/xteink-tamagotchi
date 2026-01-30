---
events: ["agent:response", "session:message"]
emoji: "📺"
---

# XTeInk Tamagotchi Display Hook

Publishes OpenClaw / Clawdbot / MoltBot messages to MQTT for the XTeInk e-ink display.

## Configuration

Set the `TAMAGOTCHI_MQTT_TOPIC` environment variable to your unique topic:

```bash
export TAMAGOTCHI_MQTT_TOPIC="tamagotchi/my-unique-id/display"
```

If not set, defaults to `tamagotchi/demo/display`.
