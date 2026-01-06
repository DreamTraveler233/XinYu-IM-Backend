#ifndef __IM_DTO_CONTACT_DTO_HPP__
#define __IM_DTO_CONTACT_DTO_HPP__

#include <cstdint>
#include <string>

namespace IM::dto {

struct ContactApplyItem {
    uint64_t id = 0;              // 申请记录ID
    uint64_t apply_user_id = 0;   // 申请人用户ID
    uint64_t target_user_id = 0;  // 被申请用户ID
    std::string remark = "";      // 申请备注
    std::string nickname = "";    // 好友昵称
    std::string avatar = "";      // 好友头像
    std::string created_at = "";  // 申请时间
};

struct ContactDetails {
    uint64_t user_id = 0;           // 用户ID
    std::string avatar;             // 头像
    uint8_t gender = 0;             // 性别
    std::string mobile;             // 手机号
    std::string motto;              // 个性签名
    std::string nickname;           // 昵称
    std::string email;              // 邮箱
    uint8_t relation = 1;           // 关系
    uint32_t contact_group_id = 0;  // 分组ID
    std::string contact_remark;     // 备注
};

struct ContactItem {
    uint64_t user_id = 0;   // 用户ID
    std::string nickname;   // 昵称
    uint8_t gender = 0;     // 性别
    std::string motto;      // 个性签名
    std::string avatar;     // 头像
    std::string remark;     // 备注
    uint64_t group_id = 0;  // 分组ID
};

struct ContactGroupItem {
    uint64_t id = 0;             // 分组ID
    std::string name = "";       // 分组名称
    uint32_t contact_count = 0;  // 分组下联系人数量
    uint32_t sort = 0;           // 排序值
};

}  // namespace IM::dto

#endif  // __IM_DTO_CONTACT_DTO_HPP__