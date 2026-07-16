#include "draft/util/files/archive_file_provider.hpp"

#include "miniz.h"

#include <stdexcept>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {
    // Strips a leading "./" and any trailing '/' so lookups don't have to special-case them
    std::string normalize(const fs::path& path) {
        std::string result = path.generic_string();
        if (result.rfind("./", 0) == 0) result.erase(0, 2);
        while (!result.empty() && result.back() == '/') result.pop_back();
        return result;
    }
}

namespace Draft {
    struct ArchiveFileProvider::Impl {
        struct Entry {
            unsigned int fileIndex; // only meaningful when !isDirectory
            bool isDirectory;
        };

        std::string archiveAbsolutePath;
        std::vector<std::byte> archiveBytes; // mz_zip_reader_init_mem doesn't copy this, must outlive `archive`
        mz_zip_archive archive{};
        std::unordered_map<std::string, Entry> index;

        ~Impl() {
            mz_zip_reader_end(&archive);
        }
    };

    ArchiveFileProvider::ArchiveFileProvider(const FileHandle& handle) : ptr(std::make_shared<Impl>()) {
        ptr->archiveAbsolutePath = handle.get_absolute_path();
        ptr->archiveBytes = handle.read_bytes();

        if (!mz_zip_reader_init_mem(&ptr->archive, ptr->archiveBytes.data(), ptr->archiveBytes.size(), 0)) {
            mz_zip_error err = mz_zip_get_last_error(&ptr->archive);
            throw std::runtime_error("ArchiveFileProvider: failed to open '" + ptr->archiveAbsolutePath + "' (" + mz_zip_get_error_string(err) + ")");
        }

        mz_uint fileCount = mz_zip_reader_get_num_files(&ptr->archive);
        for (mz_uint i = 0; i < fileCount; i++) {
            mz_zip_archive_file_stat stat;
            if (!mz_zip_reader_file_stat(&ptr->archive, i, &stat)) continue;

            std::string name = normalize(fs::path(stat.m_filename));
            if (name.empty()) continue;

            ptr->index[name] = Impl::Entry{i, static_cast<bool>(stat.m_is_directory)};

            // Register every ancestor directory this entry implies, even if the zip has no
            // explicit directory entry for it (plenty of zip writers omit them).
            for (fs::path parent = fs::path(name).parent_path(); !parent.empty(); parent = parent.parent_path()) {
                std::string parentKey = parent.generic_string();
                if (ptr->index.contains(parentKey)) break;
                ptr->index[parentKey] = Impl::Entry{0, true};
            }
        }
    }

    unsigned int ArchiveFileProvider::locate_file(const fs::path& path) const {
        auto it = ptr->index.find(normalize(path));
        if (it == ptr->index.end())
            throw std::runtime_error("ArchiveFileProvider: '" + path.generic_string() + "' does not exist in '" + ptr->archiveAbsolutePath + "'");
        if (it->second.isDirectory)
            throw std::runtime_error("ArchiveFileProvider: '" + path.generic_string() + "' is a directory, not a file");
        return it->second.fileIndex;
    }

    std::unique_ptr<FileProvider> ArchiveFileProvider::clone() const {
        return std::make_unique<ArchiveFileProvider>(*this);
    }

    bool ArchiveFileProvider::exists(const fs::path& path) const {
        return ptr->index.contains(normalize(path));
    }

    bool ArchiveFileProvider::is_directory(const fs::path& path) const {
        auto it = ptr->index.find(normalize(path));
        return it != ptr->index.end() && it->second.isDirectory;
    }

    bool ArchiveFileProvider::is_file(const fs::path& path) const {
        auto it = ptr->index.find(normalize(path));
        return it != ptr->index.end() && !it->second.isDirectory;
    }

    std::uintmax_t ArchiveFileProvider::size(const fs::path& path) const {
        mz_zip_archive_file_stat stat;
        mz_zip_reader_file_stat(&ptr->archive, locate_file(path), &stat);
        return static_cast<std::uintmax_t>(stat.m_uncomp_size);
    }

    Time ArchiveFileProvider::last_modified(const fs::path& path) const {
        mz_zip_archive_file_stat stat;
        mz_zip_reader_file_stat(&ptr->archive, locate_file(path), &stat);
        return Time::microseconds(static_cast<int64_t>(stat.m_time) * 1'000'000);
    }

    bool ArchiveFileProvider::remove(const fs::path& path) const {
        throw std::logic_error("ArchiveFileProvider: cannot remove '" + path.string() + "', archives are read-only");
    }

    bool ArchiveFileProvider::create_directories(const fs::path& path) const {
        throw std::logic_error("ArchiveFileProvider: cannot create directories for '" + path.string() + "', archives are read-only");
    }

    std::string ArchiveFileProvider::read_string(const fs::path& path) const {
        auto bytes = read_bytes(path, 0);
        return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }

    void ArchiveFileProvider::write_string(const fs::path& path, const std::string&) const {
        throw std::logic_error("ArchiveFileProvider: cannot write to '" + path.string() + "', archives are read-only");
    }

    std::vector<std::byte> ArchiveFileProvider::read_bytes(const fs::path& path, std::size_t offset) const {
        unsigned int fileIndex = locate_file(path);

        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&ptr->archive, fileIndex, &stat))
            throw std::runtime_error("ArchiveFileProvider: failed to stat '" + path.generic_string() + "'");

        std::vector<std::byte> full(stat.m_uncomp_size);
        if (stat.m_uncomp_size > 0 && !mz_zip_reader_extract_to_mem(&ptr->archive, fileIndex, full.data(), full.size(), 0))
            throw std::runtime_error("ArchiveFileProvider: failed to extract '" + path.generic_string() + "'");

        if (offset >= full.size()) return {};
        return std::vector<std::byte>(full.begin() + offset, full.end());
    }

    void ArchiveFileProvider::write_bytes(const fs::path& path, const void*, std::size_t) const {
        throw std::logic_error("ArchiveFileProvider: cannot write to '" + path.string() + "', archives are read-only");
    }

    std::vector<fs::path> ArchiveFileProvider::list(const fs::path& path) const {
        if (!is_directory(path)) return {};

        std::string prefix = normalize(path);
        std::vector<fs::path> result;

        for (const auto& [key, entry] : ptr->index) {
            fs::path parent = fs::path(key).parent_path();
            bool isDirectChild = prefix.empty() ? parent.empty() : (parent.generic_string() == prefix);
            if (isDirectChild) result.push_back(fs::path(key));
        }

        return result;
    }

    std::string ArchiveFileProvider::get_absolute_path(const fs::path& path) const {
        // Entries live inside the archive, not on the real filesystem. The archive's own
        // absolute path plus the entry's path inside it is as "absolute" as it gets.
        return ptr->archiveAbsolutePath + "!" + normalize(path);
    }
}
