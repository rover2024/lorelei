// SPDX-License-Identifier: MIT

#include <iterator>
#include <numeric>

#include <lorelei/Support/VarSizeArray.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using lore::VarSizeArray;
using lore::VarSizeArrayBase;

// A value type that counts its construction/destruction/assignment events, modeled on the
// `Constructable` helper in LLVM's unittests/ADT/SmallVectorTest.cpp. It lets the tests assert that
// VarSizeArray manages element lifetimes exactly (no leaks, no double frees, correct move/copy).
namespace {
    struct Tracked {
        static int numConstructed;
        static int numDestroyed;

        int value = 0;

        Tracked() {
            ++numConstructed;
        }
        Tracked(int v) : value(v) {
            ++numConstructed;
        }
        Tracked(const Tracked &o) : value(o.value) {
            ++numConstructed;
        }
        Tracked(Tracked &&o) noexcept : value(o.value) {
            o.value = -1;
            ++numConstructed;
        }
        Tracked &operator=(const Tracked &o) {
            value = o.value;
            return *this;
        }
        Tracked &operator=(Tracked &&o) noexcept {
            value = o.value;
            o.value = -1;
            return *this;
        }
        ~Tracked() {
            ++numDestroyed;
        }

        static void reset() {
            numConstructed = numDestroyed = 0;
        }
        static int alive() {
            return numConstructed - numDestroyed;
        }
    };

    int Tracked::numConstructed = 0;
    int Tracked::numDestroyed = 0;

    // Runs `body` against VarSizeArray inline sizes 0, 1, 2, 4 and 7, the way LLVM's typed tests
    // exercise SmallVector<T, 0/1/2/...>.
#define FOR_EACH_INLINE_SIZE(body)                                                                 \
    body<0>();                                                                                     \
    body<1>();                                                                                     \
    body<2>();                                                                                     \
    body<4>();                                                                                     \
    body<7>()

    template <std::size_t N>
    void checkGrowsAndHolds() {
        VarSizeArray<int, N> v;
        BOOST_TEST(v.capacity() == N);
        for (int i = 0; i < 50; ++i)
            v.push_back(i);
        BOOST_TEST(v.size() == 50u);
        BOOST_TEST(v.capacity() >= 50u);
        for (int i = 0; i < 50; ++i)
            BOOST_TEST(v[i] == i);
    }

    template <std::size_t N>
    void checkInsertErase() {
        VarSizeArray<int, N> v{1, 2, 3};

        v.insert(v.begin(), 0);          // front
        v.insert(v.end(), 9);            // back
        v.insert(v.begin() + 2, 99);     // middle
        // {0, 1, 99, 2, 3, 9}
        BOOST_TEST(v.size() == 6u);
        BOOST_TEST((v[0] == 0 && v[1] == 1 && v[2] == 99 && v[3] == 2 && v[4] == 3 && v[5] == 9));

        v.insert(v.begin() + 1, 3u, 7);  // count form: three 7s after index 0
        // {0, 7, 7, 7, 1, 99, 2, 3, 9}
        BOOST_TEST(v.size() == 9u);
        BOOST_TEST((v[1] == 7 && v[2] == 7 && v[3] == 7 && v[4] == 1));

        int more[] = {100, 200};
        v.insert(v.begin(), more, more + 2); // range form
        BOOST_TEST((v.front() == 100 && v[1] == 200));

        v.erase(v.begin());                       // drop the 100
        BOOST_TEST(v.front() == 200);
        v.erase(v.begin(), v.begin() + 1);        // drop the 200
        BOOST_TEST(v.front() == 0);

        // Erase the three consecutive 7s.
        v.erase(v.begin() + 1, v.begin() + 4);
        BOOST_TEST((v[0] == 0 && v[1] == 1 && v[2] == 99));
    }

    template <std::size_t N>
    void checkSelfReference() {
        // Each of these reads an element that the operation itself may relocate; the value must
        // survive the reallocation. Fill to capacity so the next add reallocates.
        VarSizeArray<int, N> v;
        for (int i = 0; i < static_cast<int>(N) + 3; ++i)
            v.push_back(i);

        int first = v[0];
        v.push_back(v[0]); // push_back of a self reference across a grow
        BOOST_TEST(v.back() == first);

        int second = v[1];
        v.emplace_back(v[1]); // emplace_back of a self reference across a grow
        BOOST_TEST(v.back() == second);

        int mid = v[v.size() / 2];
        v.insert(v.begin(), v[v.size() / 2]); // insert of a self reference
        BOOST_TEST(v.front() == mid);
    }

    template <std::size_t N>
    void checkSwap() {
        VarSizeArray<int, N> a{1, 2, 3};
        VarSizeArray<int, N> b{10, 20, 30, 40, 50};
        a.swap(b);
        BOOST_TEST((a.size() == 5u && a[4] == 50));
        BOOST_TEST((b.size() == 3u && b[2] == 3));

        swap(a, b); // free swap()
        BOOST_TEST((a.size() == 3u && a[0] == 1));
        BOOST_TEST((b.size() == 5u && b[0] == 10));
    }
} // namespace

