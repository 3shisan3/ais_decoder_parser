#include "utils/multipart_reassembler.h"

#include <algorithm>
#include <ctime>

namespace ais {

void MultipartReassembler::addFragment(const std::string& messageId, 
                                     const std::string& payload, 
                                     int fragmentNumber,
                                     int totalFragments) {
    MessageFragment fragment{payload, fragmentNumber, time(nullptr)};
    pendingMessages[messageId].push_back(fragment);
    
    // 清理过期消息
    cleanup();
}

bool MultipartReassembler::isComplete(const std::string& messageId, int totalFragments) const {
    auto it = pendingMessages.find(messageId);
    if (it == pendingMessages.end()) return false;
    
    const auto& fragments = it->second;
    if (fragments.size() != static_cast<size_t>(totalFragments)) {
        return false;
    }
    
    // 检查所有分片是否按顺序到达
    for (int i = 1; i <= totalFragments; i++) {
        bool found = false;
        for (const auto& frag : fragments) {
            if (frag.fragmentNumber == i) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    
    return true;
}

std::string MultipartReassembler::reassemble(const std::string& messageId, int totalFragments) {
    if (!isComplete(messageId, totalFragments)) {
        return "";
    }
    
    auto& fragments = pendingMessages[messageId];
    std::sort(fragments.begin(), fragments.end(), 
              [](const MessageFragment& a, const MessageFragment& b) {
                  return a.fragmentNumber < b.fragmentNumber;
              });
    
    std::string result;
    for (const auto& frag : fragments) {
        result += frag.payload;
    }
    
    pendingMessages.erase(messageId);
    return result;
}

void MultipartReassembler::cleanup() {
    time_t now = time(nullptr);
    for (auto it = pendingMessages.begin(); it != pendingMessages.end(); ) {
        auto& fragments = it->second;
        fragments.erase(
            std::remove_if(fragments.begin(), fragments.end(),
                [now, this](const MessageFragment& frag) {
                    return difftime(now, frag.timestamp) > maxAgeSeconds;
                }),
            fragments.end());
        
        if (fragments.empty()) {
            it = pendingMessages.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace ais