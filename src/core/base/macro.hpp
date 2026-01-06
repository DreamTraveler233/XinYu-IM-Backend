#ifndef __IM_BASE_MACRO_HPP__
#define __IM_BASE_MACRO_HPP__

#include "core/log/logger.hpp"
#include "core/log/logger_manager.hpp"
#include "core/util/util.hpp"
#include "core/io/thread.hpp"
#include <string.h>
#include <assert.h>

/**
 * @brief 分支预测优化宏定义
 * @details 这些宏用于帮助编译器进行分支预测优化，提高程序执行效率
 * 在支持的编译器(GCC/LLVM)上使用__builtin_expect内建函数进行优化，
 * 在不支持的编译器上退化为普通表达式
 */
#if defined __GNUC__ || defined __llvm__
#define IM_LIKELY(x) __builtin_expect(!!(x), 1)
#define IM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define IM_LIKELY(x) (x)
#define IM_UNLIKELY(x) (x)
#endif // __IM_BASE_MACRO_HPP__

#define IM_LOG(logger, level)                                         \
    if (level >= logger->getLevel())                                  \
    IM::LogEventWrap(                                                 \
        IM::LogEvent::ptr(                                            \
            new IM::LogEvent(logger, level, __FILE__, __LINE__, 0,    \
                             IM::GetThreadId(), IM::GetCoroutineId(), \
                             time(0), IM::Thread::GetName())))        \
        .getSS()

#define IM_LOG_DEBUG(logger) IM_LOG(logger, IM::Level::DEBUG)
#define IM_LOG_INFO(logger) IM_LOG(logger, IM::Level::INFO)
#define IM_LOG_WARN(logger) IM_LOG(logger, IM::Level::WARN)
#define IM_LOG_ERROR(logger) IM_LOG(logger, IM::Level::ERROR)
#define IM_LOG_FATAL(logger) IM_LOG(logger, IM::Level::FATAL)

#define IM_LOG_FMT(logger, level, fmt, ...)                           \
    if (level >= logger->getLevel())                                  \
    IM::LogEventWrap(                                                 \
        IM::LogEvent::ptr(                                            \
            new IM::LogEvent(logger, level, __FILE__, __LINE__, 0,    \
                             IM::GetThreadId(), IM::GetCoroutineId(), \
                             time(0), IM::Thread::GetName())))        \
        .getEvent()                                                   \
        ->format(fmt, __VA_ARGS__)

#define IM_LOG_FMT_DEBUG(logger, fmt, ...) IM_LOG_FMT(logger, IM::Level::DEBUG, fmt, __VA_ARGS__)
#define IM_LOG_FMT_INFO(logger, fmt, ...) IM_LOG_FMT(logger, IM::Level::INFO, fmt, __VA_ARGS__)
#define IM_LOG_FMT_WARN(logger, fmt, ...) IM_LOG_FMT(logger, IM::Level::WARN, fmt, __VA_ARGS__)
#define IM_LOG_FMT_ERROR(logger, fmt, ...) IM_LOG_FMT(logger, IM::Level::ERROR, fmt, __VA_ARGS__)
#define IM_LOG_FMT_FATAL(logger, fmt, ...) IM_LOG_FMT(logger, IM::Level::FATAL, fmt, __VA_ARGS__)

#define IM_LOG_ROOT() IM::LoggerMgr::GetInstance()->getRoot()
#define IM_LOG_NAME(name) IM::LoggerMgr::GetInstance()->getLogger(name)

#define IM_ASSERT(X)                                                          \
    if (IM_UNLIKELY(!(X)))                                                    \
    {                                                                         \
        IM_LOG_ERROR(IM_LOG_ROOT()) << "ASSERTION: " #X                       \
                                    << "\nbacktrace:\n"                       \
                                    << IM::BacktraceToString(100, 2, "    "); \
        assert(X);                                                            \
    }

#define IM_ASSERT2(X, W)                                                      \
    if (IM_UNLIKELY(!(X)))                                                    \
    {                                                                         \
        IM_LOG_ERROR(IM_LOG_ROOT()) << "ASSERTION: " #X                       \
                                    << "\n"                                   \
                                    << W                                      \
                                    << "\nbacktrace:\n"                       \
                                    << IM::BacktraceToString(100, 2, "    "); \
        assert(X);                                                            \
    }

#endif // __IM_BASE_MACRO_HPP__
