#ifndef __IM_INFRA_STORAGE_ISTORAGE_HPP__
#define __IM_INFRA_STORAGE_ISTORAGE_HPP__

#include <memory>
#include <string>
#include <vector>

namespace IM {
namespace infra {
namespace storage {

class IStorageAdapter {
public:
    using Ptr = std::shared_ptr<IStorageAdapter>;
    virtual ~IStorageAdapter() = default;

    // Move or copy a temporary file (e.g., parser tmp) to a destination (final part path)
    virtual bool MovePartFile(const std::string& src, const std::string& dest, std::string* err = nullptr) = 0;

    // Merge part files into final destination
    virtual bool MergeParts(const std::vector<std::string>& parts, const std::string& dest, std::string* err = nullptr) = 0;

    // Map storage path to URL (for web access)
    virtual std::string GetUrl(const std::string& storage_path) = 0;
};

// Factory for builtin local storage
std::shared_ptr<IStorageAdapter> CreateLocalStorageAdapter();

} // namespace storage
} // namespace infra
} // namespace IM

#endif // __IM_INFRA_STORAGE_ISTORAGE_HPP__
