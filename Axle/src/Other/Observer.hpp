#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"

namespace Axle {
    /**
     * Represents a source of events or notifications
     * Observers can subscribe handlers (functions) to this subject.
     */
    template <typename... Args>
    class Subject {
    public:
        /// The type of function the subject accepts
        using HandlerType = std::function<void(Args*...)>;

        virtual ~Subject() = default;

        /**
         * Subscribe a new handler to this Subject.
         *
         * @param handler A callable that accepts (Args...) arguments.
         * @return Subscription object that manages the subscription lifetime.
         */
        virtual size_t Subscribe(const HandlerType& handler) {
            i32 id = m_nextId++;
            m_handlers[id] = handler;
            return id;
        }

        /**
         * Notify all subscribed handlers with provided arguments
         */
        virtual void Notify(Args*... args) {
            for (auto& [id, handler] : m_handlers) {
                handler(args...);
            }
        }

        /**
         * Remove a subscription by its ID
         *
         * Called internally by Subscription when unsubscribing
         */
        virtual void Unsubscribe(size_t id) {
            m_handlers.erase(id);
        }

    protected:
        /// Active handlers mapped by their ID
        std::unordered_map<size_t, HandlerType> m_handlers;
        /// Next unique ID for new subscriptions
        size_t m_nextId = 0;
    };
} // namespace Axle
