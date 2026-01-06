#ifndef __IM_DOMAIN_REPOSITORY_COMMON_REPOSITORY_HPP__
#define __IM_DOMAIN_REPOSITORY_COMMON_REPOSITORY_HPP__

#include <memory>

#include "model/email_verify_code.hpp"
#include "model/sms_verify_code.hpp"

namespace IM::domain::repository {

class ICommonRepository {
   public:
    using Ptr = std::shared_ptr<ICommonRepository>;
    virtual ~ICommonRepository() = default;

    // 创建新的邮箱验证码
    virtual bool CreateEmailCode(const model::EmailVerifyCode& code,
                                 std::string* err = nullptr) = 0;

    // 根据邮箱和验证码验证
    virtual bool VerifyEmailCode(const std::string& email, const std::string& code,
                                 const std::string& channel, std::string* err = nullptr) = 0;

    // 标记验证码为已使用
    virtual bool MarkEmailCodeAsUsed(const uint64_t id, std::string* err = nullptr) = 0;

    // 将过期验证码标记为失效
    virtual bool MarkEmailCodeExpiredAsInvalid(std::string* err = nullptr) = 0;

    // 删除失效验证码
    virtual bool DeleteInvalidEmailCode(std::string* err = nullptr) = 0;

    // 创建新的短信验证码
    virtual bool CreateSmsCode(const model::SmsVerifyCode& code, std::string* err = nullptr) = 0;

    // 根据手机号和验证码验证
    virtual bool VerifySmsCode(const std::string& mobile, const std::string& code,
                               const std::string& channel, std::string* err = nullptr) = 0;

    // 标记验证码为已使用
    virtual bool MarkSmsCodeAsUsed(const uint64_t id, std::string* err = nullptr) = 0;

    // 将过期验证码标记为失效
    virtual bool MarkSmsCodeExpiredAsInvalid(std::string* err = nullptr) = 0;

    // 删除失效验证码
    virtual bool DeleteInvalidSmsCode(std::string* err = nullptr) = 0;
};

}  // namespace IM::domain::repository

#endif  // __IM_DOMAIN_REPOSITORY_COMMON_REPOSITORY_HPP__