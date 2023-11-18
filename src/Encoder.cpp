#include <Arduino.h>
#include "Encoder.h"

#define S1_ENCODER_PIN 21
#define S2_ENCODER_PIN 22
#define KEY_ENCODER_PIN 5

Encoder::Encoder()
{
    pinMode(S1_ENCODER_PIN, INPUT_PULLUP);
    pinMode(S2_ENCODER_PIN, INPUT_PULLUP);
    pinMode(KEY_ENCODER_PIN, INPUT_PULLUP);
    __updateValues();
}

void Encoder::__updateValues()
{
    m_last_s1 = m_s1;
    m_last_s2 = m_s2;
    m_last_key = m_key;

    m_s1 = !digitalRead(S1_ENCODER_PIN);
    m_s2 = !digitalRead(S2_ENCODER_PIN);
    m_key = !digitalRead(KEY_ENCODER_PIN);
}

int16_t Encoder::getRotationDirection() // 1 for right; -1 for left; 0 for stationary
{
    __updateValues();
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

std::pair<bool, bool> Encoder::getButtonStates()
{
    return {m_last_key, m_key};
}

void Encoder::printValues()
{
    Serial.print("s1 = ");
    Serial.print(m_s1);
    Serial.print(" s2 = ");
    Serial.print(m_s2);
    Serial.print(" key = ");
    Serial.print(m_key);
    Serial.print("\n");
}

void Encoder::printLastValues()
{
    Serial.print("last s1 = ");
    Serial.print(m_last_s1);
    Serial.print(" last s2 = ");
    Serial.print(m_last_s2);
    Serial.print(" last key = ");
    Serial.print(m_last_key);
    Serial.print("\n");
}