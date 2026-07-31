#pragma once

#include "axpch.hpp"
#include <cstddef>

#include "Core/Types.hpp"

namespace Axle {
    template <typename T>
    class WeakRef;
    class RefCounted;

    class RefControlBlock {
    public:
        explicit RefControlBlock(RefCounted* instance)
            : m_Instance(instance) {}

        RefCounted* Get() const {
            return m_Instance.load(std::memory_order_acquire);
        }

        void Expire() {
            m_Instance.store(nullptr, std::memory_order_release);
        }

        void IncWeak() const {
            m_WeakCount.fetch_add(1, std::memory_order_relaxed);
        }

        // Returns true if this was the last weak reference.
        bool DecWeak() const {
            return m_WeakCount.fetch_sub(1, std::memory_order_acq_rel) == 1;
        }

    private:
        std::atomic<RefCounted*> m_Instance;
        mutable std::atomic<u32> m_WeakCount = 1;
    };

    class RefCounted {
    public:
        RefCounted()
            : m_Control(new RefControlBlock(this)) {}
        virtual ~RefCounted() {
            m_Control->Expire();
            if (m_Control->DecWeak())
                delete m_Control;
        }

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

        // Increments only if the object isn't already at zero — prevents
        // resurrecting something that's mid-destruction on another thread.
        bool TryIncRefCount() const {
            u32 count = m_RefCount.load(std::memory_order_relaxed);
            while (count != 0) {
                if (m_RefCount.compare_exchange_weak(
                        count, count + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                    return true;
                }
            }
            return false;
        }

        RefControlBlock* GetControlBlock() const {
            return m_Control;
        }

    private:
        mutable std::atomic<u32> m_RefCount = 0;
        RefControlBlock* m_Control;
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

        Ref(const Ref& other)
            : m_Instance(other.m_Instance) {
            IncRef();
        }

        static Ref FromWeak(const WeakRef<T>& weak) {
            return weak.Lock();
        }

        static Ref AdoptAlreadyIncremented(T* instance) {
            Ref r;
            r.m_Instance = instance; // deliberately skip IncRef()
            return r;
        }

        Ref& operator=(const Ref& other) {
            if (this != &other) {
                DecRef();
                m_Instance = other.m_Instance;
                IncRef();
            }
            return *this;
        }

        Ref(Ref&& other) noexcept
            : m_Instance(other.m_Instance) {
            other.m_Instance = nullptr;
        }

        Ref& operator=(Ref&& other) noexcept {
            if (this != &other) {
                DecRef();
                m_Instance = other.m_Instance;
                other.m_Instance = nullptr;
            }
            return *this;
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
            return m_Instance;
        }
        const T* Raw() const {
            return m_Instance;
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

    template <typename T>
    class WeakRef {
    public:
        WeakRef() = default;

        WeakRef(const Ref<T>& strong) {
            if (strong.Raw()) {
                m_Control = strong.Raw()->GetControlBlock();
                m_Control->IncWeak();
            }
        }

        WeakRef(const WeakRef& other)
            : m_Control(other.m_Control) {
            if (m_Control)
                m_Control->IncWeak();
        }

        WeakRef& operator=(const WeakRef& other) {
            if (this != &other) {
                Release();
                m_Control = other.m_Control;
                if (m_Control)
                    m_Control->IncWeak();
            }
            return *this;
        }

        ~WeakRef() {
            Release();
        }

        bool Expired() const {
            return !m_Control || m_Control->Get() == nullptr;
        }

        // The weak_ptr::lock() equivalent — the only safe way to get a
        // strong Ref back out of a WeakRef.
        Ref<T> Lock() const {
            if (!m_Control)
                return nullptr;
            RefCounted* raw = m_Control->Get();
            if (!raw || !raw->TryIncRefCount())
                return nullptr;
            return Ref<T>::AdoptAlreadyIncremented(static_cast<T*>(raw));
        }

    private:
        void Release() {
            if (m_Control && m_Control->DecWeak())
                delete m_Control;
            m_Control = nullptr;
        }

        RefControlBlock* m_Control = nullptr;
    };
} // namespace Axle
