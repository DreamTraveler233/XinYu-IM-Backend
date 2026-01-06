#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "core/base/macro.hpp"
#include "common/common.hpp"
#include "core/config/config.hpp"
#include "infra/storage/istorage.hpp"
#include "core/system/env.hpp"
#include "core/util/util.hpp"

namespace IM {
namespace infra {
namespace storage {

class LocalStorageAdapter : public IStorageAdapter {
   public:
    LocalStorageAdapter() = default;
    ~LocalStorageAdapter() override = default;

    bool MovePartFile(const std::string& src, const std::string& dest,
                      std::string* err = nullptr) override {
        // Ensure dest dir exists
        auto dir = IM::FSUtil::Dirname(dest);
        if (!IM::FSUtil::Mkdir(dir)) {
            if (err) *err = "create dest directory failed";
            return false;
        }
        // Try rename for performance; if fails, fallback to copy
        if (rename(src.c_str(), dest.c_str()) == 0) {
            return true;
        }
        // copy
        std::ifstream ifs(src, std::ios::binary);
        if (!ifs) {
            if (err) *err = "open src file failed";
            return false;
        }
        std::ofstream ofs(dest, std::ios::binary | std::ios::trunc);
        if (!ofs) {
            if (err) *err = "open dest file failed";
            return false;
        }
        ofs << ifs.rdbuf();
        ofs.close();
        ifs.close();
        // remove src
        IM::FSUtil::Unlink(src);
        return true;
    }

    bool MergeParts(const std::vector<std::string>& parts, const std::string& dest,
                    std::string* err = nullptr) override {
        auto dir = IM::FSUtil::Dirname(dest);
        if (!IM::FSUtil::Mkdir(dir)) {
            if (err) *err = "create dest directory failed";
            return false;
        }
        std::ofstream ofs(dest, std::ios::binary | std::ios::trunc);
        if (!ofs) {
            if (err) *err = "open dest file failed";
            return false;
        }
        for (auto& p : parts) {
            std::ifstream ifs(p, std::ios::binary);
            if (!ifs) {
                if (err) *err = "open part file failed";
                return false;
            }
            ofs << ifs.rdbuf();
            ifs.close();
        }
        ofs.close();
        return true;
    }

    std::string GetUrl(const std::string& storage_path) override {
        // Simple local mapping: replace upload base dir with '/media'
        std::string resolved = storage_path;
        std::string base = IM::EnvMgr::GetInstance()->getAbsoluteWorkPath(
            IM::Config::Lookup<std::string>("media.upload_base_dir", std::string("data/uploads"))
                ->getValue());
        if (!base.empty() && resolved.find(base) == 0) {
            resolved.replace(0, base.length(), "/media");
        }
        return resolved;
    }
};

// Factory
std::shared_ptr<IStorageAdapter> CreateLocalStorageAdapter() {
    return std::make_shared<LocalStorageAdapter>();
}

}  // namespace storage
}  // namespace infra
}  // namespace IM
