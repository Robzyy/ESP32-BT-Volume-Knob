#include <Arduino.h>
#include <BleKeyboard.h>

#define S1_ENCODER_PIN 21
#define S2_ENCODER_PIN 22
#define KEY_ENCODER_PIN 5

#define KEY_SEND_MULTIPLIER 2
#define BUTTON_HOLD_TIME 600

BleKeyboard kb;

uint64_t ms, blink_ms;
bool block_hold = false;
bool LED_state = LOW;

class Encoder
{
private:
  bool m_s1 = 0, m_s2 = 0, m_key = 0;
  bool m_last_s1 = 0, m_last_s2 = 0, m_last_key = 0;

public:
  Encoder() : m_s1{!digitalRead(S1_ENCODER_PIN)},
              m_s2{!digitalRead(S2_ENCODER_PIN)},
              m_key{!digitalRead(KEY_ENCODER_PIN)}
  {
  }
  void updateValues()
  {
    m_last_s1 = m_s1;
    m_last_s2 = m_s2;
    m_last_key = m_key;

    m_s1 = !digitalRead(S1_ENCODER_PIN);
    m_s2 = !digitalRead(S2_ENCODER_PIN);
    m_key = !digitalRead(KEY_ENCODER_PIN);
  }

  int16_t getRotationDirection() // 1 for right; -1 for left; 0 for stationary
  {
    if (m_last_s1 && m_last_s2)
    {
      if (!m_s1 && m_s2)
      {
        return 1;
      }
      else if (m_s1 && !m_s2)
      {
        return -1;
      }
    }
    return 0;
  }

  std::pair<bool, bool> getButtonStates()
  {
    return {m_last_key, m_key};
  }

  void printValues()
  {
    Serial.print("s1 = ");
    Serial.print(m_s1);
    Serial.print(" s2 = ");
    Serial.print(m_s2);
    Serial.print(" key = ");
    Serial.print(m_key);
    Serial.print("\n");
  }
  void printLastValues()
  {
    Serial.print("last s1 = ");
    Serial.print(m_last_s1);
    Serial.print(" last s2 = ");
    Serial.print(m_last_s2);
    Serial.print(" last key = ");
    Serial.print(m_last_key);
    Serial.print("\n");
  }
} e;

void setup()
{
  Serial.begin(115200);
  pinMode(S1_ENCODER_PIN, INPUT_PULLUP);
  pinMode(S2_ENCODER_PIN, INPUT_PULLUP);
  pinMode(KEY_ENCODER_PIN, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
  ms = blink_ms = millis();

  kb.begin();
}

void loop()
{
  // Make sure built-in LED is off
  LED_state = LOW;
  digitalWrite(BUILTIN_LED, LED_state);

  e.updateValues();

  // Rotation part of the encoder
  switch (e.getRotationDirection())
  {
  case -1:
    // Serial.println("Left");
    for (int i = 0; i < KEY_SEND_MULTIPLIER; i++)
    {
      kb.write(KEY_MEDIA_VOLUME_DOWN);
    }
    break;
  case 1:
    // Serial.println("Right");
    for (int i = 0; i < KEY_SEND_MULTIPLIER; i++)
    {
      kb.write(KEY_MEDIA_VOLUME_UP);
    }
    break;
  default:
    break;
  }

  // Button part of the encoder
  if (!e.getButtonStates().first && e.getButtonStates().second)
  {
    ms = millis(); 
  }
  if (millis() - ms > BUTTON_HOLD_TIME && e.getButtonStates().second && !block_hold)
  {
    // Serial.println("MEDIA NEXT");
    kb.write(KEY_MEDIA_NEXT_TRACK);
    block_hold = true;
    delay(30); // Sometimes it also sends play/pause, I suppose this is because of bouncing, delay should solve that

  }
  if (e.getButtonStates().first && !e.getButtonStates().second && !block_hold)
  {
    // Serial.println("MEDIA PLAY/PAUSE");
    kb.write(KEY_MEDIA_PLAY_PAUSE);
    delay(30); // Sometimes the key is sent twice, I suppose this is because of bouncing, delay should solve that
  }
  if (e.getButtonStates().first && !e.getButtonStates().second)
  {
    block_hold = false;
  }

  if (!kb.isConnected())
  {
    // Serial.println("Disconnected!");
    digitalWrite(BUILTIN_LED, LED_state);
    if (millis() - blink_ms > 1000)
    {
      LED_state = !LED_state;
      blink_ms = millis();
    }
  }
}
