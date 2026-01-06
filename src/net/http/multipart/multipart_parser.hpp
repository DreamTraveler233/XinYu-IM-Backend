#ifndef __IM_NET_HTTP_MULTIPART_PARSER_HPP__
#define __IM_NET_HTTP_MULTIPART_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>

namespace IM {
namespace http {
namespace multipart {

struct Part {
    std::string name;
    std::string filename;
    std::string content_type;
    std::string data;       // small part in-memory
    std::string temp_file;  // large part saved to temp file
    size_t size = 0;
};

class MultipartParser {
   public:
    using ptr = std::shared_ptr<MultipartParser>;
    virtual ~MultipartParser() = default;

    // Parse using body string and content_type header (contains boundary)
    // On success return true and fill parts, otherwise false and optional err message
    virtual bool Parse(const std::string& body, const std::string& content_type,
                       const std::string& temp_dir, std::vector<Part>& parts,
                       std::string* err = nullptr) = 0;

    // Optional helper: Parse from request (not strictly required for this PR)
    // virtual bool ParseFromRequest(const IM::http::HttpRequest::ptr& req, std::vector<Part>& parts, std::string* err = nullptr) = 0;
};

// Factory helper to create a default implementation
// Implementation provided at infra layer
std::shared_ptr<MultipartParser> CreateMultipartParser();

}  // namespace multipart
}  // namespace http
}  // namespace IM

#endif  // __IM_NET_HTTP_MULTIPART_PARSER_HPP__
