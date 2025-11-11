#ifndef __CIM_DAO_TALK_SESSION_HPP__
#define __CIM_DAO_TALK_SESSION_HPP__

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace CIM::dao {

struct TalkSessionItem {
    uint64_t id;             // 会话ID
    uint8_t talk_mode;       // 会话模式（1=单聊 2=群聊）
    uint64_t to_from_id;     // 目标用户ID
    uint8_t is_top;          // 是否置顶
    uint8_t is_disturb;      // 是否免打扰
    uint8_t is_robot;        // 是否机器人
    std::string name;        // 会话对象名称（用户名/群名称）
    std::string avatar;      // 会话对象头像URL
    std::string remark;      // 会话备注
    uint32_t unread_num;     // 未读消息数
    std::string msg_text;    // 最后一条消息预览文本
    std::time_t updated_at;  // 最后更新时间
};

class TalkSessionDao {
   public:
    // 根据用户ID获取会话列表
    static bool getSessionListByUserId(const uint64_t user_id, std::vector<TalkSessionItem>& out,
                                       std::string* err = nullptr);
};
}  // namespace CIM::dao
#endif  // __CIM_DAO_TALK_SESSION_HPP__