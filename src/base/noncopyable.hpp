#ifndef __IM_BASE_NONCOPYABLE_HPP__
#define __IM_BASE_NONCOPYABLE_HPP__

namespace IM {
class Noncopyable {
   public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;

   protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};
}  // namespace IM

#endif // __IM_BASE_NONCOPYABLE_HPP__