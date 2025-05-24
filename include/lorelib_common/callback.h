#ifndef LORELIB_COMMON_CALLBACK_H
#define LORELIB_COMMON_CALLBACK_H

struct LoreLib_CallbackContext {
    void **ptr;
    void *org_value;
};

static inline void LoreLib_CallbackContext_Destructor(struct LoreLib_CallbackContext *cc) {
    if (cc->org_value) {
        *cc->ptr = cc->org_value;
    }
}

#endif // LORELIB_COMMON_CALLBACK_H