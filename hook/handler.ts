/**
 * OpenClaw Display Hook
 *
 * Publishes Claude Code / Clawdbot messages to MQTT for the XTeInk e-ink display.
 * Requires mosquitto_pub to be installed: brew install mosquitto (macOS) or apt install mosquitto-clients (Linux)
 */

import { execSync } from 'child_process';

// Configuration - customize via environment variables
const MQTT_BROKER = process.env.OPENCLAW_MQTT_BROKER || 'broker.hivemq.com';
const MQTT_TOPIC = process.env.OPENCLAW_MQTT_TOPIC || 'openclaw/demo/display';

// Maximum message length (matches firmware display capacity)
const MAX_MESSAGE_LENGTH = 288;

export default async (event: any) => {
  let message = '';
  let state = 'idle';
  let activity = '';

  // Extract message and determine state based on event type
  if (event.type === 'agent' && event.action === 'response') {
    message = String(event.content || event.text || '').slice(0, MAX_MESSAGE_LENGTH);
    state = 'talking';
    activity = 'Responding...';
  } else if (event.type === 'session' && event.action === 'message') {
    message = String(event.content || '').slice(0, MAX_MESSAGE_LENGTH);
    state = event.role === 'assistant' ? 'talking' : 'alert';
    activity = event.role === 'assistant' ? 'Responding...' : 'New message!';
  } else {
    // Skip other event types
    return;
  }

  if (!message) return;

  // Build JSON payload
  const payload = JSON.stringify({
    message: message,
    state: state,
    activity: activity
  });

  try {
    execSync(
      `mosquitto_pub -h "${MQTT_BROKER}" -t "${MQTT_TOPIC}" -m '${payload.replace(/'/g, "'\\''")}'`,
      { timeout: 5000 }
    );
    console.log(`[openclaw] Published: ${state} - ${activity}`);
  } catch (err) {
    console.error('[openclaw] Failed to publish:', err);
  }
};
