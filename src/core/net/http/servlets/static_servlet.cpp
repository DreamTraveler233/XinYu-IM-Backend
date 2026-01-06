#include "core/net/http/servlets/static_servlet.hpp"
#include "core/log/logger.hpp"
#include <fstream>
#include <sstream>

namespace IM::http {

static auto g_logger = IM_LOG_NAME("system");

StaticServlet::StaticServlet(const std::string& path, const std::string& prefix)
    : Servlet("StaticServlet"), m_path(path), m_prefix(prefix) {
}

int32_t StaticServlet::handle(IM::http::HttpRequest::ptr request, IM::http::HttpResponse::ptr response,
                              IM::http::HttpSession::ptr session) {
    std::string path = request->getPath();
    
    // Strip prefix
    if (path.find(m_prefix) == 0) {
        path = path.substr(m_prefix.length());
    } else {
        // Should not happen if routed correctly, but just in case
        response->setStatus(HttpStatus::NOT_FOUND);
        return 0;
    }
    
    // Prevent directory traversal
    if (path.find("..") != std::string::npos) {
        response->setStatus(HttpStatus::FORBIDDEN);
        return 0;
    }

    // Construct full path
    // Ensure m_path does not end with / and path does not start with / (handled by substr logic usually)
    // But let's be safe.
    std::string full_path = m_path;
    if (!full_path.empty() && full_path.back() != '/') {
        full_path += "/";
    }
    // path from substr might be empty or "2023/..."
    full_path += path;

    IM_LOG_DEBUG(g_logger) << "StaticServlet serving: " << full_path;

    std::ifstream ifs(full_path, std::ios::binary);
    if (!ifs) {
        IM_LOG_WARN(g_logger) << "Static file not found: " << full_path;
        response->setStatus(HttpStatus::NOT_FOUND);
        return 0;
    }

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string body = ss.str();

    response->setBody(body);
    response->setStatus(HttpStatus::OK);

    // Set Content-Type
    std::string ext = "";
    size_t pos = full_path.rfind('.');
    if (pos != std::string::npos) {
        ext = full_path.substr(pos);
    }

    if (ext == ".jpg" || ext == ".jpeg") response->setHeader("Content-Type", "image/jpeg");
    else if (ext == ".png") response->setHeader("Content-Type", "image/png");
    else if (ext == ".gif") response->setHeader("Content-Type", "image/gif");
    else if (ext == ".bmp") response->setHeader("Content-Type", "image/bmp");
    else if (ext == ".webp") response->setHeader("Content-Type", "image/webp");
    else response->setHeader("Content-Type", "application/octet-stream");

    return 0;
}

}
