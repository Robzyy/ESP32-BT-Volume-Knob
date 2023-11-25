#include <cstdint>
#include <bits/stl_pair.h>

class Encoder
{
private:
    bool m_s1 = 0, m_s2 = 0, m_key = 0;
    bool m_last_s1 = 0, m_last_s2 = 0, m_last_key = 0;

    void __updateValues();

public:
    Encoder();
    int16_t getRotationDirection(); // 1 for right; -1 for left; 0 for stationary
    std::pair<bool, bool> getButtonStates();
    void printValues();
    void printLastValues();
};