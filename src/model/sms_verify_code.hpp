#ifndef __IM_MODEL_SMS_VERIFY_CODE_HPP__
#define __IM_MODEL_SMS_VERIFY_CODE_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

struct SmsVerifyCode {
    uint64_t id = 0;             // 主键ID
    std::string mobile = "";     // 手机号
    std::string channel = "";    // 场景（register/login/forget/update_mobile等）
    std::string code = "";       // 验证码
    uint8_t status = 1;          // 状态（1=未使用 2=已使用 3=已过期）
    std::string sent_ip = "";    // 发送请求IP
    std::time_t sent_at = 0;     // 发送时间
    std::time_t expire_at = 0;   // 过期时间
    std::time_t used_at = 0;     // 使用时间
    std::time_t created_at = 0;  // 创建时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_SMS_VERIFY_CODE_HPP__