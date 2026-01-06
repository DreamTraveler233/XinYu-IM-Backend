#ifndef __IM_NET_HTTP_SERVLETS_STATUS_SERVLET_HPP__
#define __IM_NET_HTTP_SERVLETS_STATUS_SERVLET_HPP__

#include "core/net/http/http_servlet.hpp"

namespace IM::http {

class StatusServlet : public Servlet {
   public:
    StatusServlet();
    virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                           HttpSession::ptr session) override;
};

}  // namespace IM::http

#endif // __IM_NET_HTTP_SERVLETS_STATUS_SERVLET_HPP__
