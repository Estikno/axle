#pragma once

#include "axpch.hpp"
#include <cstddef>

#include "Core/Types.hpp"

namespace Axle {
    class RefCounted {
    public:
        RefCounted() = default;
        virtual ~RefCounted() = default;

        void IncRefCount() const {
            m_RefCount.fetch_add(1, std::memory_order_release);
        }

        void DecRefCount() const {
            m_RefCount.fetch_sub(1, std::memory_order_release);
        }

        u32 GetRefCount() const {
            return m_RefCount.load(std::memory_order_acquire);
        }

        u32 FetchAdd(u32 val) const {
            return m_RefCount.fetch_add(val, std::memory_order_acq_rel);
        }

        u32 FetchSub(u32 val) const {
            return m_RefCount.fetch_sub(val, std::memory_order_acq_rel);
        }

    private:
        mutable std::atomic<u32> m_RefCount = 0;
    };

    /**
     * Intrusive reference count
     * */
    template <typename T>
    class Ref {
    public:
        Ref() = default;
        ~Ref() {
            DecRef();
        }

        Ref(std::nullptr_t n)
            : m_Instance(nullptr) {}

        Ref(T* instance)
            : m_Instance(instance) {
            static_assert(std::is_base_of_v<RefCounted, T>, "Class is not RefCounted");
            IncRef();
        }

        template <typename U>
            requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
        Ref(Ref<U>&& other) {
            m_Instance = (T*) other.m_Instance;
            other.m_Instance = nullptr;
        }

        template <typename U>
            requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
        Ref(const Ref<U>& other) {
            m_Instance = (T*) other.m_Instance;
            IncRef();
        }

        template <typename U>
            requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
        Ref& operator=(Ref<U>&& other) {
            if (this != &other) {
                DecRef();
                m_Instance = other.m_Instance;
                other.m_Instance = nullptr;
            }
            return *this;
        }

        template <typename U>
            requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
        Ref& operator=(const Ref<U>& other) {
            if (this != &other) {
                DecRef();
                m_Instance = other.m_Instance;
                IncRef();
            }
            return *this;
        }

        operator bool() {
            return m_Instance != nullptr;
        }
        operator bool() const {
            return m_Instance != nullptr;
        }

        T* operator->() {
            return m_Instance;
        }
        const T* operator->() const {
            return m_Instance;
        }

        T& operator*() {
            return *m_Instance;
        }
        const T& operator*() const {
            return *m_Instance;
        }

        T* Raw() {
            return *m_Instance;
        }
        const T* Raw() const {
            return *m_Instance;
        }

        void Reset(T* instance = nullptr) {
            DecRef();
            m_Instance = instance;
        }

        template <typename U>
            requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
        Ref<U> As() const {
            return Ref<U>(*this);
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args) {
            return Ref<T>(new T(std::forward<Args>(args)...));
        }

        bool operator==(const Ref<T>& other) const {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const Ref<T>& other) const {
            return !(*this == other);
        }

        bool EqualObject(const Ref<T>& other) {
            if (!m_Instance || !other.m_Instance)
                return false;

            return *m_Instance == *other.m_Instance;
        }

    private:
        void IncRef() const {
            if (m_Instance) {
                m_Instance->IncRefCount();
            }
        }

        void DecRef() const {
            if (!m_Instance)
                return;

            // We are the last holding a reference
            if (m_Instance->FetchSub(1) == 1) {
                delete m_Instance;
                m_Instance = nullptr;
            }
        }

        template <typename U>
        friend class Ref;

        mutable T* m_Instance = nullptr;
    };
} // namespace Axle
