#ifndef __IM_NET_HTTP_SERVLETS_STATIC_SERVLET_HPP__
#define __IM_NET_HTTP_SERVLETS_STATIC_SERVLET_HPP__

#include "core/net/http/http_servlet.hpp"

namespace IM::http {

class StaticServlet : public Servlet {
public:
    typedef std::shared_ptr<StaticServlet> ptr;
    /**
     * @brief 构造函数
     * @param[in] path 映射的本地基路径
     * @param[in] prefix URL前缀，用于从请求路径中剥离
     */
    StaticServlet(const std::string& path, const std::string& prefix = "/media/");
    virtual int32_t handle(IM::http::HttpRequest::ptr request, IM::http::HttpResponse::ptr response,
                           IM::http::HttpSession::ptr session) override;

private:
    std::string m_path;
    std::string m_prefix;
};

}  // namespace IM::http

#endif
