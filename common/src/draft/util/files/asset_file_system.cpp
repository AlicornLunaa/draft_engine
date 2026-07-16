#include "draft/util/files/asset_file_system.hpp"
#include "draft/util/files/archive_file_provider.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/embedded_file_provider.hpp"

#include <memory>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    AssetFileSystem::AssetFileSystem() {
        m_providers.push_back(std::make_unique<DiskFileProvider>());

        DiskFileProvider fs;
        if(fs.is_file("assets.apak")){
            m_providers.push_back(std::make_unique<ArchiveFileProvider>(fs.open("assets.apak")));
        }

        m_providers.push_back(std::make_unique<EmbeddedFileProvider>());
    }

    AssetFileSystem::AssetFileSystem(std::vector<std::unique_ptr<FileProvider>> providers)
        : m_providers(std::move(providers)) {}

    void AssetFileSystem::add_provider(const FileProvider& provider) {
        m_providers.push_back(provider.clone());
    }

    bool AssetFileSystem::exists(const fs::path& path) const {
        for (const auto& provider : m_providers) {
            if (provider->exists(path)) return true;
        }
        return false;
    }

    FileHandle AssetFileSystem::open(const fs::path& path) const {
        for (const auto& provider : m_providers) {
            if (provider->exists(path)) return provider->open(path);
        }

        throw std::runtime_error("AssetFileSystem: '" + path.string() + "' was not found in any registered provider");
    }

    std::string AssetFileSystem::read_string(const fs::path& path) const {
        return open(path).read_string();
    }

    std::vector<std::byte> AssetFileSystem::read_bytes(const fs::path& path, std::size_t offset) const {
        return open(path).read_bytes(offset);
    }
}
