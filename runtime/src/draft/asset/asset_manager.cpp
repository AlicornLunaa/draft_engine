#include "draft/asset/asset_manager.hpp"
#include "draft/asset/default_loaders.hpp"
#include "draft/audio/music.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/physics/collider.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture_packer.hpp"
#include "draft/util/json.hpp"
#include "draft/util/localization.hpp"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace Draft::detail {
    class JobRunner {
    public:
        explicit JobRunner(std::size_t workerCount);
        ~JobRunner();

        JobRunner(const JobRunner&) = delete;
        JobRunner& operator=(const JobRunner&) = delete;

        void submit(std::function<std::function<void()>()> job);
        std::size_t drain_completed();
        std::size_t pending_count() const;

    private:
        void worker_loop(std::stop_token token);

        mutable std::mutex m_mutex;
        std::condition_variable_any m_cv;
        std::deque<std::function<std::function<void()>()>> m_pending;
        std::deque<std::function<void()>> m_completed;

        // Jobs submitted but not yet drained. Deliberately not derived from
        // m_pending.size() + m_completed.size(). a worker pops a job off m_pending before
        // running it and only pushes to m_completed after it returns, so for the job's entire
        // execution window (e.g. however long its off-thread stage takes) it would sit in
        // neither deque, undercounting exactly the in-flight work pending_count() exists to
        // report.
        std::size_t m_outstanding = 0;

        // Declared last so it's destroyed (stopped + joined) first.
        std::vector<std::jthread> m_workers;
    };

    JobRunner::JobRunner(std::size_t workerCount){
        m_workers.reserve(workerCount);

        for(std::size_t i = 0; i < workerCount; i++)
            m_workers.emplace_back([this](std::stop_token token){ worker_loop(token); });
    }

    JobRunner::~JobRunner(){
        for(auto& worker : m_workers)
            worker.request_stop();

        m_cv.notify_all();
        // std::jthread's own destructor joins each worker which, per worker_loop(), only
        // exits once m_pending is drained, so every already-submitted job still runs to
        // completion.
    }

    void JobRunner::submit(std::function<std::function<void()>()> job){
        {
            std::lock_guard lock(m_mutex);
            m_pending.push_back(std::move(job));
            m_outstanding++;
        }

        m_cv.notify_one();
    }

    std::size_t JobRunner::drain_completed(){
        std::deque<std::function<void()>> ready;

        {
            std::lock_guard lock(m_mutex);
            ready.swap(m_completed);
            m_outstanding -= ready.size();
        }

        for(auto& finish : ready)
            finish();

        return ready.size();
    }

    std::size_t JobRunner::pending_count() const {
        std::lock_guard lock(m_mutex);
        return m_outstanding;
    }

    void JobRunner::worker_loop(std::stop_token token){
        while(true){
            std::function<std::function<void()>()> job;

            {
                std::unique_lock lock(m_mutex);
                m_cv.wait(lock, token, [this]{ return !m_pending.empty(); });

                // Only reachable with an empty queue if wait() gave up because of a stop
                // request with nothing left pending, otherwise the predicate guarantees
                // m_pending is non-empty here.
                if(m_pending.empty())
                    return;

                job = std::move(m_pending.front());
                m_pending.pop_front();
            }

            std::function<void()> finish = job();

            {
                std::lock_guard lock(m_mutex);
                m_completed.push_back(std::move(finish));
            }
        }
    }
}

namespace Draft {
    AssetManager::AssetManager(AssetFileSystem fileSystem, std::size_t workerThreads)
        : m_fileSystem(std::move(fileSystem)), m_jobRunner(std::make_unique<detail::JobRunner>(workerThreads))
    {
        // Default loader implementations
        Loaders::register_default_loader<Animation>(*this);
        Loaders::register_default_loader<Collider>(*this);
        Loaders::register_default_loader<Font>(*this);
        Loaders::register_default_loader<Image>(*this);
        Loaders::register_default_loader<JSON>(*this);
        Loaders::register_default_loader<TexturePacker>(*this);
        Loaders::register_default_loader<ParticleProps>(*this);
        Loaders::register_default_loader<Shader>(*this);
        Loaders::register_default_loader<SoundBuffer>(*this);
        Loaders::register_default_loader<Music>(*this);
        Loaders::register_default_loader<Texture>(*this);
        Loaders::register_default_loader<Localization>(*this);
    }

    AssetManager::~AssetManager() = default;

    void AssetManager::load_async(){
        m_loadErrors.clear();
        auto jobs = std::move(m_pendingJobs);
        m_pendingJobs.clear();
        m_totalQueued = jobs.size();
        m_completedSinceStart = 0;

        for(auto& job : jobs){
            if(job.canRunOffThread){
                m_jobRunner->submit(std::move(job.run));
            } else {
                // Can't safely hand this to a worker thread run and finish it inline instead,
                // so load_async() blocks briefly for just this one asset rather than risking a data race.
                Logger::println(LogLevel::Warning, "AssetManager",
                    "load_async(): a queued asset's type has no async loader registered, loading it synchronously instead");

                auto finish = job.run();
                finish();
                m_completedSinceStart++;
            }
        }
    }

    bool AssetManager::poll_async(){
        m_completedSinceStart += m_jobRunner->drain_completed();
        return m_jobRunner->pending_count() == 0;
    }
}
