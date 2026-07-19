#include <gtest/gtest.h>
#include "draft/asset/asset_manager.hpp"
#include "draft/util/files/asset_file_system.hpp"
#include "draft/util/files/memory_file_provider.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace Draft;

namespace {
    // A trivial asset type, just the file's contents. Loaders below register with only a
    // single sync LoadFn, exercising get<T>()/queue()+load()
    struct TextAsset {
        std::string contents;
    };

    // A second, unrelated trivial type, so tests can prove different types don't collide.
    struct CountAsset {
        int value = 0;
    };

    // A type with a two-stage async loader. The off-thread stage sleeps briefly (to
    // stand in for real decode work) and hands back a raw string; the main-thread finish stage
    // uppercases it, so tests can tell the two stages apart by checking the result was
    // actually transformed by finish, not just passed through.
    struct SlowAsset {
        std::string value;
    };

    // Used by the "loader calls back into the manager" tests, mirroring the old engine's
    // ParticleLoader resolving a referenced Texture via assets.get<Texture>(...).
    struct Leaf {
        std::string text;
    };

    struct Branch {
        std::string text;
        Resource<Leaf> leaf;
    };

    std::string to_upper(std::string s){
        for(char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }

    // MemoryFileProvider's backing store is a process-wide static map (see its own header), not
    // scoped to one instance, so a fresh instance here still reaches the same data.
    void write_file(const std::filesystem::path& path, const std::string& contents){
        MemoryFileProvider().write_string(path, contents);
    }

    AssetFileSystem memory_fs(){
        std::vector<std::unique_ptr<FileProvider>> providers;
        providers.push_back(std::make_unique<MemoryFileProvider>());
        return AssetFileSystem(std::move(providers));
    }

    struct AssetManagerTest : ::testing::Test {
        // Scoped per test (not just "test_am_scratch"), since MemoryFileProvider's storage
        // outlives any one test within the same process, this keeps tests from ever reading
        // another test's leftovers even if someone adds a test later that forgets to write
        // before reading.
        std::filesystem::path dir = std::filesystem::path("test_am_scratch") / ::testing::UnitTest::GetInstance()->current_test_info()->name();

        std::string path(const std::string& name) const {
            return (dir / name).string();
        }
    };
}

TEST_F(AssetManagerTest, GetLoadsSynchronouslyOnFirstRequest)
{
    write_file(path("a.txt"), "hello");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    Resource<TextAsset> r = manager.get<TextAsset>(path("a.txt"));
    ASSERT_TRUE(r.is_valid());
    ASSERT_EQ(r->contents, "hello");
}

TEST_F(AssetManagerTest, GetCachesAndDoesNotReload)
{
    write_file(path("a.txt"), "hello");

    std::atomic<int> loadCount{0};
    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([&](const FileHandle& handle, AssetManager&){
        loadCount++;
        return TextAsset{handle.read_string()};
    });

    manager.get<TextAsset>(path("a.txt"));
    manager.get<TextAsset>(path("a.txt"));
    manager.get<TextAsset>(path("a.txt"));

    ASSERT_EQ(loadCount, 1);
}

TEST_F(AssetManagerTest, DifferentTypesAtDifferentKeysDoNotCollide)
{
    write_file(path("a.txt"), "text-content");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });
    manager.register_loader<CountAsset>([](const FileHandle&, AssetManager&){
        return CountAsset{42};
    });

    auto text = manager.get<TextAsset>(path("a.txt"));
    auto count = manager.get<CountAsset>(path("a.txt")); // same key string, different type

    ASSERT_EQ(text->contents, "text-content");
    ASSERT_EQ(count->value, 42);
}

TEST_F(AssetManagerTest, GetThrowsWhenNoLoaderRegistered)
{
    AssetManager manager(memory_fs());
    ASSERT_THROW(manager.get<TextAsset>(path("missing.txt")), std::logic_error);
}

TEST_F(AssetManagerTest, GetThrowsOnMissingFileWithNoPlaceholder)
{
    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    ASSERT_THROW(manager.get<TextAsset>(path("does_not_exist.txt")), std::runtime_error);
}

TEST_F(AssetManagerTest, GetFallsBackToPlaceholderAndRecordsTheError)
{
    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });
    manager.register_placeholder<TextAsset>(TextAsset{"MISSING"});

    Resource<TextAsset> r = manager.get<TextAsset>(path("does_not_exist.txt"));
    ASSERT_TRUE(r.is_valid());
    ASSERT_EQ(r->contents, "MISSING");

    ASSERT_EQ(manager.get_load_errors().size(), 1u);
    ASSERT_EQ(manager.get_load_errors()[0].key, path("does_not_exist.txt"));
}

TEST_F(AssetManagerTest, QueueAndLoadProcessesEverythingSynchronously)
{
    write_file(path("a.txt"), "AAA");
    write_file(path("b.txt"), "BBB");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    manager.queue<TextAsset>(path("a.txt"));
    manager.queue<TextAsset>(path("b.txt"));
    manager.load();

    ASSERT_FLOAT_EQ(manager.get_loading_progress(), 1.f);
    ASSERT_EQ(manager.get<TextAsset>(path("a.txt"))->contents, "AAA");
    ASSERT_EQ(manager.get<TextAsset>(path("b.txt"))->contents, "BBB");
}

