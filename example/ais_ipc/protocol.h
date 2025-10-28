#ifndef AIS_PROTOCOL_H
#define AIS_PROTOCOL_H

#include <string>
#include <cstdint>

#include "json.hpp"

namespace ais
{

    /**
     * @brief AIS IPC通信协议
     *
     * 定义GUI进程和服务进程之间的通信协议格式和命令类型
     */
    namespace protocol
    {

        using json = nlohmann::json;

        // 命令类型枚举
        enum class CommandType : uint8_t
        {
            GET_STATUS = 0,        // 获取服务状态（获取服务端监听udp处理ais数据流程状态）
            START_SERVICE = 1,     // 启动服务（启动服务端监听udp处理ais数据流程）
            STOP_SERVICE = 2,      // 停止服务（停止服务端监听udp处理ais数据流程）
            GET_SHIP_COUNT = 3,    // 获取船舶数量（client -> server）
            SEND_MESSAGE = 4,      // 发送消息（server -> client）
            CONFIG_UPDATE = 5,     // 配置更新（client -> server）
            GET_MESSAGE_STATS = 6, // 获取服务端已处理消息统计（client -> server）
            HEARTBEAT = 7,         // 心跳检测
            CHANGE_SERVICE_LOGS = 8// 获取服务日志（启用将 server 日志也主动通过 SEND_MESSAGE 发到 client）
                                   // 当前主要是将 server 监听处理的 ais 数据的原始数据和处理结果转发一份到 client
        };

        // 响应状态枚举 - 避免使用ERROR，改用ERR
        enum class ResponseStatus : uint8_t
        {
            SUCCESS = 0,         // 成功
            ERR = 1,             // 错误 (避免使用ERROR，因为Windows头文件中有ERROR宏)
            INVALID_COMMAND = 2, // 无效命令
            SERVICE_BUSY = 3,    // 服务繁忙
            NOT_CONNECTED = 4    // 未连接
        };

        /**
         * @brief 命令消息结构体
         *
         * 用于客户端向服务端发送命令的消息格式
         */
        struct CommandMessage
        {
            CommandType type;  // 命令类型
            uint32_t sequence; // 序列号（用于匹配请求和响应）
            std::string data;  // JSON格式的命令数据

            /**
             * @brief 将命令消息转换为JSON字符串
             * @return JSON格式的字符串
             */
            std::string toJson() const;

            /**
             * @brief 从JSON字符串解析命令消息
             * @param jsonStr JSON格式的字符串
             * @return 解析后的CommandMessage对象
             * @throws std::runtime_error 解析失败时抛出异常
             */
            static CommandMessage fromJson(const std::string &jsonStr);
        };

        /**
         * @brief 响应消息结构体
         *
         * 用于服务端向客户端发送响应的消息格式
         */
        struct ResponseMessage
        {
            ResponseStatus status; // 响应状态
            uint32_t sequence;     // 对应命令的序列号
            std::string data;      // JSON格式的响应数据

            /**
             * @brief 将响应消息转换为JSON字符串
             * @return JSON格式的字符串
             */
            std::string toJson() const;

            /**
             * @brief 从JSON字符串解析响应消息
             * @param jsonStr JSON格式的字符串
             * @return 解析后的ResponseMessage对象
             * @throws std::runtime_error 解析失败时抛出异常
             */
            static ResponseMessage fromJson(const std::string &jsonStr);
        };

        /**
         * @brief 服务状态信息结构体
         *
         * 包含服务的运行状态信息
         */
        struct ServiceStatus
        {
            bool isRunning;             // 服务是否正在运行
            uint32_t shipCount;         // 当前跟踪的船舶数量
            uint64_t uptime;            // 服务运行时间（秒）
            uint64_t messagesReceived;  // 接收到的消息总数
            uint64_t messagesSent;      // 发送的消息总数
            uint64_t messagesProcessed; // 处理的消息总数
            uint64_t lastMessageTime;   // 最后一条消息的时间戳

            /**
             * @brief 将服务状态转换为JSON对象
             * @return nlohmann::json对象
             */
            nlohmann::json toJson() const;

            /**
             * @brief 从JSON对象解析服务状态
             * @param j JSON对象
             * @return 解析后的ServiceStatus对象
             */
            static ServiceStatus fromJson(const nlohmann::json &j);
        };

    } // namespace protocol
} // namespace ais

#endif // AIS_PROTOCOL_H