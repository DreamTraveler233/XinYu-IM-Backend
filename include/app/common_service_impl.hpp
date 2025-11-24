#ifndef __IM_APP_COMMON_SERVICE_IMPL_HPP__
#define __IM_APP_COMMON_SERVICE_IMPL_HPP__

#include "domain/repository/common_repository.hpp"
#include "domain/service/common_service.hpp"
namespace IM::app {

class CommonServiceImpl : public IM::domain::service::ICommonService {
   public:
    explicit CommonServiceImpl(IM::domain::repository::ICommonRepository::Ptr common_repo);

    // 发送短信验证码
    Result<model::SmsVerifyCode> SendSmsCode(const std::string& mobile, const std::string& channel,
                                             IM::http::HttpSession::ptr session) override;

    // 验证短信验证码
    Result<void> VerifySmsCode(const std::string& mobile, const std::string& code,
                               const std::string& channel) override;
    // 发送邮箱验证码
    Result<model::EmailVerifyCode> SendEmailCode(const std::string& email,
                                                 const std::string& channel,
                                                 IM::http::HttpSession::ptr session) override;
    // 验证邮箱验证码
    Result<void> VerifyEmailCode(const std::string& email, const std::string& code,
                                 const std::string& channel) override;

    // 初始化验证码清理定时器（幂等）
    void InitCleanupTimer() override;

    // 初始化无效验证码删除定时器
    void InitInvalidCodeCleanupTimer() override;

   private:
    // 实际发送短信
    bool SendRealSms(const std::string& mobile, const std::string& sms_code,
                     const std::string& channel, std::string* err) override;

    // 实际发送邮件
    bool SendRealEmail(const std::string& email, const std::string& title, const std::string& body,
                       std::string* err) override;
    // 阿里云短信发送
    bool SendSmsViaAliyun(const std::string& mobile, const std::string& sms_code,
                          const std::string& channel, std::string* err) override;

    // 腾讯云短信发送
    bool SendSmsViaTencent(const std::string& mobile, const std::string& sms_code,
                           const std::string& channel, std::string* err) override;

   private:
    IM::domain::repository::ICommonRepository::Ptr m_common_repo;
};

}  // namespace IM::app

#endif  // __IM_APP_COMMON_SERVICE_IMPL_HPP__