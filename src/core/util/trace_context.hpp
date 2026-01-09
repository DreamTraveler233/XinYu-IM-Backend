#ifndef __IM_UTIL_TRACE_CONTEXT_HPP__
#define __IM_UTIL_TRACE_CONTEXT_HPP__

#include <string>
#include <random>
#include <iomanip>
#include <sstream>

namespace IM {

class TraceContext {
public:
    static std::string GetTraceId();
    static void SetTraceId(const std::string& traceId);
    static void Clear();
    static std::string GenerateTraceId();

private:
    static thread_local std::string s_trace_id;
};

class TraceGuard {
public:
    TraceGuard(const std::string& traceId) {
        TraceContext::SetTraceId(traceId);
    }
    ~TraceGuard() {
        TraceContext::Clear();
    }
};

} // namespace IM

#endif // __IM_UTIL_TRACE_CONTEXT_HPP__
