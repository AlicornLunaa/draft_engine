#pragma once

#include "draft/asset/resource.hpp"
#include "draft/util/files/asset_file_system.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <any>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    class AssetManager;

    /**
     * @brief The off-thread half of a two-stage async loader. It does the slow part (decode a
     * file into some intermediate representation) with no access to AssetManager, since this
     * runs on a worker thread and touching shared manager state from there isn't safe.
     * The result is type-erased (std::any) so AssetManager's per-type storage doesn't need a
     * second template parameter for whatever intermediate type a given loader uses internally.
     */
    template<typename T>
    using OffThreadLoadFn = std::function<std::any(const FileHandle&)>;

    /**
     * @brief The main-thread half of a two-stage async loader. It turns the intermediate value
     * OffThreadLoadFn produced into the final T. Runs on AssetManager's owning thread (via
     * poll_async()), so it may freely call back into @p manager.
     */
    template<typename T>
    using FinishLoadFn = std::function<T(std::any, AssetManager&)>;

    /**
     * @brief This is a simple short-hand function for syncronous only loaders. It shunts the FileHandle
     * through the OffThreadLoadFn automatically. Used to prevent lots of any casts for just FileHandle.
     */
    template<typename T>
    using LoadFn = std::function<T(const FileHandle&, AssetManager&)>;

    /**
     * @brief Records one failed load from a queue()+load()/load_async() batch.
     * The function get<T>() surfaces failures by throwing directly while the
     * batch API goes past a single bad assets, rather than aborting the whole process.
     */
    struct AssetLoadError {
        std::type_index type;
        std::string key;
        std::exception_ptr error;
    };

    namespace detail {
        /**
         * @brief Owns a pool of worker threads draining a FIFO of jobs.
         *
         * A submitted job runs on a worker thread and returns a "finish" callback; finish
         * callbacks are never run by a worker, they queue up until drain_completed() is called.
         * This split is what keeps worker threads from ever touching AssetManager's shared
         * state. Everything a job needs must be captured by value into the submitted callable,
         * and everything the result needs to update lives in the returned finish callback
         * instead, which runs later on the calling thread.
         */
        class JobRunner;
    }

    /**
     * @brief Base of TypeRegistry<T>, letting AssetManager hold a heterogeneous collection of
     * per-type registries behind a single std::type_index-keyed map.
     */
    struct TypeRegistryInterface {
        virtual ~TypeRegistryInterface() = default;

        /**
         * @brief Drops every loaded resource of this type, setting each slot to null first so
         * any outstanding Resource<T> immediately observes the unload.
         */
        virtual void clear() = 0;
    };

    template<typename T>
    struct TypeRegistry : TypeRegistryInterface {
        OffThreadLoadFn<T> offThreadLoad;
        FinishLoadFn<T> finishLoad;
        std::shared_ptr<AssetSlot<T>> placeholder;
        std::unordered_map<std::string, std::shared_ptr<AssetSlot<T>>> resources;

        bool has_loader() const { return static_cast<bool>(offThreadLoad) && static_cast<bool>(finishLoad); }

        void clear() override {
            for(auto& [key, slot] : resources)
                slot->set(nullptr);

            resources.clear();
        }
    };

    /**
     * @brief Owns a set of loaded assets, keyed by (type, path).
     * Loaders are registered per type via register_loader().
     *
     * Thread-safety: AssetManager's own methods (get/queue/load/load_async/poll_async/reload/
     * unload/cleanup/...) must all be called from a single owning thread. There is no locking
     * around registry state, by design, since the whole loading pipeline is meant to run on
     * (and be driven by) one thread, e.g. the main/game thread. What *is* safe from any thread
     * is reading a Resource<T> you already hold (see AssetSlot<T>).
     */
    class AssetManager {
    public:
        /**
         * @brief Constructs an AssetManager backed by @p fileSystem, with @p workerThreads
         * background threads available to load_async().
         */
        explicit AssetManager(AssetFileSystem fileSystem = AssetFileSystem(), std::size_t workerThreads = 2);

        // Declared, not defaulted, because ~AssetManager() must run where detail::JobRunner is implemented
        ~AssetManager();

        // Not copyable or movable because worker jobs capture `this`
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

        /**
         * @brief The file system this manager resolves asset paths against.
         */
        const AssetFileSystem& file_system() const { return m_fileSystem; }

        /**
         * @brief Registers an asset loader for T.
         * @p offThreadLoad runs on a worker thread (must not touch this AssetManager).
         * @p finishLoad runs back on the owning thread afterward (may freely call back into this AssetManager, e.g. to resolve a referenced sub-asset).
         */
        template<typename T>
        void register_loader(OffThreadLoadFn<T> offThreadLoad, FinishLoadFn<T> finishLoad){
            auto& reg = registry_for<T>();
            reg.offThreadLoad = std::move(offThreadLoad);
            reg.finishLoad = std::move(finishLoad);
        }

        /**
         * @brief Short-hand for a syncronous-only loader for T.
         * @p load runs on the owning thread. 
         * @p finishLoad runs back on the owning thread afterward (may freely call back into this AssetManager, e.g. to resolve a referenced sub-asset).
         */
        template<typename T>
        void register_loader(LoadFn<T> load){
            register_loader<T>(
                [](const FileHandle& handle){
                    // Shunt handle through to the syncronous function
                    return handle;
                },
                [load](std::any data, AssetManager& manager){
                    auto handle = std::any_cast<FileHandle>(data);
                    return load(handle, manager);
                }
            );
        }

        /**
         * @brief Registers the fallback value get<T>() and the batch APIs use when a load
         * fails. Without one, a failure propagates as an exception in get<T>() or is recorded
         * via get_load_errors() and leaves that key unloaded in load()/load_async().
         */
        template<typename T>
        void register_placeholder(T value){
            registry_for<T>().placeholder = std::make_shared<AssetSlot<T>>(std::make_shared<T>(std::move(value)));
        }

        /**
         * @brief Gets the resource at @p key, loading it synchronously right now if it isn't
         * already loaded or queued.
         * @throws Whatever T's registered loader throws, or std::logic_error if no loader is
         * registered for T, unless a placeholder is registered for T in which case the
         * failure is logged and the placeholder is returned instead.
         */
        template<typename T>
        Resource<T> get(const std::string& key){
            auto& reg = registry_for<T>();

            auto it = reg.resources.find(key);
            if(it != reg.resources.end())
                return Resource<T>(it->second);

            if(!reg.has_loader())
                throw std::logic_error("AssetManager::get(): no loader registered for requested type");

            auto slot = std::make_shared<AssetSlot<T>>();
            reg.resources.emplace(key, slot);

            try {
                FileHandle handle = m_fileSystem.open(key);
                slot->set(std::make_shared<T>(reg.finishLoad(reg.offThreadLoad(handle), *this)));
            } catch(...){
                std::exception_ptr error = std::current_exception();
                record_error<T>(key, error);

                if(reg.placeholder){
                    log_fallback(key, error);
                    slot->set(reg.placeholder->get());
                } else {
                    reg.resources.erase(key); // don't permanently cache a failed, un-recoverable slot
                    std::rethrow_exception(error);
                }
            }

            return Resource<T>(slot);
        }

        /**
         * @brief Queues @p key for loading on the next load() or load_async() call.
         * A no-op if @p key is already loaded or queued for T.
         */
        template<typename T>
        void queue(const std::string& key){
            auto& reg = registry_for<T>();
            if(reg.resources.contains(key))
                return;

            auto slot = std::make_shared<AssetSlot<T>>();
            reg.resources.emplace(key, slot);
            enqueue_load<T>(key, slot);
        }

        /**
         * @brief Runs every queued load synchronously, right now, on the calling thread.
         * Failures are logged and fall back to a placeholder if one is registered for that
         * asset's type, otherwise the key is left unloaded
         */
        void load(){
            m_loadErrors.clear();
            auto jobs = std::move(m_pendingJobs);
            m_pendingJobs.clear();
            m_totalQueued = jobs.size();
            m_completedSinceStart = 0;

            for(auto& job : jobs){
                job.run()();
                m_completedSinceStart++;
            }
        }

        /**
         * @brief Starts every queued load on background worker threads where possible,
         * call poll_async() periodically (e.g. once per frame) to pick up completions and
         * drive progress toward done. Non-blocking, aside from any finish tasks.
         */
        void load_async();

        /**
         * @brief Finishes whatever background loads have completed since the last call, on the
         * calling thread. Call this periodically (e.g. once per frame) after load_async().
         * @return True once every load queued by the triggering load_async() call has finished.
         */
        bool poll_async();

        /**
         * @brief Fraction of the current (or most recent) load_async()/load() batch that has
         * finished, in [0, 1]. 1 if nothing has ever been queued.
         */
        float get_loading_progress() const {
            if(m_totalQueued == 0)
                return 1.f;

            return static_cast<float>(m_completedSinceStart) / static_cast<float>(m_totalQueued);
        }

        /**
         * @brief Failures from the most recent load()/load_async() batch (cleared at the start
         * of each one). Empty if everything in that batch succeeded.
         */
        const std::vector<AssetLoadError>& get_load_errors() const { return m_loadErrors; }

        /**
         * @brief Re-queues an already-loaded resource for a fresh load, reusing its existing
         * slot. Every Resource<T> already handed out for @p key will observe the new value as
         * soon as the reload completes (via load() or load_async()+poll_async()), with no need
         * to call get<T>() again.
         * @return False if @p key isn't currently loaded for T.
         */
        template<typename T>
        bool reload(const std::string& key){
            auto& reg = registry_for<T>();
            auto it = reg.resources.find(key);
            if(it == reg.resources.end())
                return false;

            enqueue_load<T>(key, it->second);
            return true;
        }

        /**
         * @brief Unloads @p key, immediately swapping its slot to the registered placeholder
         * for T (if any) or null. Any Resource<T> already handed out for it observes this
         * right away, with no dangling access.
         * @return False if @p key isn't currently loaded for T.
         */
        template<typename T>
        bool unload(const std::string& key){
            auto& reg = registry_for<T>();
            auto it = reg.resources.find(key);
            if(it == reg.resources.end())
                return false;

            it->second->set(reg.placeholder ? reg.placeholder->get() : nullptr);
            reg.resources.erase(it);
            return true;
        }

        /**
         * @brief Reverse lookup: the key @p resource was loaded under for T, if it's still
         * loaded under one. Used by scene serialization to turn a live Resource<T> back into a
         * string key. Doesn't create a registry for T if one doesn't already exist.
         */
        template<typename T>
        std::optional<std::string> key_for(const Resource<T>& resource) const {
            auto it = m_registries.find(typeid(T));
            if(it == m_registries.end())
                return std::nullopt;

            const auto& reg = static_cast<const TypeRegistry<T>&>(*it->second);
            const void* id = resource.slot_id();

            for(const auto& [key, slot] : reg.resources)
                if(slot.get() == id)
                    return key;

            return std::nullopt;
        }

        /**
         * @brief Unloads every resource of every type, setting all slots to null.
         */
        void cleanup(){
            for(auto& [type, reg] : m_registries)
                reg->clear();

            m_pendingJobs.clear();
            m_loadErrors.clear();
        }

    private:
        template<typename T>
        TypeRegistry<T>& registry_for(){
            auto it = m_registries.find(typeid(T));
            if(it == m_registries.end())
                it = m_registries.emplace(typeid(T), std::make_unique<TypeRegistry<T>>()).first;

            return static_cast<TypeRegistry<T>&>(*it->second);
        }

        template<typename T>
        void record_error(const std::string& key, std::exception_ptr error){
            m_loadErrors.push_back(AssetLoadError{typeid(T), key, error});
        }

        static void log_fallback(const std::string& key, std::exception_ptr error){
            std::string reason = "unknown error";
            try { std::rethrow_exception(error); }
            catch(const std::exception& e){ reason = e.what(); }
            catch(...){}

            Logger::println(LogLevel::Warning, "AssetManager", "Failed to load \"" + key + "\" (" + reason + ") - using placeholder");
        }

        template<typename T>
        void complete_two_stage_load(const std::string& key, const std::shared_ptr<AssetSlot<T>>& slot,
                                      const std::shared_ptr<AssetSlot<T>>& placeholder,
                                      const FinishLoadFn<T>& finish,
                                      std::any&& data, std::exception_ptr error)
        {
            try {
                if(error)
                    std::rethrow_exception(error);

                T value = finish(std::move(data), *this);
                slot->set(std::make_shared<T>(std::move(value)));
            } catch(...){
                std::exception_ptr caught = std::current_exception();
                record_error<T>(key, caught);

                if(placeholder){
                    log_fallback(key, caught);
                    slot->set(placeholder->get());
                } else {
                    registry_for<T>().resources.erase(key);
                }
            }
        }

        /**
         * @brief Builds and queues the job for (re)loading @p key into @p slot
         */
        template<typename T>
        void enqueue_load(const std::string& key, std::shared_ptr<AssetSlot<T>> slot){
            auto& reg = registry_for<T>();
            std::shared_ptr<AssetSlot<T>> placeholder = reg.placeholder;

            if(reg.has_loader()){
                OffThreadLoadFn<T> offThreadLoad = reg.offThreadLoad;
                FinishLoadFn<T> finish = reg.finishLoad;

                m_pendingJobs.push_back(PendingJob{
                    .canRunOffThread = true,
                    .run = [this, key, offThreadLoad, slot, placeholder, finish]() -> std::function<void()> {
                        // Off-thread stage, only touches m_fileSystem (const, safe for concurrent reads)
                        // and the copied loader function. Never touches m_registries/m_pendingJobs/etc.
                        std::any data;
                        std::exception_ptr error;

                        try {
                            FileHandle handle = m_fileSystem.open(key);
                            data = offThreadLoad(handle);
                        } catch(...){
                            error = std::current_exception();
                        }

                        return [this, key, slot, placeholder, finish, data = std::move(data), error]() mutable {
                            complete_two_stage_load<T>(key, slot, placeholder, finish, std::move(data), error);
                        };
                    }
                });
            } else {
                throw std::logic_error("AssetManager::queue(): no loader registered for requested type");
            }
        }

        struct PendingJob {
            bool canRunOffThread;
            std::function<std::function<void()>()> run;
        };

        AssetFileSystem m_fileSystem;
        std::unordered_map<std::type_index, std::unique_ptr<TypeRegistryInterface>> m_registries;
        std::vector<PendingJob> m_pendingJobs;
        std::vector<AssetLoadError> m_loadErrors;
        std::size_t m_totalQueued = 0;
        std::size_t m_completedSinceStart = 0;

        // Declared last so it's destroyed (and its workers joined) first
        std::unique_ptr<detail::JobRunner> m_jobRunner;
    };
}
