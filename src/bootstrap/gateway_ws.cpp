#include "interface/api/ws_gateway_module.hpp"
#include "application/app/user_service_impl.hpp"
#include "application/app/media_service_impl.hpp"
#include "application/app/common_service_impl.hpp"
#include "application/app/talk_service_impl.hpp"
#include "application/app/message_service_impl.hpp"
#include "application/app/contact_service_impl.hpp"
#include "application/app/group_service_impl.hpp"
#include "core/base/macro.hpp"
#include "infra/db/mysql.hpp"
#include "infra/repository/user_repository_impl.hpp"
#include "infra/repository/talk_repository_impl.hpp"
#include "infra/repository/media_repository_impl.hpp"
#include "infra/repository/common_repository_impl.hpp"
#include "infra/repository/message_repository_impl.hpp"
#include "infra/repository/contact_repository_impl.hpp"
#include "infra/repository/group_repository_impl.hpp"
#include "infra/storage/istorage.hpp"
#include "infra/module/crypto_module.hpp"
#include "infra/module/module.hpp"
#include "core/system/application.hpp"

/**
 * @brief WebSocket 网关进程入口
 * 
 * 责任：
 * 1. 维护客户端 WebSocket 长连接
 * 2. 处理心跳、鉴权握手
 * 3. 接收下行推送并转发给客户端
 */
int main(int argc, char** argv) {
    /* 创建并初始化应用程序实例 */
    IM::Application app;
    if (!app.init(argc, argv)) {
        IM_LOG_ERROR(IM_LOG_ROOT()) << "Gateway WS init failed";
        return 1;
    }

    std::srand(std::time(nullptr));

    // 注册加解密模块
    IM::ModuleMgr::GetInstance()->add(std::make_shared<IM::CryptoModule>());

    // --- 临时链路：在彻底拆分服务前，网关仍暂持有一些逻辑依赖 ---
    // 未来这里将替换为 RPC Client
    auto db_manager =
        std::shared_ptr<IM::MySQLManager>(IM::MySQLMgr::GetInstance(), [](IM::MySQLManager*) {});
    
    // Repositories
    auto user_repo = std::make_shared<IM::infra::repository::UserRepositoryImpl>(db_manager);
    auto talk_repo = std::make_shared<IM::infra::repository::TalkRepositoryImpl>(db_manager);
    auto media_repo = std::make_shared<IM::infra::repository::MediaRepositoryImpl>(db_manager);
    auto common_repo = std::make_shared<IM::infra::repository::CommonRepositoryImpl>(db_manager);
    auto message_repo = std::make_shared<IM::infra::repository::MessageRepositoryImpl>(db_manager);
    auto contact_repo = std::make_shared<IM::infra::repository::ContactRepositoryImpl>(db_manager);
    auto group_repo = std::make_shared<IM::infra::repository::GroupRepositoryImpl>();

    // Services
    auto storage_adapter = IM::infra::storage::CreateLocalStorageAdapter();
    auto media_service = std::make_shared<IM::app::MediaServiceImpl>(media_repo, storage_adapter);
    auto common_service = std::make_shared<IM::app::CommonServiceImpl>(common_repo);
    auto user_service = std::make_shared<IM::app::UserServiceImpl>(user_repo, media_service,
                                                                   common_service, talk_repo);

    // WebSocket 网关模块 (核心)
    IM::ModuleMgr::GetInstance()->add(
        std::make_shared<IM::api::WsGatewayModule>(user_service, talk_repo));

    IM_LOG_INFO(IM_LOG_ROOT()) << "Gateway WS is starting...";
    return app.run() ? 0 : 2;
}
