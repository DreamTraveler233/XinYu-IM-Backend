#include "net/http/http_session.hpp"

#include "base/macro.hpp"
#include "net/http/http_parser.hpp"

namespace IM::http {
HttpSession::HttpSession(Socket::ptr sock, bool owner) : SocketStream(sock, owner) {}

HttpRequest::ptr HttpSession::recvRequest() {
    // 创建HTTP请求解析器实例，用于解析接收到的数据
    HttpRequestParser::ptr parser(new HttpRequestParser);
    // 获取HTTP请求缓冲区大小配置，用于控制每次读取数据的大小（防止恶意数据）
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    // uint64_t buff_size = 100;
    // 分配缓冲区内存，使用智能指针管理内存自动释放
    std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr) { delete[] ptr; });
    char* data = buffer.get();
    int offset = 0;  // 记录缓冲区中未处理数据的偏移量

    // 循环读取数据直到解析完成或出错
    do {
        int len = read(data + offset, buff_size - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }

        len += offset;
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            close();
            return nullptr;
        }

        offset = len - nparse;
        if (offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if (parser->isFinished()) {
            if (offset > 0) {
                m_leftoverBuf.append(data, offset);
            }
            break;
        }
    } while (true);

    int64_t length = parser->getContentLength();
    if (length > 0) {
        std::string body(length, '\0');
        size_t copied = 0;
        if (!m_leftoverBuf.empty()) {
            copied = std::min(static_cast<size_t>(length), m_leftoverBuf.size());
            memcpy(&body[0], m_leftoverBuf.data(), copied);
            m_leftoverBuf = m_leftoverBuf.substr(copied);
        }
        size_t remaining = static_cast<size_t>(length) - copied;
        if (remaining > 0) {
            if (readFixSize(&body[copied], remaining) <= 0) {
                close();
                return nullptr;
            }
        }
        // 设置解析得到的HTTP请求对象的请求体
        parser->getData()->setBody(body);
    }

    // 初始化HTTP请求对象（解析请求头中的信息）
    parser->getData()->init();
    // 返回解析得到的HTTP请求对象
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}

int HttpSession::read(void* buffer, size_t length) {
    if (!m_leftoverBuf.empty()) {
        size_t copy_len = std::min(length, m_leftoverBuf.size());
        memcpy(buffer, m_leftoverBuf.data(), copy_len);
        m_leftoverBuf = m_leftoverBuf.substr(copy_len);
        return copy_len;
    }
    return SocketStream::read(buffer, length);
}

int HttpSession::read(ByteArray::ptr ba, size_t length) {
    if (!m_leftoverBuf.empty()) {
        size_t copy_len = std::min(length, m_leftoverBuf.size());
        ba->write(m_leftoverBuf.data(), copy_len);
        m_leftoverBuf = m_leftoverBuf.substr(copy_len);
        return copy_len;
    }
    return SocketStream::read(ba, length);
}

}  // namespace IM::http