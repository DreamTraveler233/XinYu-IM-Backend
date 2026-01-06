#ifndef __IM_NET_ROCK_ROCK_SERVER_HPP__
#define __IM_NET_ROCK_ROCK_SERVER_HPP__

#include "rock_stream.hpp"
#include "core/net/core/tcp_server.hpp"

namespace IM {
class RockServer : public TcpServer {
   public:
    typedef std::shared_ptr<RockServer> ptr;
    RockServer(const std::string& type = "rock", IOManager* worker = IOManager::GetThis(),
               IOManager* io_worker = IOManager::GetThis(),
               IOManager* accept_worker = IOManager::GetThis());

   protected:
    virtual void handleClient(Socket::ptr client) override;
};

}  // namespace IM

#endif // __IM_NET_ROCK_ROCK_SERVER_HPP__
