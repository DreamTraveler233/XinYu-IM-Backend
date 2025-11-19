#ifndef __CIM_COMMON_MESSAGE_TYPE_HPP__
#define __CIM_COMMON_MESSAGE_TYPE_HPP__

#include <cstdint>

namespace CIM::common {

/**
 * MessageType - 后端使用的消息类型枚举
 * 注：数值与数据库 im_message.msg_type 字段、前端 msgTypeMap 保持一致
 */
enum class MessageType : uint16_t {
    Text = 1,
    Code = 2,
    Image = 3,
    Audio = 4,
    Video = 5,
    File = 6,
    Location = 7,
    Card = 8,
    Forward = 9,
    Login = 10,
    Vote = 11,
    Mixed = 12,
    GroupNotice = 13,
    // 系统消息起始 1000
    SysText = 1000,
    SysGroupCreate = 1101,
    SysGroupMemberJoin = 1102,
    SysGroupMemberQuit = 1103,
    SysGroupMemberKicked = 1104,
    SysGroupMessageRevoke = 1105,
    SysGroupDismissed = 1106,
    SysGroupMuted = 1107,
    SysGroupCancelMuted = 1108,
    SysGroupMemberMuted = 1109,
    SysGroupMemberCancelMuted = 1110,
    SysGroupTransfer = 1113
};

}  // namespace CIM::common

#endif  // __CIM_COMMON_MESSAGE_TYPE_HPP__
