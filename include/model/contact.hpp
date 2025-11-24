#ifndef __IM_MODEL_CONTACT_HPP__
#define __IM_MODEL_CONTACT_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

struct Contact {
    uint64_t id = 0;              // 联系人ID
    uint64_t owner_user_id = 0;   // 联系人拥有者（发起展示视角的用户）
    uint64_t friend_user_id = 0;  // 好友用户ID
    uint64_t group_id = 0;        // 联系人分组ID（im_contact_group.id）
    std::string remark = "";      // 我对该联系人的备注名
    uint8_t status = 1;           // 状态，1正常，2删除
    uint8_t relation = 1;         // 我视角的关系，1=陌生人，2=好友，3=企业同事，4=本人
    uint8_t is_block = 0;         // 是否拉黑，0否，1是
    std::time_t created_at = 0;   // 创建时间（成为好友时间）
    std::time_t updated_at = 0;   // 更新时间
    std::time_t deleted_at = 0;   // 软删除时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_CONTACT_HPP__