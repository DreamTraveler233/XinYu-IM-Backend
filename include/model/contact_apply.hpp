#ifndef __IM_CONTACT_APPLY_HPP__
#define __IM_CONTACT_APPLY_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

struct ContactApply {
    uint64_t id = 0;                 // 主键ID
    uint64_t apply_user_id = 0;      // 申请人用户ID
    uint64_t target_user_id = 0;     // 被申请用户ID
    std::string remark = "";         // 申请附言（打招呼）
    uint8_t status = 1;              // 状态：1=待处理 2=已同意 3=已拒绝
    uint64_t handler_user_id = 0;    // 处理人用户ID（通常等于 target_user_id）
    std::string handle_remark = "";  // 处理备注（可选）
    std::time_t handled_at = 0;      // 处理时间（同意/拒绝）
    std::time_t created_at = 0;      // 申请时间
    std::time_t updated_at = 0;      // 更新时间
};

}  // namespace IM::model

#endif  // __IM_CONTACT_APPLY_HPP__