TEST_F(AssetManagerTest, LoadContinuesPastAFailureWhenPlaceholderExists)
{
    write_file(path("a.txt"), "AAA");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });
    manager.register_placeholder<TextAsset>(TextAsset{"MISSING"});

    manager.queue<TextAsset>(path("a.txt"));
    manager.queue<TextAsset>(path("does_not_exist.txt"));
    manager.load();

    ASSERT_EQ(manager.get<TextAsset>(path("a.txt"))->contents, "AAA");
    ASSERT_EQ(manager.get<TextAsset>(path("does_not_exist.txt"))->contents, "MISSING");
    ASSERT_EQ(manager.get_load_errors().size(), 1u);
}

TEST_F(AssetManagerTest, LoadAsyncDoesNotBlockAndPollAsyncDrivesProgressToDone)
{
    write_file(path("slow.txt"), "hello");

    AssetManager manager(memory_fs());
    manager.register_loader<SlowAsset>(
        [](const FileHandle& handle) -> std::any {
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            return std::make_any<std::string>(handle.read_string());
        },
        [](std::any data, AssetManager&) -> SlowAsset {
            return SlowAsset{to_upper(std::any_cast<std::string>(data))};
        }
    );

    manager.queue<SlowAsset>(path("slow.txt"));

    auto start = std::chrono::steady_clock::now();
    manager.load_async();

    // load_async() itself must return almost immediately, the 80ms sleep happens on a worker
    // thread, not here. Checked by asking poll_async()
    ASSERT_FALSE(manager.poll_async());

    bool done = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while(std::chrono::steady_clock::now() < deadline){
        done = manager.poll_async();
        if(done) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    auto finished = std::chrono::steady_clock::now();

    ASSERT_TRUE(done);
    ASSERT_GE(finished - start, std::chrono::milliseconds(80)); // proves the sleep really ran
    ASSERT_FLOAT_EQ(manager.get_loading_progress(), 1.f);
    ASSERT_EQ(manager.get<SlowAsset>(path("slow.txt"))->value, "HELLO");
}

TEST_F(AssetManagerTest, ReloadUpdatesExistingResourceHandlesInPlace)
{
    write_file(path("a.txt"), "version1");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    Resource<TextAsset> handle = manager.get<TextAsset>(path("a.txt"));
    ASSERT_EQ(handle->contents, "version1");

    write_file(path("a.txt"), "version2");
    ASSERT_TRUE(manager.reload<TextAsset>(path("a.txt")));
    manager.load();

    // Same Resource<T> object, never re-requested from the manager, sees the new content.
    ASSERT_EQ(handle->contents, "version2");
}

TEST_F(AssetManagerTest, ReloadOnUnknownKeyReturnsFalse)
{
    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    ASSERT_FALSE(manager.reload<TextAsset>(path("never_loaded.txt")));
}

TEST_F(AssetManagerTest, UnloadInvalidatesExistingHandlesWithoutRefetching)
{
    write_file(path("a.txt"), "hello");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    Resource<TextAsset> handle = manager.get<TextAsset>(path("a.txt"));
    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(manager.unload<TextAsset>(path("a.txt")));
    ASSERT_FALSE(handle.is_valid());
}

TEST_F(AssetManagerTest, UnloadFallsBackToPlaceholderWhenRegistered)
{
    write_file(path("a.txt"), "hello");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });
    manager.register_placeholder<TextAsset>(TextAsset{"MISSING"});

    Resource<TextAsset> handle = manager.get<TextAsset>(path("a.txt"));
    manager.unload<TextAsset>(path("a.txt"));

    ASSERT_TRUE(handle.is_valid());
    ASSERT_EQ(handle->contents, "MISSING");
}

TEST_F(AssetManagerTest, LoaderCanCallBackIntoTheManagerForNestedAssets)
{
    write_file(path("leaf.txt"), "leaf-value");
    write_file(path("branch.txt"), "branch-value");

    AssetManager manager(memory_fs());
    manager.register_loader<Leaf>([](const FileHandle& handle, AssetManager&){
        return Leaf{handle.read_string()};
    });
    manager.register_loader<Branch>([this](const FileHandle& handle, AssetManager& mgr){
        Branch b;
        b.text = handle.read_string();
        b.leaf = mgr.get<Leaf>(path("leaf.txt"));
        return b;
    });

    Resource<Branch> branch = manager.get<Branch>(path("branch.txt"));
    ASSERT_EQ(branch->text, "branch-value");
    ASSERT_TRUE(branch->leaf.is_valid());
    ASSERT_EQ(branch->leaf->text, "leaf-value");
}

TEST_F(AssetManagerTest, CleanupInvalidatesEverything)
{
    write_file(path("a.txt"), "hello");

    AssetManager manager(memory_fs());
    manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });

    Resource<TextAsset> handle = manager.get<TextAsset>(path("a.txt"));
    ASSERT_TRUE(handle.is_valid());

    manager.cleanup();
    ASSERT_FALSE(handle.is_valid());
}

TEST_F(AssetManagerTest, TwoManagersAreFullyIndependent)
{
    write_file(path("a.txt"), "hello");

    AssetManager first(memory_fs());
    AssetManager second(memory_fs());

    first.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
        return TextAsset{handle.read_string()};
    });
    // Deliberately do not register a loader on `second`.

    ASSERT_NO_THROW(first.get<TextAsset>(path("a.txt")));
    ASSERT_THROW(second.get<TextAsset>(path("a.txt")), std::logic_error);
}
