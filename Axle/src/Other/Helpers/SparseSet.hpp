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

    template <typename T, size_t N>
    class SparseSet : public ISparseSet {
    public:
        static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

        /**
         * Adds an element of type T to the sparse set with the given ID.
         *
         * @param id The index to add the element to.
         * @param component The element to be added.
         */
        void Add(size_t id, T component) {
            AX_ASSERT(id < N, "Index {0} is out of bounds for SparseSet with size {1}.", id, N);
            AX_ASSERT(!Contains(id), "Can't add an element that already exists.");

            m_Sparse.at(id) = m_Size;
            m_DenseToSparse.at(m_Size) = id;
            m_Dense.at(m_Size) = component;

            m_Size++;
        }

        /**
         * Removes an element of type T from the index specified
         *
         * @param id The index to remove the element from
         */
        void Remove(size_t id) {
            AX_ASSERT(id < N, "Index {0} is out of bounds for SparseSet with size {1}.", id, N);
            AX_ASSERT(Contains(id),
                      "Trying to remove a non-existent element of type {0} from index {1}",
                      typeid(T).name(),
                      id);

            // Swap the back of the dense array with the element to be deleted
            size_t deletedIndex = m_Sparse.at(id);
            std::swap(m_Dense.at(deletedIndex), m_Dense.at(m_Size - 1));

            // Update the sparse array to reflect the change
            size_t entityOfLastElement = m_DenseToSparse.at(m_Size - 1);
            std::swap(m_Sparse.at(entityOfLastElement), m_Sparse.at(id));
            std::swap(m_DenseToSparse.at(deletedIndex), m_DenseToSparse.at(m_Size - 1));

            m_Size--;
        }

        /**
         * Clears the sparse set
         */
        void Clear() {
            m_Size = 0;
            m_Sparse.fill(InvalidIndex);
        }

        /**
         * Gets an element of type T from the given index
         *
         * @param id The index to get the element from
         *
         * @returns A reference to the element of type T
         */
        T& Get(size_t id) const {
            AX_ASSERT(Contains(id),
                      "Trying to retrieve a non-existent element of type: {0} from index {1}",
                      typeid(T).name(),
                      id);

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
            if (Contains(id) && id < N) {
                Remove(id);
            }
        }

        /**
         * Returns the size of the inserted elements.
         *
         * @returns The size of the inserted elements.
         */
        size_t Size() const override {
            return m_Size;
        }

        /**
         * Gets the indexes which have an element assigned.
         * The returned indexes are not sorted.
         *
         * @returns A vector of indexes which are certain to have an element assigned
         */
        std::vector<size_t> GetList() override {
            std::vector<size_t> list;
            list.reserve(m_Size);

            for (size_t i = 0; i < m_Size; i++) {
                list.push_back(m_DenseToSparse.at(i));
            }

            return list;
        }

        /**
         * Checks if an index has an element assigned to it
         *
         * @param id The index to check
         *
         * @returns True if it has an element, False otherwise
         */
        bool Has(size_t id) const override {
            size_t denseIdx = m_Sparse.at(id);
            return denseIdx != InvalidIndex && denseIdx < m_Size && m_DenseToSparse.at(denseIdx) == id;
        }

    private:
        /// A dense array of elements of type T
        std::array<T, N> m_Dense{};

        /// Handy array to convert from dense indexes to sparse indexes
        std::array<size_t, N> m_DenseToSparse{};

        /// Handy array to convert from indexes to dense indexes
        std::array<size_t, N> m_Sparse = [] {
            std::array<size_t, N> arr;
            arr.fill(InvalidIndex);
            return arr;
        }();

        /// Total size of valid entries in the array
        size_t m_Size = 0;
    };

} // namespace Axle