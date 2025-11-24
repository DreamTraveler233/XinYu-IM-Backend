#include "api/article_api_module.hpp"
#include "api/common_api_module.hpp"
#include "api/contact_api_module.hpp"
#include "api/emoticon_api_module.hpp"
#include "api/group_api_module.hpp"
#include "api/message_api_module.hpp"
#include "api/organize_api_module.hpp"
#include "api/talk_api_module.hpp"
#include "api/upload_api_module.hpp"
#include "api/user_api_module.hpp"
#include "api/ws_gateway_module.hpp"
#include "app/common_service_impl.hpp"
#include "app/contact_service_impl.hpp"
#include "app/media_service_impl.hpp"
#include "app/message_service_impl.hpp"
#include "app/talk_service_impl.hpp"
#include "app/user_service_impl.hpp"
#include "base/macro.hpp"
#include "db/mysql.hpp"
#include "http/http_server.hpp"
#include "infra/repository/common_repository_impl.hpp"
#include "infra/repository/contact_repository_impl.hpp"
#include "infra/repository/media_repository_impl.hpp"
#include "infra/repository/message_repository_impl.hpp"
#include "infra/repository/talk_repository_impl.hpp"
#include "infra/repository/user_repository_impl.hpp"
#include "other/crypto_module.hpp"
#include "other/module.hpp"
#include "system/application.hpp"

int main(int argc, char** argv) {
    /* 创建并初始化应用程序实例 */
    IM::Application app;
    if (!app.init(argc, argv)) {
        IM_LOG_ERROR(IM_LOG_ROOT()) << "Application init failed";
        return 1;
    }

    std::srand(std::time(nullptr));  // 初始化随机数种子

    // 注册加解密模块
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::CryptoModule>());

    /* 注册最小化 API 模块 */
    // 文章模块占位
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::ArticleApiModule>());
    // 表情模块占位
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::EmoticonApiModule>());
    // 群组模块占位
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::GroupApiModule>());
    // 组织架构模块占位
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::OrganizeApiModule>());

    // MySQL manager is a Singleton that returns a raw pointer; several repository
    // constructors expect a shared_ptr to manager, so wrap it with a no-op deleter
    // to avoid double-free (singleton ownership remains external).
    auto db_manager =
        std::shared_ptr<IM::MySQLManager>(IM::MySQLMgr::GetInstance(), [](IM::MySQLManager*) {});

    // Repositories
    auto user_repo = std::make_shared<IM::infra::repository::UserRepositoryImpl>(db_manager);
    auto contact_repo = std::make_shared<IM::infra::repository::ContactRepositoryImpl>(db_manager);
    auto common_repo = std::make_shared<IM::infra::repository::CommonRepositoryImpl>(db_manager);
    auto upload_repo = std::make_shared<IM::infra::repository::MediaRepositoryImpl>(db_manager);
    auto message_repo = std::make_shared<IM::infra::repository::MessageRepositoryImpl>(db_manager);
    auto talk_repo = std::make_shared<IM::infra::repository::TalkRepositoryImpl>(db_manager);

    // Services
    auto media_service = std::make_shared<IM::app::MediaServiceImpl>(upload_repo);
    auto common_service = std::make_shared<IM::app::CommonServiceImpl>(common_repo);
    auto user_service =
        std::make_shared<IM::app::UserServiceImpl>(user_repo, media_service, common_service);
    auto message_service = std::make_shared<IM::app::MessageServiceImpl>(message_repo, talk_repo,
                                                                         user_repo, contact_repo);
    auto talk_service =
        std::make_shared<IM::app::TalkServiceImpl>(talk_repo, contact_repo, message_repo);
    auto contact_service = std::make_shared<IM::app::ContactServiceImpl>(
        contact_repo, user_repo, talk_repo, message_service, talk_service);

    // Register API modules in the order that satisfies dependencies
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::UserApiModule>(user_service));
    IM::ModuleMgr::GetInstance()->add(
        std::make_shared<IM::api::ContactApiModule>(contact_service, user_service));
    IM::ModuleMgr::GetInstance()->add(
        std::make_shared<IM::api::CommonApiModule>(common_service, user_service));
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::UploadApiModule>(media_service));
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::MessageApiModule>(message_service));
    IM::ModuleMgr::GetInstance()->add(
        std::make_shared<IM::api::TalkApiModule>(talk_service, user_service, message_service));

    // WebSocket 网关模块
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::api::WsGatewayModule>(user_service));

    return app.run() ? 0 : 2;
}
