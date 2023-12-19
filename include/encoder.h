#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <bits/stl_pair.h>

class Encoder
{
private:
    // Pins
    const uint8_t m_s1_pin = 0, m_s2_pin = 0, m_key_pin = 0;
    
    // Values
    bool m_s1 = 0, m_s2 = 0, m_key = 0;
    bool m_last_s1 = 0, m_last_s2 = 0, m_last_key = 0;

    void m_updateValues();

public:
    Encoder(const uint8_t s1_pin, const uint8_t s2_pin, const uint8_t key_pin);
    Encoder() = delete;
    int8_t getRotationDirection(); // 1 for right; -1 for left; 0 for stationary
    std::pair<bool, bool> getButtonStates();
    void printValues();
    void printLastValues();
};

#endif