---
events: ["agent:response", "session:message"]
emoji: "📺"
---

# OpenClaw Display Hook

Publishes Claude Code / Clawdbot messages to MQTT for the XTeInk e-ink display.

## Configuration

Set the `OPENCLAW_MQTT_TOPIC` environment variable to your unique topic:

```bash
export OPENCLAW_MQTT_TOPIC="openclaw/my-unique-id/display"
```

If not set, defaults to `openclaw/demo/display`.
