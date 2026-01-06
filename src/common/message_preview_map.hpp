#ifndef __IM_COMMON_MESSAGE_PREVIEW_MAP_HPP__
#define __IM_COMMON_MESSAGE_PREVIEW_MAP_HPP__

#include <map>
#include <string>

#include "common/message_type.hpp"

namespace IM::common {

// 后端 MessageType -> 会话预览字符串映射
static const std::map<MessageType, std::string> kMessagePreviewMap = {
    {MessageType::Text, "[文本消息]"},
    {MessageType::Code, "[代码消息]"},
    {MessageType::Image, "[图片消息]"},
    {MessageType::Audio, "[语音消息]"},
    {MessageType::Video, "[视频消息]"},
    {MessageType::File, "[文件消息]"},
    {MessageType::Location, "[位置消息]"},
    {MessageType::Card, "[名片消息]"},
    {MessageType::Forward, "[转发消息]"},
    {MessageType::Login, "[登录消息]"},
    {MessageType::Vote, "[投票消息]"},
    {MessageType::Mixed, "[图文消息]"},
    {MessageType::GroupNotice, "[群公告]"},
    {MessageType::SysText, "[系统消息]"},
    {MessageType::SysGroupCreate, "[创建群消息]"},
    {MessageType::SysGroupMemberJoin, "[加入群消息]"},
    {MessageType::SysGroupMemberQuit, "[退出群消息]"},
    {MessageType::SysGroupMemberKicked, "[踢出群消息]"},
    {MessageType::SysGroupMessageRevoke, "[撤回消息]"},
    {MessageType::SysGroupDismissed, "[群解散消息]"},
    {MessageType::SysGroupMuted, "[群禁言消息]"},
    {MessageType::SysGroupCancelMuted, "[群解除禁言消息]"},
    {MessageType::SysGroupMemberMuted, "[群成员禁言消息]"},
    {MessageType::SysGroupMemberCancelMuted, "[群成员解除禁言消息]"}
};

}  // namespace IM::common

#endif  // __IM_COMMON_MESSAGE_PREVIEW_MAP_HPP__
