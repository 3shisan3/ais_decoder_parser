#include "core/bit_buffer.h"

#include <cmath>
#include <stdexcept>

namespace ais {

BitBuffer::BitBuffer(const std::string& payload) : payload(payload) {}

int BitBuffer::getInt(int start, int length) {
    int result = 0;
    for (int i = 0; i < length; i++) {
        int bytePos = (start + i) / 6;
        int bitPos = 5 - ((start + i) % 6);
        
        if (bytePos < payload.length()) {
            unsigned char ch = payload[bytePos] - 48;
            if (ch > 40) ch -= 8;
            
            if (ch & (1 << bitPos)) {
                result |= (1 << (length - 1 - i));
            }
        }
    }
    return result;
}

std::string BitBuffer::getString(int start, int length) {
    std::string result;
    int charCount = length / 6;
    
    for (int i = 0; i < charCount; i++) {
        int bits = getInt(start + i * 6, 6);
        if (bits == 0) break;
        
        char ch;
        if (bits < 32) {
            ch = bits + 64; // @, A-Z
        } else if (bits < 64) {
            ch = bits; // 其他字符
        } else {
            ch = ' '; // 无效字符
        }
        result += ch;
    }
    
    while (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    
    return result;
}

bool BitBuffer::getBool(int start) {
    return getInt(start, 1) != 0;
}

double BitBuffer::getLatitude(int start) {
    int value = getInt(start, 27);
    if (value == 0x3412140) return 91.0; // 默认值
    return value / 600000.0;
}

double BitBuffer::getLongitude(int start) {
    int value = getInt(start, 28);
    if (value == 0x6791AC0) return 181.0; // 默认值
    return value / 600000.0;
}

double BitBuffer::getSpeed(int start) {
    int sogRaw = getInt(start, 10);
    return sogRaw == 1023 ? 0 : sogRaw / 10.0;
}

double BitBuffer::getCourse(int start) {
    int cogRaw = getInt(start, 12);
    return cogRaw == 3600 ? 0 : cogRaw / 10.0;
}

int BitBuffer::getNavStatus(int start) {
    return getInt(start, 4);
}

int BitBuffer::getManeuverIndicator(int start) {
    return getInt(start, 2);
}

int BitBuffer::getRAIMFlag(int start) {
    return getBool(start) ? 1 : 0;
}

int BitBuffer::getCommunicationState(int start) {
    return getInt(start, 19);
}

} // namespace ais