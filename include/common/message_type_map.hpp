#ifndef __CIM_COMMON_MESSAGE_TYPE_MAP_HPP__
#define __CIM_COMMON_MESSAGE_TYPE_MAP_HPP__

#include <cstdint>
#include <map>
#include <string>

#include "common/message_type.hpp"

namespace CIM::common {

// 前端的 msgTypeMap 字符串 -> 后端 msg_type 数值 映射
// NOTE: 一旦新增或修改，需要同时在前端 (CIM_F/src/store/modules/async-message.ts)
//       的 msgTypeMap 与这里的映射保持一致。
static const std::map<std::string, MessageType> kMessageTypeMap = {
    {"text", MessageType::Text},                // 文本消息
    {"code", MessageType::Code},                // 代码片段
    {"image", MessageType::Image},              // 图片
    {"audio", MessageType::Audio},              // 音频
    {"video", MessageType::Video},              // 视频
    {"file", MessageType::File},                // 文件
    {"location", MessageType::Location},        // 位置
    {"card", MessageType::Card},                // 名片
    {"forward", MessageType::Forward},          // 转发消息
    {"login", MessageType::Login},              // 登录通知
    {"vote", MessageType::Vote},                // 投票
    {"mixed", MessageType::Mixed},              // 混合消息
    {"group_notice", MessageType::GroupNotice}  // 群公告
};

}  // namespace CIM::common

#endif  // __CIM_COMMON_MESSAGE_TYPE_MAP_HPP__
