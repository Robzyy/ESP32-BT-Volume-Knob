#include <Arduino.h>
#include <BleKeyboard.h>
#include "Encoder.h"

#define BUTTON_HOLD_TIME 600 // Time (ms) to differentiate between a short press and a holding down the button
#define LED_BLINK_FREQ 1000  // Built-in led blink frequency (ms)

BleKeyboard kb;

Encoder e;

uint64_t button_ms, blink_ms;
bool block_hold = false;
bool LED_state = LOW;

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize bluetooth keyboard lib
  kb.begin();

  pinMode(BUILTIN_LED, OUTPUT);
  button_ms = blink_ms = millis();
}

void loop()
{
  while (kb.isConnected())
  {
    // Make sure built-in LED is off
    LED_state = LOW;
    digitalWrite(BUILTIN_LED, LED_state);

    // ================================
    //   Rotation part of the encoder
    // ================================
    switch (e.getRotationDirection())
    {
    case -1:
      // Serial.println("Left");
      kb.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    case 1:
      // Serial.println("Right");
      kb.write(KEY_MEDIA_VOLUME_UP);
      break;
    default:
      break;
    }

    // ================================
    //    Button part of the encoder
    // ================================

    // Register when the button was pressed
    if (!e.getButtonStates().first && e.getButtonStates().second)
    {
      button_ms = millis();
    }

    // If time for long press is reached and button is still pressed, play next track
    if (e.getButtonStates().second && millis() - button_ms == BUTTON_HOLD_TIME && !block_hold)
    {
      // Serial.println("MEDIA NEXT");
      kb.write(KEY_MEDIA_NEXT_TRACK);

      // We do this to prevent continous skipping while button is held down
      block_hold = true;
      delay(100); // Sometimes it also sends play/pause, I suppose this is because of bouncing, delay should solve that
    }

    // If button was released and haven't sent MEDIA NEXT
    else if (e.getButtonStates().first && !e.getButtonStates().second && !block_hold)
    {
      // Serial.println("MEDIA PLAY/PAUSE");
      kb.write(KEY_MEDIA_PLAY_PAUSE);
      delay(100); // Sometimes the key is sent twice, I suppose this is because of bouncing, delay should solve that
    }

    // On button release, disable the block for the next button hold
    if (e.getButtonStates().first && !e.getButtonStates().second)
    {
      block_hold = false;
    }
  }

  // Serial.println("Disconnected!");
  digitalWrite(BUILTIN_LED, LED_state);
  if (millis() - blink_ms == LED_BLINK_FREQ)
  {
    LED_state = !LED_state;
    blink_ms = millis();
  }
}
