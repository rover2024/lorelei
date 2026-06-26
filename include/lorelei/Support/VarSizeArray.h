// SPDX-License-Identifier: MIT

#ifndef LORE_SUPPORT_VARSIZEARRAY_H
#define LORE_SUPPORT_VARSIZEARRAY_H

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace lore {

    /// VarSizeArrayBase - The size-agnostic base of VarSizeArray.
    ///
    /// Owns the pointer/size/capacity and the allocator, but not the inline buffer, so a single
    /// \c VarSizeArrayBase<T> & can refer to a \c VarSizeArray<T, N> of any inline size N (the role
    /// LLVM's \c SmallVectorImpl plays for \c SmallVector). Functions should take this type by
    /// reference.
    ///
    /// It starts out pointing at the derived object's inline buffer (registered via
    /// \a adoptInlineBuffer) and only touches the allocator once the data outgrows that buffer.
    template <class T, class Alloc = std::allocator<T>>
    class VarSizeArrayBase {
        using AT = std::allocator_traits<Alloc>;

    public:
        using value_type = T;
        using allocator_type = Alloc;
        using size_type = std::size_t;
        using reference = T &;
        using const_reference = const T &;
        using pointer = T *;
        using const_pointer = const T *;
        using iterator = T *;
        using const_iterator = const T *;

        VarSizeArrayBase(const VarSizeArrayBase &) = delete;
        VarSizeArrayBase(VarSizeArrayBase &&) = delete;

        VarSizeArrayBase &operator=(const VarSizeArrayBase &RHS) {
            assign(RHS);
            return *this;
        }
        VarSizeArrayBase &operator=(VarSizeArrayBase &&RHS) {
            assign(std::move(RHS));
            return *this;
        }

        // --- Capacity -------------------------------------------------------------------------

        [[nodiscard]] bool empty() const {
            return m_size == 0;
        }
        size_type size() const {
            return m_size;
        }
        size_type capacity() const {
            return m_capacity;
        }
        const Alloc &get_allocator() const {
            return m_alloc;
        }

        /// Ensures room for at least \c n elements without changing the size.
        void reserve(size_type n) {
            if (n > m_capacity)
                grow(n);
        }

        // --- Element access -------------------------------------------------------------------

        reference operator[](size_type i) {
            return m_begin[i];
        }
        const_reference operator[](size_type i) const {
            return m_begin[i];
        }
        reference front() {
            return m_begin[0];
        }
        const_reference front() const {
            return m_begin[0];
        }
        reference back() {
            return m_begin[m_size - 1];
        }
        const_reference back() const {
            return m_begin[m_size - 1];
        }
        pointer data() {
            return m_begin;
        }
        const_pointer data() const {
            return m_begin;
        }

        iterator begin() {
            return m_begin;
        }
        const_iterator begin() const {
            return m_begin;
        }
        iterator end() {
            return m_begin + m_size;
        }
        const_iterator end() const {
            return m_begin + m_size;
        }

        // --- Adding elements ------------------------------------------------------------------

        void push_back(const T &value) {
            emplace_back(value);
        }
        void push_back(T &&value) {
            emplace_back(std::move(value));
        }

        /// Constructs an element in place.
        ///
        /// Safe even when the arguments alias an existing element (e.g. \c emplace_back(v[0])): on a
        /// reallocation the new element is built first, while the old buffer is still alive.
        template <class... Args>
        reference emplace_back(Args &&...args) {
            if (m_size == m_capacity) {
                growAndEmplaceBack(std::forward<Args>(args)...);
            } else {
                AT::construct(m_alloc, m_begin + m_size, std::forward<Args>(args)...);
                ++m_size;
            }
            return back();
        }

        template <class InputIt>
        void append(InputIt first, InputIt last) {
            for (; first != last; ++first)
                push_back(*first);
        }

        // --- Inserting at a position ----------------------------------------------------------
        //
        // Each insert appends the new element(s) at the end and then rotates them into place. The
        // value is copied before any shifting happens, so inserting an element that lives inside the
        // array (e.g. \c v.insert(v.begin(), v[3])) is well defined.

        iterator insert(const_iterator pos, const T &value) {
            size_type index = static_cast<size_type>(pos - m_begin);
            push_back(value);
            std::rotate(m_begin + index, m_begin + (m_size - 1), m_begin + m_size);
            return m_begin + index;
        }
        iterator insert(const_iterator pos, T &&value) {
            size_type index = static_cast<size_type>(pos - m_begin);
            push_back(std::move(value));
            std::rotate(m_begin + index, m_begin + (m_size - 1), m_begin + m_size);
            return m_begin + index;
        }

        iterator insert(const_iterator pos, size_type count, const T &value) {
            size_type index = static_cast<size_type>(pos - m_begin);
            if (count == 0)
                return m_begin + index;
            T copy(value); // independent of the array: reserve below may relocate `value`
            reserve(m_size + count);
            for (size_type i = 0; i < count; ++i, ++m_size)
                AT::construct(m_alloc, m_begin + m_size, copy);
            std::rotate(m_begin + index, m_begin + (m_size - count), m_begin + m_size);
            return m_begin + index;
        }

        template <class InputIt, class = std::enable_if_t<!std::is_integral_v<InputIt>>>
        iterator insert(const_iterator pos, InputIt first, InputIt last) {
            size_type index = static_cast<size_type>(pos - m_begin);
            size_type oldSize = m_size;
            append(first, last);
            std::rotate(m_begin + index, m_begin + oldSize, m_begin + m_size);
            return m_begin + index;
        }

        iterator insert(const_iterator pos, std::initializer_list<T> init) {
            return insert(pos, init.begin(), init.end());
        }

        // --- Removing elements ----------------------------------------------------------------

        void pop_back() {
            --m_size;
            AT::destroy(m_alloc, m_begin + m_size);
        }

        iterator erase(const_iterator pos) {
            size_type index = static_cast<size_type>(pos - m_begin);
            std::move(m_begin + index + 1, m_begin + m_size, m_begin + index);
            pop_back();
            return m_begin + index;
        }

        iterator erase(const_iterator first, const_iterator last) {
            size_type from = static_cast<size_type>(first - m_begin);
            size_type to = static_cast<size_type>(last - m_begin);
            if (from == to)
                return m_begin + from;
            std::move(m_begin + to, m_begin + m_size, m_begin + from);
            size_type removed = to - from;
            destroyRange(m_size - removed, m_size);
            m_size -= removed;
            return m_begin + from;
        }

        /// Destroys every element but keeps the current buffer.
        void clear() {
            destroyRange(0, m_size);
            m_size = 0;
        }

        void resize(size_type n) {
            if (n < m_size) {
                destroyRange(n, m_size);
            } else if (n > m_size) {
                reserve(n);
                for (; m_size < n; ++m_size)
                    AT::construct(m_alloc, m_begin + m_size);
            }
            m_size = n;
        }
        void resize(size_type n, const T &value) {
            if (n < m_size) {
                destroyRange(n, m_size);
            } else if (n > m_size) {
                reserve(n);
                for (; m_size < n; ++m_size)
                    AT::construct(m_alloc, m_begin + m_size, value);
            }
            m_size = n;
        }

        // --- Swap -----------------------------------------------------------------------------

        /// Swaps contents with \c RHS. Two heap-backed arrays just trade buffers; otherwise the
        /// shared elements are swapped and the longer one's tail is moved over, since neither can
        /// trade away its own inline buffer.
        void swap(VarSizeArrayBase &RHS) {
            if (this == &RHS)
                return;

            if (!isInline() && !RHS.isInline()) {
                std::swap(m_begin, RHS.m_begin);
                std::swap(m_size, RHS.m_size);
                std::swap(m_capacity, RHS.m_capacity);
                return;
            }

            reserve(RHS.m_size);
            RHS.reserve(m_size);

            size_type shared = std::min(m_size, RHS.m_size);
            for (size_type i = 0; i < shared; ++i)
                std::swap(m_begin[i], RHS.m_begin[i]);

            VarSizeArrayBase &longer = (m_size >= RHS.m_size) ? *this : RHS;
            VarSizeArrayBase &shorter = (m_size >= RHS.m_size) ? RHS : *this;
            for (size_type i = shared; i < longer.m_size; ++i)
                AT::construct(shorter.m_alloc, shorter.m_begin + i, std::move(longer.m_begin[i]));

            size_type longerSize = longer.m_size;
            longer.destroyRange(shared, longer.m_size);
            longer.m_size = shared;
            shorter.m_size = longerSize;
        }

    protected:
        explicit VarSizeArrayBase(const Alloc &alloc) : m_alloc(alloc) {
        }

        // Frees the heap buffer (if any) on the way out; derived adds no owning members.
        ~VarSizeArrayBase() {
            destroyRange(0, m_size);
            if (!isInline())
                AT::deallocate(m_alloc, m_begin, m_capacity);
        }

        /// Registers the derived object's inline buffer. Call once, right after construction.
        void adoptInlineBuffer(T *buffer, size_type capacity) {
            m_begin = buffer;
            m_capacity = capacity;
            m_inlineBegin = buffer;
            m_inlineCapacity = capacity;
        }

        void assign(const VarSizeArrayBase &RHS) {
            if (this == &RHS)
                return;
            clear();
            reserve(RHS.m_size);
            for (m_size = 0; m_size < RHS.m_size; ++m_size)
                AT::construct(m_alloc, m_begin + m_size, RHS.m_begin[m_size]);
        }

        void assign(VarSizeArrayBase &&RHS) {
            if (this == &RHS)
                return;
            resetToInline();
            if (!RHS.isInline()) {
                // Steal the heap buffer outright; its elements come along with it.
                m_begin = RHS.m_begin;
                m_size = RHS.m_size;
                m_capacity = RHS.m_capacity;
                RHS.m_begin = RHS.m_inlineBegin;
                RHS.m_capacity = RHS.m_inlineCapacity;
                RHS.m_size = 0;
            } else {
                reserve(RHS.m_size);
                for (m_size = 0; m_size < RHS.m_size; ++m_size)
                    AT::construct(m_alloc, m_begin + m_size, std::move(RHS.m_begin[m_size]));
                RHS.clear();
            }
        }

    private:
        bool isInline() const {
            return m_begin == m_inlineBegin;
        }

        void destroyRange(size_type from, size_type to) {
            for (size_type i = from; i < to; ++i)
                AT::destroy(m_alloc, m_begin + i);
        }

        size_type computeNewCapacity(size_type minCapacity) const {
            size_type c = m_capacity * 2;
            if (c < minCapacity)
                c = minCapacity;
            if (c < kMinHeapCapacity)
                c = kMinHeapCapacity;
            return c;
        }

        // Move-relocates [0, m_size) into dst and destroys the originals.
        void relocateTo(T *dst) {
            for (size_type i = 0; i < m_size; ++i) {
                AT::construct(m_alloc, dst + i, std::move_if_noexcept(m_begin[i]));
                AT::destroy(m_alloc, m_begin + i);
            }
        }

        // Grows for reserve(): no new element, the elements just move to a bigger buffer.
        void grow(size_type minCapacity) {
            size_type newCapacity = computeNewCapacity(minCapacity);
            T *newBuffer = AT::allocate(m_alloc, newCapacity);
            relocateTo(newBuffer);
            if (!isInline())
                AT::deallocate(m_alloc, m_begin, m_capacity);
            m_begin = newBuffer;
            m_capacity = newCapacity;
        }

        // Grows and appends. The new element is constructed first, before the old buffer is touched,
        // so arguments that reference an existing element stay valid.
        template <class... Args>
        void growAndEmplaceBack(Args &&...args) {
            size_type newCapacity = computeNewCapacity(m_size + 1);
            T *newBuffer = AT::allocate(m_alloc, newCapacity);
            AT::construct(m_alloc, newBuffer + m_size, std::forward<Args>(args)...);
            relocateTo(newBuffer);
            if (!isInline())
                AT::deallocate(m_alloc, m_begin, m_capacity);
            m_begin = newBuffer;
            m_capacity = newCapacity;
            ++m_size;
        }

        // Destroys the elements and releases any heap buffer, returning to the inline buffer.
        void resetToInline() {
            destroyRange(0, m_size);
            if (!isInline()) {
                AT::deallocate(m_alloc, m_begin, m_capacity);
                m_begin = m_inlineBegin;
                m_capacity = m_inlineCapacity;
            }
            m_size = 0;
        }

        static constexpr size_type kMinHeapCapacity = 4;

        T *m_begin = nullptr;
        size_type m_size = 0;
        size_type m_capacity = 0;
        T *m_inlineBegin = nullptr;
        size_type m_inlineCapacity = 0;
        [[no_unique_address]] Alloc m_alloc;
    };

    template <class T, class Alloc>
    void swap(VarSizeArrayBase<T, Alloc> &lhs, VarSizeArrayBase<T, Alloc> &RHS) {
        lhs.swap(RHS);
    }

    /// VarSizeArray - A dynamic array with N elements of inline (pre-allocated) storage.
    ///
    /// Behaves like a small \c std::vector that stays off the heap until it holds more than N
    /// elements. All the behavior lives in \c VarSizeArrayBase<T, Alloc>; this layer only adds the
    /// inline buffer, so a \c VarSizeArray<T, N> binds to \c VarSizeArrayBase<T> & regardless of N.
    template <class T, std::size_t N = 4, class Alloc = std::allocator<T>>
    class VarSizeArray : public VarSizeArrayBase<T, Alloc> {
        using Base = VarSizeArrayBase<T, Alloc>;

    public:
        VarSizeArray() : VarSizeArray(Alloc()) {
        }
        explicit VarSizeArray(const Alloc &alloc) : Base(alloc) {
            this->adoptInlineBuffer(inlineData(), N);
        }

        VarSizeArray(const VarSizeArray &RHS) : VarSizeArray(RHS.get_allocator()) {
            this->assign(RHS);
        }
        VarSizeArray(VarSizeArray &&RHS) noexcept : VarSizeArray(RHS.get_allocator()) {
            this->assign(std::move(RHS));
        }

        // Cross-size construction: accept any VarSizeArray<T, M> through the common base.
        VarSizeArray(const Base &RHS) : VarSizeArray(RHS.get_allocator()) {
            this->assign(RHS);
        }
        VarSizeArray(Base &&RHS) : VarSizeArray(RHS.get_allocator()) {
            this->assign(std::move(RHS));
        }

        VarSizeArray(std::initializer_list<T> init, const Alloc &alloc = Alloc())
            : VarSizeArray(alloc) {
            this->append(init.begin(), init.end());
        }

        template <class InputIt>
        VarSizeArray(InputIt first, InputIt last, const Alloc &alloc = Alloc())
            : VarSizeArray(alloc) {
            this->append(first, last);
        }

        VarSizeArray &operator=(const VarSizeArray &RHS) {
            this->assign(RHS);
            return *this;
        }
        VarSizeArray &operator=(VarSizeArray &&RHS) noexcept {
            this->assign(std::move(RHS));
            return *this;
        }
        VarSizeArray &operator=(const Base &RHS) {
            this->assign(RHS);
            return *this;
        }
        VarSizeArray &operator=(Base &&RHS) {
            this->assign(std::move(RHS));
            return *this;
        }
        VarSizeArray &operator=(std::initializer_list<T> init) {
            this->clear();
            this->append(init.begin(), init.end());
            return *this;
        }

    private:
        T *inlineData() {
            return reinterpret_cast<T *>(m_buffer);
        }

        // Raw, uninitialized storage for N elements (at least one byte so N == 0 stays valid).
        alignas(T) unsigned char m_buffer[N ? N * sizeof(T) : 1];
    };

}

#endif // LORE_SUPPORT_VARSIZEARRAY_H
