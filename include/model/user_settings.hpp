#ifndef __IM_MODEL_USER_SETTINGS_HPP__
#define __IM_MODEL_USER_SETTINGS_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

/**
 * @brief 用户设置实体类 (对应数据库 tb_user_settings 表)
 */
struct UserSettings {
    uint64_t user_id = 0;                     // 用户ID，外键关联users.id
    std::string theme_mode = "light";         // 主题模式：light/dark/auto
    std::string theme_bag_img = "";           // 主题背景图片URL
    std::string theme_color = "#409eff";      // 主题颜色，十六进制
    std::string notify_cue_tone = "N";        // 通知提示音: Y是 N否
    std::string keyboard_event_notify = "N";  // 键盘事件通知：Y是 N否
    std::time_t created_at = 0;               // 创建时间
    std::time_t updated_at = 0;               // 更新时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_USER_SETTINGS_HPP__
