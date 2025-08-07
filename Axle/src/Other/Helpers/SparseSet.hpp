#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Error/Panic.hpp"

namespace Axle {
    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual void RemoveNoPanic(size_t id) = 0;
        virtual size_t Size() const = 0;
        virtual std::vector<size_t> GetList() = 0;
        virtual bool Has(size_t id) const = 0;
    };

    template <typename T>
    class SparseSet : public ISparseSet {
    public:
        static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

        SparseSet() {
            // Some arbitraty size values to avoid frequent reallocations
            m_Dense.reserve(1000);
            m_DenseToSparse.reserve(1000);
            m_Sparse.reserve(1000);
        }

        /**
         * Adds an element of type T to the sparse set with the given ID.
         *
         * @param id The index to add the element to.
         * @param component The element to be added.
         */
        template <typename U>
        void Add(size_t id, U&& component) {
            // AX_ASSERT(id < m_Sparse.size(), "Index {0} is out of bounds for SparseSet with size {1}.", id, N);
            static_assert(std::is_same_v<T, std::decay_t<U>>,
                          "Trying to add a component of incorrect type to SparseSet");
            AX_ASSERT(!Has(id), "Can't add an element that already exists.");

            if (id >= m_Sparse.size()) {
                m_Sparse.resize(id + 1, InvalidIndex);
            }

            m_Sparse.at(id) = m_Dense.size();
            m_DenseToSparse.push_back(id);
            m_Dense.push_back(std::forward<U>(component));
        }

        /**
         * Removes an element of type T from the index specified
         *
         * @param id The index to remove the element from
         */
        void Remove(size_t id) {
            AX_ASSERT(id < m_Sparse.size(), "Index {0} is out of bounds in the SparseSet", id);
            AX_ASSERT(
                Has(id), "Trying to remove a non-existent element of type {0} from index {1}", typeid(T).name(), id);

            // Swap the back of the dense array with the element to be deleted
            size_t deletedIndex = m_Sparse.at(id);

            if (deletedIndex != m_Dense.size() - 1) {
                std::swap(m_Dense.at(deletedIndex), m_Dense.back());

                // Update the sparse array to reflect the change
                // We don't use swap here because when adding another element to the deleted
                // id we will have to rewrite the values either way so preserving old ones is useless
                size_t sparseIdxOfLastElement = m_DenseToSparse.at(m_Dense.size() - 1);
                m_Sparse.at(sparseIdxOfLastElement) = deletedIndex;
                m_DenseToSparse.at(deletedIndex) = sparseIdxOfLastElement;
            }

            m_Dense.pop_back();
            m_DenseToSparse.pop_back();
            m_Sparse.at(id) = InvalidIndex;
        }

        /**
         * Clears the sparse set
         */
        void Clear() {
            m_Dense.clear();
            m_DenseToSparse.clear();
            std::fill(m_Sparse.begin(), m_Sparse.end(), InvalidIndex);
        }

        /**
         * Gets an element of type T from the given index
         *
         * @param id The index to get the element from
         *
         * @returns A reference to the element of type T
         */
        T& Get(size_t id) {
            AX_ASSERT(
                Has(id), "Trying to retrieve a non-existent element of type: {0} from index {1}", typeid(T).name(), id);

            return m_Dense.at(m_Sparse.at(id));
        }

        /**
         * Same as Remove but it doesn't panic. If the request is erroneous it simply
         * ignores it.
         *
         * Although it may seem better to always call this method it's still recommended to deal with
         * errors instead of ignoring them
         *
         * @param id The index to remove the element from
         */
        void RemoveNoPanic(size_t id) override {
            if (Has(id)) {
                Remove(id);
            }
        }

        /**
         * Returns the size of the inserted elements.
         *
         * @returns The size of the inserted elements.
         */
        size_t Size() const override {
            return m_Dense.size();
        }

        /**
         * Gets the indexes which have an element assigned.
         * The returned indexes are not sorted.
         *
         * @returns A vector of indexes which are certain to have an element assigned
         */
        std::vector<size_t> GetList() override {
            return m_DenseToSparse;
        }

        /**
         * Checks if an index has an element assigned to it
         *
         * @param id The index to check
         *
         * @returns True if it has an element, False otherwise
         */
        bool Has(size_t id) const override {
            if (id >= m_Sparse.size()) {
                return false; // Out of bounds
            }

            size_t denseIdx = m_Sparse.at(id);
            return denseIdx != InvalidIndex && denseIdx < m_Dense.size() && m_DenseToSparse.at(denseIdx) == id;
        }

    private:
        /// A dense array of elements of type T
        std::vector<T> m_Dense{};

        /// Handy array to convert dense indexes to sparse indexes
        std::vector<size_t> m_DenseToSparse{};

        /// Handy array to convert indexes to dense indexes
        std::vector<size_t> m_Sparse{};
    };

} // namespace Axle