BOOST_AUTO_TEST_SUITE(test_VarSizeArray)

// --- Basics -----------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(empty_vector) {
    VarSizeArray<int, 4> v;
    BOOST_TEST(v.empty());
    BOOST_TEST(v.size() == 0u);
    BOOST_TEST(v.capacity() == 4u);
    BOOST_TEST(v.begin() == v.end());
}

BOOST_AUTO_TEST_CASE(push_back_and_index) {
    VarSizeArray<int, 4> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    BOOST_TEST(v.size() == 3u);
    BOOST_TEST((v[0] == 10 && v[1] == 20 && v[2] == 30));
    BOOST_TEST((v.front() == 10 && v.back() == 30));
    BOOST_TEST(v.data() == &v[0]);
}

BOOST_AUTO_TEST_CASE(stays_inline_then_spills_to_heap) {
    VarSizeArray<int, 2> v;
    v.push_back(1);
    v.push_back(2);
    const int *inlinePtr = v.data();
    BOOST_TEST(v.capacity() == 2u);

    v.push_back(3);
    BOOST_TEST(v.size() == 3u);
    BOOST_TEST(v.capacity() >= 3u);
    BOOST_TEST(v.data() != inlinePtr);
    BOOST_TEST((v[0] == 1 && v[1] == 2 && v[2] == 3));
}

BOOST_AUTO_TEST_CASE(grows_and_holds_across_inline_sizes) {
    FOR_EACH_INLINE_SIZE(checkGrowsAndHolds);
}

BOOST_AUTO_TEST_CASE(emplace_back_returns_reference_and_builds_in_place) {
    VarSizeArray<Tracked, 2> v;
    Tracked::reset();
    Tracked &ref = v.emplace_back(42);
    BOOST_TEST(ref.value == 42);
    BOOST_TEST(&ref == &v.back());
    BOOST_TEST(Tracked::numConstructed == 1); // no temporary copied/moved
}

BOOST_AUTO_TEST_CASE(resize_grow_shrink_and_value) {
    VarSizeArray<int, 2> v;
    v.resize(5);
    BOOST_TEST(v.size() == 5u);
    for (int x : v)
        BOOST_TEST(x == 0);
    v.resize(2);
    BOOST_TEST(v.size() == 2u);
    v.resize(4, 7);
    BOOST_TEST((v.size() == 4u && v[2] == 7 && v[3] == 7));
}

BOOST_AUTO_TEST_CASE(clear_keeps_capacity) {
    VarSizeArray<int, 2> v{1, 2, 3, 4};
    size_t cap = v.capacity();
    v.clear();
    BOOST_TEST((v.empty() && v.capacity() == cap));
}

BOOST_AUTO_TEST_CASE(iterator_walk) {
    VarSizeArray<int, 2> v{1, 2, 3, 4};
    BOOST_TEST(std::accumulate(v.begin(), v.end(), 0) == 10);
    int expected = 1;
    for (int x : v)
        BOOST_TEST(x == expected++);
}

// --- insert / erase ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(insert_and_erase_across_inline_sizes) {
    FOR_EACH_INLINE_SIZE(checkInsertErase);
}

BOOST_AUTO_TEST_CASE(insert_initializer_list_and_returned_iterator) {
    VarSizeArray<int, 2> v{1, 4};
    auto it = v.insert(v.begin() + 1, {2, 3});
    BOOST_TEST(v.size() == 4u);
    BOOST_TEST((v[0] == 1 && v[1] == 2 && v[2] == 3 && v[3] == 4));
    BOOST_TEST((it == v.begin() + 1 && *it == 2));

    auto e = v.erase(v.begin() + 1);
    BOOST_TEST((*e == 3 && v.size() == 3u));
}

// --- Self-referential operations (the case LLVM specifically guards) ---------------------------

BOOST_AUTO_TEST_CASE(self_referential_operations_survive_realloc) {
    FOR_EACH_INLINE_SIZE(checkSelfReference);
}

// --- swap -------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(swap_across_inline_sizes) {
    FOR_EACH_INLINE_SIZE(checkSwap);
}

BOOST_AUTO_TEST_CASE(swap_heap_with_heap_trades_buffers) {
    VarSizeArray<int, 2> a{1, 2, 3};       // heap
    VarSizeArray<int, 2> b{4, 5, 6, 7};    // heap
    const int *pa = a.data();
    const int *pb = b.data();
    a.swap(b);
    BOOST_TEST((a.data() == pb && b.data() == pa)); // pointers traded, no element moves
    BOOST_TEST((a.size() == 4u && b.size() == 3u));
}

