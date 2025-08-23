// Expected implementation by Alexandrescu's --> https://www.youtube.com/watch?v=kaI4R0Ng4E8
// Current time: 53:06

#pragma once

#include "axpch.hpp"
#include <exception>
#include <stdexcept>

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

        // We don't create a special constructor for exceptions because it could lead to ambiguos behavior and
        // be surprising for the user.
        // Constructors usually mean "I’m initializing with a value of type T".
        // Exceptions aren’t a value of T, they’re an alternative state. So having a special constructor for exceptions
        // can be surprising for the reader.
        template <typename E>
        static Expected<T> FromException(const E& exception) {
            // The static type of the exception should be the same as the runtime type
            if (typeid(exception) != typeid(E)) {
                throw std::invalid_argument("Slicing detected");
            }
            return FromException(std::exception_ptr(exception));
        }

        static Expected<T> FromException(std::exception_ptr p) {
            Expected<T> result;
            result.m_HasVal = false;
            new (&result.m_HasVal) std::exception_ptr(std::move(p));
            return result;
        }

        static Expected<T> FromException() {
            return FromException(std::current_exception());
        }

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
                    // This is a way of avoiding repetitions
                    rhs.Swap(*this);
                } else {
                    // Swap the exceptions
                    m_Exception.swap(rhs.m_Exception);
                    std::swap(m_HasVal, rhs.m_HasVal);
                }
            }
        }

        bool IsValid() const {
            return m_HasVal;
        }

        T& Get() {
            if (!m_HasVal)
                std::rethrow_exception(m_Exception);
            return m_Value;
        }

        const T& Get() const {
            if (!m_HasVal)
                std::rethrow_exception(m_Exception);
            return m_Value;
        }

        // This is exctremely slow
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
