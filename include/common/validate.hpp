#ifndef __CIM_COMMON_VALIDATE_HPP__
#define __CIM_COMMON_VALIDATE_HPP__

#include <json/json.h>

#include <string>
#include <vector>

namespace CIM::common {

// 判断字符串是否为 32 字符十六进制 (hex)
inline bool isHex32(const std::string& s) {
    if (s.size() != 32) return false;
    for (char c : s) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            return false;
    }
    return true;
}

// 从 JSON 数组中解析 msg_ids：支持字符串数组 (32 hex) 或数字数组 (uint64)
// strict = true 时：只接受 string 且必须为 isHex32；返回 false 则表示解析失败
// strict = false 时：可接收 uint64 -> 转成 string
inline bool parseMsgIdsFromJson(const Json::Value& v, std::vector<std::string>& out,
                                bool strict = true) {
    out.clear();
    if (!v.isArray()) {
        return false;
    }

    for (auto& it : v) {
        if (it.isString()) {
            auto s = it.asString();
            if (strict && !isHex32(s)) {
                return false;
            }
            out.push_back(s);
        } else if (it.isUInt64()) {
            if (strict) {
                // 在严格模式下，不接受纯数字作为 ID
                return false;
            }
            out.push_back(std::to_string(it.asUInt64()));
        } else {
            // 不支持的类型
            return false;
        }
    }

    return true;
}

}  // namespace CIM::common

#endif  // __CIM_COMMON_VALIDATE_HPP__
