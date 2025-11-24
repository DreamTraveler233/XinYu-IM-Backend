#ifndef __IM_DOMAIN_SERVICE_COMMON_SERVICE_HPP__
#define __IM_DOMAIN_SERVICE_COMMON_SERVICE_HPP__

#include <string>

#include "common/result.hpp"
#include "http/http_session.hpp"
#include "model/email_verify_code.hpp"
#include "model/sms_verify_code.hpp"

namespace IM::domain::service {

class ICommonService {
   public:
    using Ptr = std::shared_ptr<ICommonService>;
    virtual ~ICommonService() = default;

    // 发送短信验证码
    virtual Result<model::SmsVerifyCode> SendSmsCode(const std::string& mobile,
                                                     const std::string& channel,
                                                     IM::http::HttpSession::ptr session) = 0;

    // 验证短信验证码
    virtual Result<void> VerifySmsCode(const std::string& mobile, const std::string& code,
                                       const std::string& channel) = 0;

    // 发送邮箱验证码
    virtual Result<model::EmailVerifyCode> SendEmailCode(const std::string& email,
                                                         const std::string& channel,
                                                         IM::http::HttpSession::ptr session) = 0;

    // 验证邮箱验证码
    virtual Result<void> VerifyEmailCode(const std::string& email, const std::string& code,
                                         const std::string& channel) = 0;

    // 初始化验证码清理定时器（幂等）
    virtual void InitCleanupTimer() = 0;

    // 初始化无效验证码删除定时器
    virtual void InitInvalidCodeCleanupTimer() = 0;

   private:
    // 实际发送短信
    virtual bool SendRealSms(const std::string& mobile, const std::string& sms_code,
                             const std::string& channel, std::string* err) = 0;

    // 实际发送邮件
    virtual bool SendRealEmail(const std::string& email, const std::string& title,
                               const std::string& body, std::string* err) = 0;
    // 阿里云短信发送
    virtual bool SendSmsViaAliyun(const std::string& mobile, const std::string& sms_code,
                                  const std::string& channel, std::string* err) = 0;

    // 腾讯云短信发送
    virtual bool SendSmsViaTencent(const std::string& mobile, const std::string& sms_code,
                                   const std::string& channel, std::string* err) = 0;
};

}  // namespace IM::domain::service

#endif  // __IM_DOMAIN_SERVICE_COMMON_SERVICE_HPP__