BOOST_AUTO_TEST_CASE(swap_inline_with_heap) {
    VarSizeArray<int, 4> inl{1, 2};            // inline
    VarSizeArray<int, 4> hp{9, 8, 7, 6, 5};    // heap
    inl.swap(hp);
    BOOST_TEST((inl.size() == 5u && inl[4] == 5));
    BOOST_TEST((hp.size() == 2u && hp[0] == 1 && hp[1] == 2));
}

BOOST_AUTO_TEST_CASE(swap_balances_lifetimes) {
    Tracked::reset();
    {
        VarSizeArray<Tracked, 2> a;
        VarSizeArray<Tracked, 2> b;
        for (int i = 0; i < 3; ++i)
            a.emplace_back(i);     // inline -> heap
        b.emplace_back(100);       // inline
        a.swap(b);
        BOOST_TEST((a.size() == 1u && a[0].value == 100));
        BOOST_TEST((b.size() == 3u && b[2].value == 2));
    }
    BOOST_TEST(Tracked::alive() == 0);
}

// --- Copy / move ------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(copy_is_deep) {
    VarSizeArray<int, 2> a{1, 2, 3};
    VarSizeArray<int, 2> b = a;
    b[0] = 99;
    BOOST_TEST((a[0] == 1 && b[0] == 99 && b.size() == 3u));
    VarSizeArray<int, 2> c;
    c = a;
    BOOST_TEST((c.size() == 3u && c[2] == 3));
}

BOOST_AUTO_TEST_CASE(move_steals_heap_buffer) {
    VarSizeArray<int, 2> a{1, 2, 3, 4};
    const int *heap = a.data();
    VarSizeArray<int, 2> b = std::move(a);
    BOOST_TEST((b.size() == 4u && b.data() == heap));
    BOOST_TEST(a.empty());
    BOOST_TEST(b[3] == 4);
}

BOOST_AUTO_TEST_CASE(move_from_inline_transfers_elements) {
    VarSizeArray<int, 4> a{1, 2};
    VarSizeArray<int, 4> b = std::move(a);
    BOOST_TEST((b.size() == 2u && b[0] == 1 && b[1] == 2));
    BOOST_TEST(a.empty());
}

// --- Size-agnostic base -----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(base_reference_is_inline_size_agnostic) {
    auto sum = [](const VarSizeArrayBase<int> &a) {
        int total = 0;
        for (int x : a)
            total += x;
        return total;
    };
    VarSizeArray<int, 2> small{1, 2, 3};
    VarSizeArray<int, 16> big{10, 20};
    BOOST_TEST(sum(small) == 6);
    BOOST_TEST(sum(big) == 30);
}

BOOST_AUTO_TEST_CASE(cross_inline_size_assignment_and_swap) {
    VarSizeArray<int, 8> src{7, 8, 9};
    VarSizeArray<int, 2> dst;
    dst = src; // operator=(const VarSizeArrayBase&)
    BOOST_TEST((dst.size() == 3u && dst[0] == 7 && dst[2] == 9));

    VarSizeArray<int, 2> moved = std::move(src);
    BOOST_TEST((moved.size() == 3u && moved[1] == 8));

    VarSizeArray<int, 4> x{1, 2};
    VarSizeArray<int, 16> y{3, 4, 5};
    static_cast<VarSizeArrayBase<int> &>(x).swap(y); // swap through the common base
    BOOST_TEST((x.size() == 3u && x[2] == 5));
    BOOST_TEST((y.size() == 2u && y[1] == 2));
}

// --- Lifetime accounting ----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(element_lifetimes_are_balanced) {
    Tracked::reset();
    {
        VarSizeArray<Tracked, 2> a;
        for (int i = 0; i < 10; ++i)
            a.emplace_back(i);
        BOOST_TEST((a.size() == 10u && a[9].value == 9 && Tracked::alive() == 10));

        VarSizeArray<Tracked, 2> b(std::move(a));
        VarSizeArray<Tracked, 4> c = b;
        c.insert(c.begin() + 1, Tracked(77));
        c.erase(c.begin());
        BOOST_TEST(Tracked::alive() == 20 + 1 - 1);
    }
    BOOST_TEST(Tracked::alive() == 0); // no leaks or double frees
}

BOOST_AUTO_TEST_CASE(self_assignment_is_a_noop) {
    VarSizeArray<int, 2> v{1, 2, 3};
    VarSizeArrayBase<int> &ref = v;
    ref = ref;
    BOOST_TEST((v.size() == 3u && v[0] == 1 && v[2] == 3));
    ref = std::move(ref);
    BOOST_TEST((v.size() == 3u && v[2] == 3));
}

BOOST_AUTO_TEST_CASE(zero_inline_is_pure_heap) {
    VarSizeArray<int, 0> v;
    BOOST_TEST((v.capacity() == 0u && v.empty()));
    v.push_back(5);
    BOOST_TEST((v.size() == 1u && v[0] == 5 && v.capacity() >= 1u));
}

BOOST_AUTO_TEST_SUITE_END()
