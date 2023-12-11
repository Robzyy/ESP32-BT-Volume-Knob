#include <Arduino.h>
#include "encoder.h"

Encoder::Encoder(const uint8_t s1_pin, const uint8_t s2_pin, const uint8_t key_pin)
    : m_s1_pin{s1_pin}, m_s2_pin{s2_pin}, m_key_pin{key_pin}
{
    pinMode(m_s1_pin, INPUT_PULLUP);
    pinMode(m_s2_pin, INPUT_PULLUP);
    pinMode(m_key_pin, INPUT_PULLUP);
    m_updateValues();
}

void Encoder::m_updateValues()
{
    m_last_s1 = m_s1;
    m_last_s2 = m_s2;
    m_last_key = m_key;

    m_s1 = !digitalRead(m_s1_pin);
    m_s2 = !digitalRead(m_s2_pin);
    m_key = !digitalRead(m_key_pin);
}

int8_t Encoder::getRotationDirection() // 1 for right; -1 for left; 0 for stationary
{
    m_updateValues();
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