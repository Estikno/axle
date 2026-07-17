#pragma once

#include "axpch.hpp"

#include <expected>

#include "Error.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    template <typename T, typename E = Error>
    class Result {
    public:
        Result(const T& val)
            : m_Inner(val) {}
        Result(T&& val)
            : m_Inner(std::move(val)) {}
        Result(std::unexpected<E> err)
            : m_Inner(std::move(err)) {}

        static Result Ok(T val) {
            return Result(std::move(val));
        }
        static Result Err(E err) {
            return Result(std::unexpected<E>(std::move(err)));
        }

        bool IsOk() const noexcept {
            return m_Inner.has_value();
        }
        bool IsErr() const noexcept {
            return !m_Inner.has_value();
        }
        explicit operator bool() const noexcept {
            return IsOk();
        }

        T& Unwrap() & {
            if (IsErr())
                AX_PANIC(LogChannel::Core, "Unwrap() called on an Err Result: {0}", m_Inner.error());
            return *m_Inner;
        }

        T&& Unwrap() && {
            if (IsErr())
                AX_PANIC(LogChannel::Core, "Unwrap() called on an Err Result: {0}", m_Inner.error());
            return std::move(*m_Inner);
        }

        T& Expect(std::string_view msg) & {
            if (IsErr())
                AX_PANIC(LogChannel::Core, "{0}: {1}", msg, m_Inner.error());
            return *m_Inner;
        }

        E& UnwrapErr() {
            if (IsOk())
                AX_PANIC(LogChannel::Core, "UnwrapErr() called on a Ok Result");
            return m_Inner.error();
        }

        T& UnwrapOr(const T& fallback) const {
            return IsOk() ? *m_Inner : std::move(fallback);
        }

        template <typename F>
        T& UnwrapOrElse(F&& f) const {
            return IsOk() ? *m_Inner : f(m_Inner.error());
        }

        template <typename F>
        auto Map(F&& f) -> Result<std::invoke_result_t<F, T>, E> {
            using U = std::invoke_result_t<F, T>;
            if (IsOk())
                return Result<U, E>::Ok(f(*m_Inner));
            return Result<U, E>::Err(m_Inner.error());
        }

        template <typename F>
        Result MapErr(F&& f) {
            if (!m_Inner.has_value())
                return Result::Err(f(m_Inner.error()));
            return *this;
        }

        template <typename F>
        auto AndThen(F&& f) -> std::invoke_result_t<F, T> {
            if (m_Inner.has_value())
                return f(*m_Inner);
            return std::invoke_result_t<F, T>::Err(m_Inner.error());
        }

        std::optional<T> Ok() const {
            return m_Inner.has_value() ? std::optional<T>(*m_Inner) : std::nullopt;
        }

    private:
        std::expected<T, E> m_Inner;
    };

    // Void specialization
    template <typename E>
    class Result<void, E> {
    public:
        Result()
            : m_Inner() {}
        Result(std::unexpected<E> err)
            : m_Inner(std::move(err)) {}

        static Result Ok() {
            return Result();
        }
        static Result Err(E err) {
            return Result(std::unexpected<E>(std::move(err)));
        }

        bool IsOk() const noexcept {
            return m_Inner.has_value();
        }
        bool IsErr() const noexcept {
            return !m_Inner.has_value();
        }

        void Unwrap() const {
            if (IsErr())
                AX_PANIC(LogChannel::Core, "Unwrap() called on an Err Result: {}", m_Inner.error());
        }

        E& UnwrapErr() {
            if (IsOk())
                AX_PANIC(LogChannel::Core, "UnwrapErr() called on an Ok Result");
            return m_Inner.error();
        }

    private:
        std::expected<void, E> m_Inner;
    };
} // namespace Axle
