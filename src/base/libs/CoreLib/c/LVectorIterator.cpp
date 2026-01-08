#include "LVectorIterator.h"

#include <vector>
#include <string>

void lore_stringVectorBegin(void *vec, LVectorIterator *it) {
    auto v = (std::vector<std::string> *) vec;
    it->data = vec;
    it->size = v->size();
    it->index = 0;
}

void lore_stringVectorEnd(void *vec, LVectorIterator *it) {
    auto v = (std::vector<std::string> *) vec;
    it->data = vec;
    it->size = v->size();
    it->index = it->size;
}

void lore_stringVectorNext(LVectorIterator *it) {
    it->index++;
}

void lore_stringVectorGet(LVectorIterator *it, char **out) {
    auto v = (std::vector<std::string> *) it->data;
    *out = const_cast<char *>(v->at(it->index).c_str());
}
