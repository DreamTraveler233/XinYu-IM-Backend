#include "core/util/trace_context.hpp"

#include <random>
#include <iomanip>
#include <sstream>

namespace IM {

thread_local std::string TraceContext::s_trace_id = "";

std::string TraceContext::GetTraceId() {
    return s_trace_id;
}

void TraceContext::SetTraceId(const std::string& traceId) {
    s_trace_id = traceId;
}

void TraceContext::Clear() {
    s_trace_id = "";
}

std::string TraceContext::GenerateTraceId() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t id1 = dis(gen);
    uint64_t id2 = dis(gen);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << id1
       << std::setw(16) << std::setfill('0') << id2;
    return ss.str();
}

} // namespace IM
