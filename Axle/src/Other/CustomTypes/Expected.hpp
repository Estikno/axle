// This implementation is heavily inspired by Alexandrescu's and by Rust's Result type
// Video explaining the concept: https://www.youtube.com/watch?v=kaI4R0Ng4E8
// Until I start using C++23, I can't use std::expected because it's not available yet,
// so this will be my custom implementation

#pragma once

#include <exception>
#include <stdexcept>
#include <sys/stat.h>

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Core.hpp"

namespace Axle {
    template <typename T>
    class Expected {
    public:
        // Construct from a const lvalue reference to T
        // Copies 'rhs' into the union's m_Value member
        Expected(const T& rhs)
            : m_Value(rhs),
              m_HasVal(true) {}

        // Construct from an rvalue reference to T
        // Moves 'rhs' into the union's m_Value member
        Expected(T&& rhs)
            : m_Value(std::move(rhs)),
              m_HasVal(true) {}

        // Copy constructor (from another Expected<T>)
        Expected(const Expected& rhs)
            : m_HasVal(rhs.m_HasVal) {
            if (m_HasVal)
                // If rhs contains a value, construct m_Value in-place
                // (must use placement new since we need to decide at runtime)
                new (&m_Value) T(rhs.m_Value);
            else
                // Otherwise, construct m_Exception in-place
                new (&m_Exception) std::exception_ptr(rhs.m_Exception);
        }

        // Move constructor (from another Expected<T>)
        Expected(Expected&& rhs)
            : m_HasVal(rhs.m_HasVal) {
            // Same as before
            if (m_HasVal)
                new (&m_Value) T(std::move(rhs.m_Value));
            else
                new (&m_Exception) std::exception_ptr(std::move(rhs.m_Exception));
        }

        ~Expected() {
            // Can't use std::exception_ptr because it doesn't parse
            using std::exception_ptr;
            if (m_HasVal)
                m_Value.~T();
            else
                m_Exception.~exception_ptr();
        }

        /**
         * Creates an Expected from an exception object
         *
         * @param exception The exception object
         *
         * @returns An Expected of type T whith an exception
         * */
        template <typename E>
        static Expected<T> FromException(const E& exception) {
            // The static type of the exception should be the same as the runtime type
            if (typeid(exception) != typeid(E)) {
                throw std::invalid_argument("Slicing detected");
            }
            return FromException(std::exception_ptr(exception));
        }

        /**
         * Creates an Expected from an exception pointer
         *
         * @param p The exception pointer
         *
         * @returns An Expected of type T whith an exception
         * */
        static Expected<T> FromException(std::exception_ptr p) {
            Expected<T> result;
            result.m_HasVal = false;
            new (&result.m_HasVal) std::exception_ptr(std::move(p));
            return result;
        }

        /**
         * Creates an Expected from the current exception
         *
         * If there is no current exception, the behavior is undefined.
         *
         * @returns An Expected of type T whith an exception
         * */
        static Expected<T> FromException() {
            return FromException(std::current_exception());
        }

        /**
         * Swaps the contents of this Expected with another one.
         * It handles all the possible cases.
         * */
        void Swap(Expected& rhs) {
            // The first Expected has a value
            if (m_HasVal) {
                if (rhs.m_HasVal) {
                    // Simply swap values because everything else is the same
                    std::swap(m_Value, rhs.m_Value);
                } else {
                    // It's needed to move the exception from the second Expected to the first one and the value of the
                    // first to the second one
                    std::exception_ptr t = std::move(rhs.m_Exception);
                    new (&rhs.m_Value) T(std::move(m_Value));
                    new (&m_Exception) std::exception_ptr(t);
                    std::swap(m_HasVal, rhs.m_HasVal);
                }
            } else {
                if (rhs.m_HasVal) {
                    // We repeat the process but inverting the order
                    // This is a way of avoiding repetitions by using recursion
                    rhs.Swap(*this);
                } else {
                    // Swap the exceptions
                    m_Exception.swap(rhs.m_Exception);
                    std::swap(m_HasVal, rhs.m_HasVal);
                }
            }
        }

        /**
         * Determines if the Expected contains a valid value
         *
         * @returns true if it contains a valid value, false if it contains an exception
         * */
        bool IsValid() const {
            return m_HasVal;
        }

        /**
         * Unwraps the value contained in the Expected.
         * If the Expected contains an exception, it is rethrown.
         *
         * @returns A reference to the T value contained in the Expected if there is no exception
         * */
        T& Unwrap() {
            if (!m_HasVal)
                std::rethrow_exception(m_Exception);
            return m_Value;
        }

        /**
         * Unwraps the value contained in the Expected.
         * If the Expected contains an exception, it is rethrown.
         *
         * @returns A reference to the T value contained in the Expected if there is no exception
         * */
        const T& Unwrap() const {
            if (!m_HasVal)
                std::rethrow_exception(m_Exception);
            return m_Value;
        }

        /**
         * Checks if the given exception type is the one contained in the Expected.
         * This process is expensive because it requires rethrowing the exception and catching it.
         * So it shouldn't be used in performance critical paths.
         *
         * @returns true if the exception type matches, false otherwise
         * */
        template <typename E>
        bool HasException() const {
            try {
                if (!m_HasVal)
                    std::rethrow_exception(m_Exception);
            } catch (const E& object) {
                return true;
            } catch (...) {}
            return false;
        }

        /**
         * Static helper to create an Expected from a function that may throw.
         *
         * @returns An Expected of type T containing either the return value of the function or the exception
         * */
        template <typename F>
        static Expected FromCode(F func) {
            try {
                return Expected(func());
            } catch (...) {
                return FromException();
            }
        }

    private:
        union {
            T m_Value;
            std::exception_ptr m_Exception;
        };
        // Tells which union member is active
        bool m_HasVal;

        // Private default constructor (used internally)
        Expected() {}
    };
} // namespace Axle
