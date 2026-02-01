#!/bin/bash
# XTeInk Display - MQTT Publisher for Grimacegotchi
# Usage: ./script.sh "message" [state]
# States: idle, thinking, talking, excited, sleeping, error, alert

MESSAGE="${1:-${CLAWDBOT_MESSAGE:-}}"
STATE="${2:-${CLAWDBOT_STATE:-talking}}"

# Map state names to sprite names
case "$STATE" in
    idle)         SPRITE="idle" ;;
    thinking)     SPRITE="thinking" ;;
    talking)      SPRITE="talking" ;;
    excited)      SPRITE="excited" ;;
    sleeping)     SPRITE="sleeping" ;;
    error)        SPRITE="error" ;;
    alert)        SPRITE="alert" ;;
    working)      SPRITE="working" ;;
    *)            SPRITE="talking" ;;
esac

# Exit if no message
if [ -z "$MESSAGE" ]; then
    exit 0
fi

# Write to shared file for poller
echo "$MESSAGE" > /tmp/xteink_last_message.txt

# MQTT settings
MQTT_BROKER="${CLAWDBOT_XTEINK_BROKER:-broker.hivemq.com}"
MQTT_TOPIC="${CLAWDBOT_XTEINK_TOPIC:-grimacegotchi/display}"

# Escape message for JSON (handle quotes, newlines)
ESCAPED_MESSAGE=$(echo "$MESSAGE" | head -c 288 | jq -Rs '.')

# Build JSON payload with state and sprite
PAYLOAD="{\"message\": $ESCAPED_MESSAGE, \"state\": \"$SPRITE\", \"activity\": \"New message\"}"

# Publish to MQTT
if command -v mosquitto_pub &> /dev/null; then
    mosquitto_pub -h "$MQTT_BROKER" -t "$MQTT_TOPIC" -m "$PAYLOAD"
else
    echo "Error: mosquitto_pub not found"
    exit 1
fi
