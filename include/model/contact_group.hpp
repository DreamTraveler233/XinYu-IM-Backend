#ifndef __IM_MODEL_CONTACT_GROUP_HPP__
#define __IM_MODEL_CONTACT_GROUP_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

struct ContactGroup {
    uint64_t id = 0;             // 分组ID
    uint64_t user_id = 0;        // 所属用户ID
    std::string name = "";       // 分组名称
    uint32_t sort = 100;         // 排序值
    uint32_t contact_count = 0;  // 分组下联系人数量
    std::time_t created_at = 0;  // 创建时间
    std::time_t updated_at = 0;  // 更新时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_CONTACT_GROUP_HPP__