#pragma once

#include <stdexcept>

namespace Draft::Serializer {
    template<typename Ctx>
    struct ContextSlot {
        static inline thread_local Ctx* current = nullptr;
    };

    /**
     * @brief Makes @p ctx reachable via Serializer::context<Ctx>() until this goes out of scope.
     */
    template<typename Ctx>
    struct ScopedContext {
        Ctx* previous;

        explicit ScopedContext(Ctx& ctx) : previous(ContextSlot<Ctx>::current) {
            ContextSlot<Ctx>::current = &ctx;
        }

        ~ScopedContext(){
            ContextSlot<Ctx>::current = previous;
        }

        ScopedContext(const ScopedContext&) = delete;
        ScopedContext& operator=(const ScopedContext&) = delete;
    };

    /**
     * @brief Returns the currently active Ctx. Throws if no ScopedContext<Ctx> is active.
     */
    template<typename Ctx>
    Ctx& context(){
        if(!ContextSlot<Ctx>::current)
            throw std::logic_error("Serializer::context<Ctx>(): no ScopedContext<Ctx> is active");

        return *ContextSlot<Ctx>::current;
    }
}
