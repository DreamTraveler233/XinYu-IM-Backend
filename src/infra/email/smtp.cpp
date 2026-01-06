#include "infra/email/smtp.hpp"

#include "core/base/macro.hpp"
#include <cctype>

namespace IM {
static Logger::ptr g_logger = IM_LOG_NAME("system");

SmtpClient::SmtpClient(Socket::ptr sock) : SocketStream(sock) {}

SmtpClient::ptr SmtpClient::Create(const std::string& host, uint32_t port, bool ssl) {
    IPAddress::ptr addr = Address::LookupAnyIpAddress(host);
    if (!addr) {
        IM_LOG_ERROR(g_logger) << "invalid smtp server: " << host << ":" << port << " ssl=" << ssl;
        return nullptr;
    }
    addr->setPort(port);
    Socket::ptr sock;
    if (ssl) {
        sock = SSLSocket::CreateTCP(addr);
    } else {
        sock = Socket::CreateTCP(addr);
    }
    if (!sock->connect(addr)) {
        IM_LOG_ERROR(g_logger) << "connect smtp server: " << host << ":" << port << " ssl=" << ssl
                               << " fail";
        return nullptr;
    }
    std::string buf;
    buf.resize(1024);

    SmtpClient::ptr rt(new SmtpClient(sock));
    int len = rt->read(&buf[0], buf.size());
    if (len <= 0) {
        return nullptr;
    }
    buf.resize(len);
    if (TypeUtil::Atoi(buf) != 220) {
        return nullptr;
    }
    rt->m_host = host;
    return rt;
}

SmtpResult::ptr SmtpClient::doCmd(const std::string& cmd, bool debug) {
    if (writeFixSize(cmd.c_str(), cmd.size()) <= 0) {
        return std::make_shared<SmtpResult>(SmtpResult::IO_ERROR, "write io error");
    }
    std::string buf;
    buf.resize(4096);
    auto len = read(&buf[0], buf.size());
    if (len <= 0) {
        return std::make_shared<SmtpResult>(SmtpResult::IO_ERROR, "read io error");
    }
    buf.resize(len);

    // Handle multi-line response (e.g., 250-...)
    if (buf.size() >= 4 && buf[3] == '-') {
        std::string code = buf.substr(0, 3);
        while (true) {
            // Check if the last line (code + " ") is present
            bool found_end = false;
            size_t pos = 0;
            while (pos < buf.size()) {
                size_t next_nl = buf.find("\r\n", pos);
                if (next_nl == std::string::npos) break;  // Incomplete line at end

                if (next_nl - pos >= 4) {
                    if (buf.substr(pos, 4) == code + " ") {
                        found_end = true;
                        break;
                    }
                }
                pos = next_nl + 2;
            }
            if (found_end) break;

            std::string tmp;
            tmp.resize(4096);
            int l = read(&tmp[0], tmp.size());
            if (l <= 0) {
                return std::make_shared<SmtpResult>(SmtpResult::IO_ERROR, "read io error");
            }
            tmp.resize(l);
            buf += tmp;
        }
    }

    if (debug) {
        m_ss << "C: " << cmd;
        m_ss << "S: " << buf;
    }

    int code = TypeUtil::Atoi(buf);
    if (code >= 400) {
        return std::make_shared<SmtpResult>(code,
                                            replace(buf.substr(buf.find(' ') + 1), "\r\n", ""));
    }
    return nullptr;
}

SmtpResult::ptr SmtpClient::send(EMail::ptr email, int64_t timeout_ms, bool debug) {
#define DO_CMD()                \
    result = doCmd(cmd, debug); \
    if (result) {               \
        return result;          \
    }

    Socket::ptr sock = getSocket();
    if (sock) {
        sock->setRecvTimeout(timeout_ms);
        sock->setSendTimeout(timeout_ms);
    }

    SmtpResult::ptr result;
    std::string cmd = "EHLO " + m_host + "\r\n";
    DO_CMD();
    if (!m_authed) {
        cmd = "AUTH LOGIN\r\n";
        DO_CMD();
        std::string auth_user;
        // prefer explicit auth user if provided
        if (!email->getAuthUser().empty()) {
            auth_user = email->getAuthUser();
        } else {
            // Extract pure address if From contains a display name like: "Name <addr@domain>"
            std::string from_raw = email->getFromEMailAddress();
            auth_user = from_raw;
            size_t lt = from_raw.find('<');
            size_t gt = std::string::npos;
            if (lt != std::string::npos) {
                gt = from_raw.find('>', lt);
            }
            if (lt != std::string::npos && gt != std::string::npos && gt > lt + 1) {
                std::string addr = from_raw.substr(lt + 1, gt - lt - 1);
                // Trim spaces (basic)
                size_t a = 0;
                while (a < addr.size() && isspace((unsigned char)addr[a])) a++;
                size_t b = addr.size();
                while (b > a && isspace((unsigned char)addr[b - 1])) b--;
                auth_user = addr.substr(a, b - a);
            }
        }
        cmd = base64encode(auth_user) + "\r\n";
        DO_CMD();
        cmd = base64encode(email->getFromEMailPasswd()) + "\r\n";
        DO_CMD();

        m_authed = true;
    }

    // Use pure email address for MAIL FROM (strip display name if present)
    std::string mail_from = email->getFromEMailAddress();
    size_t lpos = mail_from.find('<');
    size_t rpos = std::string::npos;
    if (lpos != std::string::npos) {
        rpos = mail_from.find('>', lpos);
    }
    if (lpos != std::string::npos && rpos != std::string::npos && rpos > lpos + 1) {
        // Extract addr inside <> and trim
        std::string addr = mail_from.substr(lpos + 1, rpos - lpos - 1);
        size_t a = 0;
        while (a < addr.size() && isspace((unsigned char)addr[a])) a++;
        size_t b = addr.size();
        while (b > a && isspace((unsigned char)addr[b - 1])) b--;
        mail_from = addr.substr(a, b - a);
    }
    cmd = "MAIL FROM: <" + mail_from + ">\r\n";
    DO_CMD();
    std::set<std::string> targets;
#define XX(fun)                    \
    for (auto& i : email->fun()) { \
        targets.insert(i);         \
    }
    XX(getToEMailAddress);
    XX(getCcEMailAddress);
    XX(getBccEMailAddress);
#undef XX
    for (auto& i : targets) {
        cmd = "RCPT TO: <" + i + ">\r\n";
        DO_CMD();
    }

    cmd = "DATA\r\n";
    DO_CMD();

    auto& entitys = email->getEntitys();

    std::stringstream ss;
    // Build From header: support optional display name in the stored from string
    std::string header_from = email->getFromEMailAddress();
    // If header_from contains angle brackets, assume it is in format "Name <addr>" and leave as-is
    // Otherwise, format as <addr>
    if (header_from.find('<') == std::string::npos || header_from.find('>') == std::string::npos) {
        header_from = "<" + header_from + ">";
    }
    ss << "From: " << header_from << "\r\n"
       << "To: ";
#define XX(fun)                                 \
    do {                                        \
        auto& v = email->fun();                 \
        for (size_t i = 0; i < v.size(); ++i) { \
            if (i) {                            \
                ss << ",";                      \
            }                                   \
            ss << "<" << v[i] << ">";           \
        }                                       \
        if (!v.empty()) {                       \
            ss << "\r\n";                       \
        }                                       \
    } while (0);
    XX(getToEMailAddress);
    if (!email->getCcEMailAddress().empty()) {
        ss << "Cc: ";
        XX(getCcEMailAddress);
    }
    ss << "Subject: " << email->getTitle() << "\r\n";
    std::string boundary;
    if (!entitys.empty()) {
        boundary = random_string(16);
        ss << "Content-Type: multipart/mixed;boundary=" << boundary << "\r\n";
    }
    ss << "MIME-Version: 1.0\r\n";
    if (!boundary.empty()) {
        ss << "\r\n--" << boundary << "\r\n";
    }
    ss << "Content-Type: text/html;charset=\"utf-8\"\r\n"
       << "\r\n"
       << email->getBody() << "\r\n";
    for (auto& i : entitys) {
        ss << "\r\n--" << boundary << "\r\n";
        ss << i->toString();
    }
    if (!boundary.empty()) {
        ss << "\r\n--" << boundary << "--\r\n";
    }
    // ss << "\r\n.\r\n";

    // Dot stuffing
    std::string content = ss.str();
    std::string stuffed_content;
    stuffed_content.reserve(content.size() + 1024);

    size_t pos = 0;
    while (pos < content.size()) {
        size_t next = content.find("\r\n", pos);
        std::string line;
        if (next == std::string::npos) {
            line = content.substr(pos);
            pos = content.size();
        } else {
            line = content.substr(pos, next - pos);
            pos = next + 2;
        }

        if (!line.empty() && line[0] == '.') {
            stuffed_content += ".";
        }
        stuffed_content += line;
        if (next != std::string::npos) {
            stuffed_content += "\r\n";
        }
    }

    cmd = stuffed_content + "\r\n.\r\n";
    DO_CMD();
#undef XX
#undef DO_CMD
    return std::make_shared<SmtpResult>(0, "ok");
}

std::string SmtpClient::getDebugInfo() {
    return m_ss.str();
}

}  // namespace IM