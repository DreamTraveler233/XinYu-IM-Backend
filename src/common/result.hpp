#ifndef __IM_COMMON_RESULT_HPP__
#define __IM_COMMON_RESULT_HPP__

#include <optional>
#include <string>

namespace IM {

// 通用错误码定义（可选）
enum class ErrorCode {
    OK = 0,
    INTERNAL_ERROR = 500,
    INVALID_PARAM = 400,
    NOT_FOUND = 404,
    // ...
};

template <typename T>
struct Result {
    bool ok = false;
    int code = 0;     // 错误码
    std::string err;  // 错误信息
    T data;           // 数据载荷

    // 默认构造：失败
    Result() : ok(false), code(500) {}

    // 成功构造
    static Result<T> Success(const T& val) {
        Result<T> r;
        r.ok = true;
        r.code = 0;
        r.data = val;
        return r;
    }

    // 失败构造
    static Result<T> Error(int c, const std::string& msg) {
        Result<T> r;
        r.ok = false;
        r.code = c;
        r.err = msg;
        return r;
    }
};

// 特化：无返回值的 Result
template <>
struct Result<void> {
    bool ok = false;
    int code = 0;
    std::string err;

    static Result<void> Success() {
        Result<void> r;
        r.ok = true;
        r.code = 0;
        return r;
    }

    static Result<void> Error(int c, const std::string& msg) {
        Result<void> r;
        r.ok = false;
        r.code = c;
        r.err = msg;
        return r;
    }
};

}  // namespace IM

#endif