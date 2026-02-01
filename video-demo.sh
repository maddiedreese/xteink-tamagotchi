#!/bin/bash
# XTeInk Sprite Demo Script for Video
# Goes through each sprite state for filming

cd ~/.clawdbot/skills/xteink-display

MESSAGE="Hi from @maddiedreese!"

echo "Starting sprite demo in 3 seconds..."
sleep 3

echo "Sending idle sprite (5 second hold)..."
./script.sh "$MESSAGE" idle
sleep 5

echo "Sending alert sprite (2 second hold)..."
./script.sh "$MESSAGE" alert
sleep 2

echo "Sending thinking sprite (2 second hold)..."
./script.sh "$MESSAGE" thinking
sleep 2

echo "Sending talking sprite (2 second hold)..."
./script.sh "$MESSAGE" talking
sleep 2

echo "Sending excited sprite (2 second hold)..."
./script.sh "$MESSAGE" excited
sleep 2

echo "Sending sleeping sprite (2 second hold)..."
./script.sh "$MESSAGE" sleeping
sleep 2

echo "Sending working sprite (2 second hold)..."
./script.sh "$MESSAGE" working
sleep 2

echo "Sending error sprite (2 second hold)..."
./script.sh "$MESSAGE" error
sleep 2

echo "Demo complete!"
