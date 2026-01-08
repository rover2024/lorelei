#ifndef LORE_BASE_CORELIB_LVECTORITERATOR_H
#define LORE_BASE_CORELIB_LVECTORITERATOR_H

#include <stddef.h>
#include <stdbool.h>

#include <LoreBase/CoreLib/Global.h>

LORE_BEGIN_EXTERN_C

struct LVectorIterator {
    void *data;
    size_t size;
    size_t index;
};

/// Get the C iterator for the beginning of an \c std::vector<std::string>.
LORECORE_EXPORT void lore_stringVectorBegin(void *vec, LVectorIterator *it);

/// Get the C iterator for the end of an \c std::vector<std::string>.
LORECORE_EXPORT void lore_stringVectorEnd(void *vec, LVectorIterator *it);

/// Get the next element of an \c std::vector<std::string>.
LORECORE_EXPORT void lore_stringVectorNext(LVectorIterator *it);

/// Get the current element data of an \c std::vector<std::string>.
LORECORE_EXPORT void lore_stringVectorGet(LVectorIterator *it, char **out);

LORE_END_EXTERN_C

#endif // LORE_BASE_CORELIB_LVECTORITERATOR_H
