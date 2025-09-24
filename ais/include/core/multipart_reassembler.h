#ifndef AIS_MULTIPART_REASSEMBLER_H
#define AIS_MULTIPART_REASSEMBLER_H

#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>

namespace ais
{

/**
 * @brief 多部分消息重组器类
 * 
 * 用于重组分多部分传输的AIS消息
 */
class MultipartReassembler
{
private:
    /**
     * @brief 消息分片结构体
     */
    struct MessageFragment
    {
        std::string payload; // 分片负载
        int fragmentNumber;  // 分片号
        time_t timestamp;    // 接收时间戳
    };

    std::unordered_map<std::string, std::vector<MessageFragment>> pendingMessages; // 待重组消息
    int maxAgeSeconds;                                                             // 最大保留时间(秒)

public:
    /**
     * @brief 构造函数
     * @param maxAge 最大保留时间(秒)
     */
    explicit MultipartReassembler(int maxAge = 300) : maxAgeSeconds(maxAge) {}

    /**
     * @brief 添加消息分片
     * @param messageId 消息ID
     * @param payload 分片负载
     * @param fragmentNumber 分片号
     * @param totalFragments 总分片数
     */
    void addFragment(const std::string &messageId, const std::string &payload,
                     int fragmentNumber, int totalFragments);

    /**
     * @brief 检查消息是否完整
     * @param messageId 消息ID
     * @param totalFragments 总分片数
     * @return 是否完整
     */
    bool isComplete(const std::string &messageId, int totalFragments) const;

    /**
     * @brief 重组消息
     * @param messageId 消息ID
     * @param totalFragments 总分片数
     * @return 重组后的完整负载
     */
    std::string reassemble(const std::string &messageId, int totalFragments);

    /**
     * @brief 清理过期消息
     */
    void cleanup();
};

} // namespace ais

#endif // AIS_MULTIPART_REASSEMBLER_H