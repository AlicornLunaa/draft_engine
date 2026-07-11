#include "draft/util/files/disk_file_provider.hpp"

#include <chrono>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    std::unique_ptr<FileProvider> DiskFileProvider::clone() const {
        return std::make_unique<DiskFileProvider>(*this);
    }

    bool DiskFileProvider::exists(const fs::path& path) const {
        return fs::exists(path);
    }

    bool DiskFileProvider::is_directory(const fs::path& path) const {
        return fs::is_directory(path);
    }

    bool DiskFileProvider::is_file(const fs::path& path) const {
        return fs::is_regular_file(path);
    }

    std::uintmax_t DiskFileProvider::size(const fs::path& path) const {
        return fs::file_size(path);
    }

    Time DiskFileProvider::last_modified(const fs::path& path) const {
        auto last_time = fs::last_write_time(path);
        auto since_epoch = std::chrono::file_clock::to_sys(last_time).time_since_epoch();
        return Time::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count());
    }

    bool DiskFileProvider::remove(const fs::path& path) const {
        return fs::remove_all(path) > 0;
    }

    bool DiskFileProvider::create_directories(const fs::path& path) const {
        if (path.parent_path().empty()) return false;
        return fs::create_directories(path.parent_path());
    }

    std::string DiskFileProvider::read_string(const fs::path& path) const {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in) throw std::runtime_error("DiskFileProvider: failed to open '" + path.string() + "' for reading");

        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        return contents;
    }

    void DiskFileProvider::write_string(const fs::path& path, const std::string& str) const {
        create_directories(path);
        std::ofstream out(path, std::ios::out | std::ios::binary);
        if (!out) throw std::runtime_error("DiskFileProvider: failed to open '" + path.string() + "' for writing");
        out << str;
    }

    std::vector<std::byte> DiskFileProvider::read_bytes(const fs::path& path, std::size_t offset) const {
        std::ifstream in(path, std::ios::binary);
        if (!in) throw std::runtime_error("DiskFileProvider: failed to open '" + path.string() + "' for reading");

        in.seekg(0, std::ios::end);
        std::size_t file_size = in.tellg();

        if (offset >= file_size) return {};

        std::size_t read_size = file_size - offset;
        std::vector<std::byte> buffer(read_size);

        in.seekg(offset, std::ios::beg);
        in.read(reinterpret_cast<char*>(buffer.data()), read_size);
        return buffer;
    }

    void DiskFileProvider::write_bytes(const fs::path& path, const void* data, std::size_t size) const {
        create_directories(path);
        std::ofstream out(path, std::ios::binary);
        if (!out) throw std::runtime_error("DiskFileProvider: failed to open '" + path.string() + "' for writing");
        out.write(reinterpret_cast<const char*>(data), size);
    }

    std::vector<fs::path> DiskFileProvider::list(const fs::path& path) const {
        if (!fs::is_directory(path)) return {};

        std::vector<fs::path> result;
        for (const auto& entry : fs::directory_iterator(path)) {
            result.push_back(entry.path());
        }
        return result;
    }

    std::string DiskFileProvider::get_absolute_path(const fs::path& path) const {
        if (path.empty()) return "";
        return fs::absolute(path).string();
    }
}
