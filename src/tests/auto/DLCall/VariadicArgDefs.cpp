#include <lorelei/DLCall/Tools/VariadicArgDefs.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_VariadicArgDefs)

BOOST_AUTO_TEST_CASE(typeid_and_value_roundtrip) {
    int i = 42;
    CVargEntry vi = CVargGet(i);
    BOOST_VERIFY(vi.type == CVargType_Int);
    BOOST_VERIFY(CVargValue(int, vi) == 42);

    unsigned long ul = 7;
    CVargEntry vul = CVargGet(ul);
    BOOST_VERIFY(vul.type == CVargType_ULong);
    BOOST_VERIFY(CVargValue(unsigned long, vul) == 7u);

    double d = 3.5;
    CVargEntry vd = CVargGet(d);
    BOOST_VERIFY(vd.type == CVargType_Double);
    BOOST_VERIFY(CVargValue(double, vd) == 3.5);

    const char *s = "hi";
    CVargEntry vp = CVargGet(s);
    BOOST_VERIFY(vp.type == CVargType_Pointer);
    BOOST_VERIFY(CVargValue(void *, vp) == (void *) s);
}

BOOST_AUTO_TEST_CASE(entry_length_stops_at_void_sentinel) {
    CVargEntry entries[] = {CVargGet(1), CVargGet(2), CVargGet(3), {}};
    BOOST_VERIFY(CVargEntryLength(entries) == 3);

    CVargEntry empty[] = {{}};
    BOOST_VERIFY(CVargEntryLength(empty) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
