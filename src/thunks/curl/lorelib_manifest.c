#include <curl/curl.h>

#include <lorelei/loreshared.h>
#include <lorelei/loreuser.h>

#include <lorelib_common/manifest-predef.h>
#include <lorelib_common/callback.h>



//
// Option macros
//
#define LORELIB_GCB_AUTO_DEPTH 1
#define LORELIB_HCB_AUTO_DEPTH 1
#define LORELIB_CALLBACK_REPLACE



//
// Annotations
//
#ifdef LORELIB_VISUAL

static int __HINT_GCB_curl_progress_callback(void *clientp, double dltotal, double dlnow,
                                             double ultotal, double ulnow);
static int __HINT_GCB_curl_xferinfo_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                                             curl_off_t ultotal, curl_off_t ulnow);
static size_t __HINT_GCB_curl_write_callback(char *buffer, size_t size, size_t nitems,
                                             void *outstream);
static int __HINT_GCB_curl_resolver_start_callback(void *resolver_state, void *reserved,
                                                   void *userdata);
static long __HINT_GCB_curl_chunk_bgn_callback(const void *transfer_info, void *ptr, int remains);
static long __HINT_GCB_curl_chunk_end_callback(void *ptr);
static int __HINT_GCB_curl_fnmatch_callback(void *ptr, const char *pattern, const char *string);
static int __HINT_GCB_curl_seek_callback(void *instream, curl_off_t offset, int origin);
static size_t __HINT_GCB_curl_read_callback(char *buffer, size_t size, size_t nitems,
                                            void *instream);
static int __HINT_GCB_curl_trailer_callback(struct curl_slist **list, void *userdata);
static int __HINT_GCB_curl_sockopt_callback(void *clientp, curl_socket_t curlfd,
                                            curlsocktype purpose);
static curl_socket_t __HINT_GCB_curl_opensocket_callback(void *clientp, curlsocktype purpose,
                                                         struct curl_sockaddr *address);
static int __HINT_GCB_curl_closesocket_callback(void *clientp, curl_socket_t item);
static curlioerr __HINT_GCB_curl_ioctl_callback(CURL *handle, int cmd, void *clientp);
static void *__HINT_GCB_curl_malloc_callback(size_t size);
static void __HINT_GCB_curl_free_callback(void *ptr);
static void *__HINT_GCB_curl_realloc_callback(void *ptr, size_t size);
static char *__HINT_GCB_curl_strdup_callback(const char *str);
static void *__HINT_GCB_curl_calloc_callback(size_t nmemb, size_t size);
static CURLcode __HINT_GCB_curl_conv_callback(char *buffer, size_t length);
static CURLcode __HINT_GCB_curl_ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *userptr);
static size_t __HINT_GCB_curl_formget_callback(void *arg, const char *buf, size_t len);

static int __HINT_GCB_curl_debug_callback(CURL *handle, /* the handle/transfer this concerns */
                                          curl_infotype type, /* what kind of data */
                                          char *data,         /* points to the data */
                                          size_t size,        /* size of the data pointed to */
                                          void *userptr);     /* whatever the user please */
static int __HINT_GCB_curl_sshkeycallback(CURL *easy,         /* easy handle */
                                          const struct curl_khkey *knownkey, /* known */
                                          const struct curl_khkey *foundkey, /* found */
                                          enum curl_khmatch, /* libcurl's view on the keys */
                                          void *clientp);    /* custom pointer passed from app */

static void __HINT_GCB_curl_lock_function(CURL *handle, curl_lock_data data,
                                          curl_lock_access locktype, void *userptr);
static void __HINT_GCB_curl_unlock_function(CURL *handle, curl_lock_data data, void *userptr);

static int __HINT_GCB_curl_socket_callback(CURL *easy,           /* easy handle */
                                           curl_socket_t s,      /* socket */
                                           int what,             /* see above */
                                           void *userp,          /* private callback
                                                                    pointer */
                                           void *socketp);       /* private socket
                                                                    pointer */
static int __HINT_GCB_curl_multi_timer_callback(CURLM *multi,    /* multi handle */
                                                long timeout_ms, /* see above */
                                                void *userp);    /* private callback
                                                                    pointer */
static int __HINT_GCB_curl_push_callback(CURL *parent, CURL *easy, size_t num_headers,
                                         struct curl_pushheaders *headers, void *userp);



static int __HINT_HCB_curl_progress_callback(void *clientp, double dltotal, double dlnow,
                                             double ultotal, double ulnow);
static int __HINT_HCB_curl_xferinfo_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                                             curl_off_t ultotal, curl_off_t ulnow);
static size_t __HINT_HCB_curl_write_callback(char *buffer, size_t size, size_t nitems,
                                             void *outstream);
static int __HINT_HCB_curl_resolver_start_callback(void *resolver_state, void *reserved,
                                                   void *userdata);
static long __HINT_HCB_curl_chunk_bgn_callback(const void *transfer_info, void *ptr, int remains);
static long __HINT_HCB_curl_chunk_end_callback(void *ptr);
static int __HINT_HCB_curl_fnmatch_callback(void *ptr, const char *pattern, const char *string);
static int __HINT_HCB_curl_seek_callback(void *instream, curl_off_t offset, int origin);
static size_t __HINT_HCB_curl_read_callback(char *buffer, size_t size, size_t nitems,
                                            void *instream);
static int __HINT_HCB_curl_trailer_callback(struct curl_slist **list, void *userdata);
static int __HINT_HCB_curl_sockopt_callback(void *clientp, curl_socket_t curlfd,
                                            curlsocktype purpose);
static curl_socket_t __HINT_HCB_curl_opensocket_callback(void *clientp, curlsocktype purpose,
                                                         struct curl_sockaddr *address);
static int __HINT_HCB_curl_closesocket_callback(void *clientp, curl_socket_t item);
static curlioerr __HINT_HCB_curl_ioctl_callback(CURL *handle, int cmd, void *clientp);
static void *__HINT_HCB_curl_malloc_callback(size_t size);
static void __HINT_HCB_curl_free_callback(void *ptr);
static void *__HINT_HCB_curl_realloc_callback(void *ptr, size_t size);
static char *__HINT_HCB_curl_strdup_callback(const char *str);
static void *__HINT_HCB_curl_calloc_callback(size_t nmemb, size_t size);
static CURLcode __HINT_HCB_curl_conv_callback(char *buffer, size_t length);
static CURLcode __HINT_HCB_curl_ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *userptr);
static size_t __HINT_HCB_curl_formget_callback(void *arg, const char *buf, size_t len);

static int __HINT_HCB_curl_debug_callback(CURL *handle, /* the handle/transfer this concerns */
                                          curl_infotype type, /* what kind of data */
                                          char *data,         /* points to the data */
                                          size_t size,        /* size of the data pointed to */
                                          void *userptr);     /* whatever the user please */
static int __HINT_HCB_curl_sshkeycallback(CURL *easy,         /* easy handle */
                                          const struct curl_khkey *knownkey, /* known */
                                          const struct curl_khkey *foundkey, /* found */
                                          enum curl_khmatch, /* libcurl's view on the keys */
                                          void *clientp);    /* custom pointer passed from app */

static void __HINT_HCB_curl_lock_function(CURL *handle, curl_lock_data data,
                                          curl_lock_access locktype, void *userptr);
static void __HINT_HCB_curl_unlock_function(CURL *handle, curl_lock_data data, void *userptr);

static int __HINT_HCB_curl_socket_callback(CURL *easy,           /* easy handle */
                                           curl_socket_t s,      /* socket */
                                           int what,             /* see above */
                                           void *userp,          /* private callback
                                                                    pointer */
                                           void *socketp);       /* private socket
                                                                    pointer */
static int __HINT_HCB_curl_multi_timer_callback(CURLM *multi,    /* multi handle */
                                                long timeout_ms, /* see above */
                                                void *userp);    /* private callback
                                                                    pointer */
static int __HINT_HCB_curl_push_callback(CURL *parent, CURL *easy, size_t num_headers,
                                         struct curl_pushheaders *headers, void *userp);

#endif



//
// Custom(Guest)
//
#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)

static CURLcode ___GTP_curl_easy_getinfo(CURL *arg1, CURLINFO arg2, void *arg3, void *arg4,
                                         void *arg5, void *arg6) {
    CURLcode ret;
    void *args[] = {
        (void *) &arg1, (void *) &arg2, (void *) &arg3,
        (void *) &arg4, (void *) &arg5, (void *) &arg6,
    };
    LORELIB_INVOKE_HTP(LORELIB_HTP(curl_easy_getinfo), args, &ret, NULL);
    return ret;
}

static CURLcode __GTP_curl_easy_getinfo(CURL *arg1, CURLINFO arg2, ...) {
    CURLcode ret;
    va_list ap;
    va_start(ap, arg2);
    void *arg3 = va_arg(ap, void *);
    void *arg4 = va_arg(ap, void *);
    void *arg5 = va_arg(ap, void *);
    void *arg6 = va_arg(ap, void *);
    ret = ___GTP_curl_easy_getinfo(arg1, arg2, arg3, arg4, arg5, arg6);
    va_end(ap);
    return ret;
}

static CURLcode ___GTP_curl_easy_setopt(CURL *arg1, CURLoption arg2, void *arg3) {
    CURLcode ret;
    void *args[] = {
        (void *) &arg1,
        (void *) &arg2,
        (void *) &arg3,
    };
    LORELIB_INVOKE_HTP(LORELIB_HTP(curl_easy_setopt), args, &ret, NULL);
    return ret;
}

static CURLcode __GTP_curl_easy_setopt(CURL *arg1, CURLoption arg2, ...) {
    CURLcode ret;
    va_list ap;
    va_start(ap, arg2);
    void *arg3 = va_arg(ap, void *);
    ret = ___GTP_curl_easy_setopt(arg1, arg2, arg3);
    va_end(ap);
    return ret;
}

static CURLSHcode ___GTP_curl_share_setopt(CURLSH *arg1, CURLSHoption arg2, void *arg3) {
    CURLSHcode ret;
    void *args[] = {
        (void *) &arg1,
        (void *) &arg2,
        (void *) &arg3,
    };
    LORELIB_INVOKE_HTP(LORELIB_HTP(curl_share_setopt), args, &ret, NULL);
    return ret;
}

static CURLSHcode __GTP_curl_share_setopt(CURLSH *arg1, CURLSHoption arg2, ...) {
    CURLSHcode ret;
    va_list ap;
    va_start(ap, arg2);
    void *arg3 = va_arg(ap, void *);
    ret = ___GTP_curl_share_setopt(arg1, arg2, arg3);
    va_end(ap);
    return ret;
}

static CURLMcode ___GTP_curl_multi_setopt(CURLM *arg1, CURLMoption arg2, void *arg3) {
    CURLMcode ret;
    void *args[] = {
        (void *) &arg1,
        (void *) &arg2,
        (void *) &arg3,
    };
    LORELIB_INVOKE_HTP(LORELIB_HTP(curl_multi_setopt), args, &ret, NULL);
    return ret;
}

static CURLMcode __GTP_curl_multi_setopt(CURLM *arg1, CURLMoption arg2, ...) {
    CURLMcode ret;
    va_list ap;
    va_start(ap, arg2);
    void *arg3 = va_arg(ap, void *);
    ret = ___GTP_curl_multi_setopt(arg1, arg2, arg3);
    va_end(ap);
    return ret;
}

#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)

static CURLcode ___HTP_curl_easy_getinfo(CURL *arg1, CURLINFO arg2, void *arg3, void *arg4,
                                         void *arg5, void *arg6) {
    CURLcode ret;
    ret = LORELIB_API(curl_easy_getinfo)(arg1, arg2, arg3, arg4, arg5, arg6);
    return ret;
}

static void __HTP_curl_easy_getinfo(void **args, void *ret, void **metadata) {
    __auto_type arg1 = *(CURL **) args[0];
    __auto_type arg2 = *(CURLINFO *) args[1];
    __auto_type arg3 = *(void **) args[2];
    __auto_type arg4 = *(void **) args[3];
    __auto_type arg5 = *(void **) args[4];
    __auto_type arg6 = *(void **) args[5];
    __auto_type ret_ref = (CURLcode *) ret;
    *ret_ref = ___HTP_curl_easy_getinfo(arg1, arg2, arg3, arg4, arg5, arg6);
}

#  ifdef __clang__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wundefined-internal"
#  endif
static int __HTP_GCB_curl_progress_callback(void *clientp, double dltotal, double dlnow,
                                            double ultotal, double ulnow);
static int __HTP_GCB_curl_xferinfo_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                                            curl_off_t ultotal, curl_off_t ulnow);
static size_t __HTP_GCB_curl_write_callback(char *buffer, size_t size, size_t nitems,
                                            void *outstream);
static int __HTP_GCB_curl_resolver_start_callback(void *resolver_state, void *reserved,
                                                  void *userdata);
static long __HTP_GCB_curl_chunk_bgn_callback(const void *transfer_info, void *ptr, int remains);
static long __HTP_GCB_curl_chunk_end_callback(void *ptr);
static int __HTP_GCB_curl_fnmatch_callback(void *ptr, const char *pattern, const char *string);
static int __HTP_GCB_curl_seek_callback(void *instream, curl_off_t offset, int origin);
static size_t __HTP_GCB_curl_read_callback(char *buffer, size_t size, size_t nitems,
                                           void *instream);
static int __HTP_GCB_curl_trailer_callback(struct curl_slist **list, void *userdata);
static int __HTP_GCB_curl_sockopt_callback(void *clientp, curl_socket_t curlfd,
                                           curlsocktype purpose);
static curl_socket_t __HTP_GCB_curl_opensocket_callback(void *clientp, curlsocktype purpose,
                                                        struct curl_sockaddr *address);
static int __HTP_GCB_curl_closesocket_callback(void *clientp, curl_socket_t item);
static curlioerr __HTP_GCB_curl_ioctl_callback(CURL *handle, int cmd, void *clientp);
static void *__HTP_GCB_curl_malloc_callback(size_t size);
static void __HTP_GCB_curl_free_callback(void *ptr);
static void *__HTP_GCB_curl_realloc_callback(void *ptr, size_t size);
static char *__HTP_GCB_curl_strdup_callback(const char *str);
static void *__HTP_GCB_curl_calloc_callback(size_t nmemb, size_t size);
static CURLcode __HTP_GCB_curl_conv_callback(char *buffer, size_t length);
static CURLcode __HTP_GCB_curl_ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *userptr);
static size_t __HTP_GCB_curl_formget_callback(void *arg, const char *buf, size_t len);

static int __HTP_GCB_curl_debug_callback(CURL *handle,       /* the handle/transfer this concerns */
                                         curl_infotype type, /* what kind of data */
                                         char *data,         /* points to the data */
                                         size_t size,        /* size of the data pointed to */
                                         void *userptr);     /* whatever the user please */
static int __HTP_GCB_curl_sshkeycallback(CURL *easy,         /* easy handle */
                                         const struct curl_khkey *knownkey, /* known */
                                         const struct curl_khkey *foundkey, /* found */
                                         enum curl_khmatch, /* libcurl's view on the keys */
                                         void *clientp);    /* custom pointer passed from app */

static void __HTP_GCB_curl_lock_function(CURL *handle, curl_lock_data data,
                                         curl_lock_access locktype, void *userptr);
static void __HTP_GCB_curl_unlock_function(CURL *handle, curl_lock_data data, void *userptr);

static int __HTP_GCB_curl_socket_callback(CURL *easy,           /* easy handle */
                                          curl_socket_t s,      /* socket */
                                          int what,             /* see above */
                                          void *userp,          /* private callback
                                                                   pointer */
                                          void *socketp);       /* private socket
                                                                   pointer */
static int __HTP_GCB_curl_multi_timer_callback(CURLM *multi,    /* multi handle */
                                               long timeout_ms, /* see above */
                                               void *userp);    /* private callback
                                                                   pointer */
static int __HTP_GCB_curl_push_callback(CURL *parent, CURL *easy, size_t num_headers,
                                        struct curl_pushheaders *headers, void *userp);
#  ifdef __clang__
#    pragma GCC diagnostic pop
#  endif

LORELIB_GCB_THUNK_ALLOCATOR(curl_calloc_callback_alloc, __HTP_GCB_curl_calloc_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_chunk_bgn_callback_alloc, __HTP_GCB_curl_chunk_bgn_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_chunk_end_callback_alloc, __HTP_GCB_curl_chunk_end_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_closesocket_callback_alloc, __HTP_GCB_curl_closesocket_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_conv_callback_alloc, __HTP_GCB_curl_conv_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_fnmatch_callback_alloc, __HTP_GCB_curl_fnmatch_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_formget_callback_alloc, __HTP_GCB_curl_formget_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_free_callback_alloc, __HTP_GCB_curl_free_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_ioctl_callback_alloc, __HTP_GCB_curl_ioctl_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_malloc_callback_alloc, __HTP_GCB_curl_malloc_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_opensocket_callback_alloc, __HTP_GCB_curl_opensocket_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_progress_callback_alloc, __HTP_GCB_curl_progress_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_read_callback_alloc, __HTP_GCB_curl_read_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_realloc_callback_alloc, __HTP_GCB_curl_realloc_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_resolver_start_callback_alloc,
                            __HTP_GCB_curl_resolver_start_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_seek_callback_alloc, __HTP_GCB_curl_seek_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_sockopt_callback_alloc, __HTP_GCB_curl_sockopt_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_ssl_ctx_callback_alloc, __HTP_GCB_curl_ssl_ctx_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_strdup_callback_alloc, __HTP_GCB_curl_strdup_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_trailer_callback_alloc, __HTP_GCB_curl_trailer_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_write_callback_alloc, __HTP_GCB_curl_write_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_xferinfo_callback_alloc, __HTP_GCB_curl_xferinfo_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_debug_callback_alloc, __HTP_GCB_curl_debug_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_sshkeycallback_alloc, __HTP_GCB_curl_sshkeycallback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_lock_function_alloc, __HTP_GCB_curl_lock_function)
LORELIB_GCB_THUNK_ALLOCATOR(curl_unlock_function_alloc, __HTP_GCB_curl_unlock_function)
LORELIB_GCB_THUNK_ALLOCATOR(curl_socket_callback_alloc, __HTP_GCB_curl_socket_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_multi_timer_callback_alloc, __HTP_GCB_curl_multi_timer_callback)
LORELIB_GCB_THUNK_ALLOCATOR(curl_push_callback_alloc, __HTP_GCB_curl_push_callback)

enum {
    _CURLOPT_HSTSREADFUNCTION = 20000 + 301,
    _CURLOPT_HSTSWRITEFUNCTION = 20000 + 303,
    _CURLOPT_PREREQFUNCTION = 20000 + 312,
    _CURLOPT_SSH_HOSTKEYFUNCTION = 20000 + 316,
};

// MULTI
static CURLMcode ___HTP_curl_multi_setopt(CURLM *handle, CURLMoption option, void *param) {
    CURLMcode ret;
    ret = LORELIB_API(curl_multi_setopt)(handle, option, param);
    return ret;
}

static void __HTP_curl_multi_setopt(void **args, void *ret, void **metadata) {
    __auto_type handle = *(CURLM **) args[0];
    __auto_type option = *(CURLMoption *) args[1];
    __auto_type param = *(void **) args[2];
    __auto_type ret_ref = (CURLMcode *) ret;

    switch (option) {
        case CURLMOPT_SOCKETFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_socket_callback_alloc);
            *ret_ref = ___HTP_curl_multi_setopt(handle, option, param);
            break;
        }
        case CURLMOPT_TIMERFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_multi_timer_callback_alloc);
            *ret_ref = ___HTP_curl_multi_setopt(handle, option, param);
            break;
        }
        case CURLMOPT_PUSHFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_push_callback_alloc);
            *ret_ref = ___HTP_curl_multi_setopt(handle, option, param);
            break;
        }
        default: {
            *ret_ref = ___HTP_curl_multi_setopt(handle, option, param);
            break;
        }
    }
}

// SHARE
static CURLSHcode ___HTP_curl_share_setopt(CURLSH *handle, CURLSHoption option, void *param) {
    CURLSHcode ret;
    ret = LORELIB_API(curl_share_setopt)(handle, option, param);
    return ret;
}

static void __HTP_curl_share_setopt(void **args, void *ret, void **metadata) {
    __auto_type handle = *(CURLSH **) args[0];
    __auto_type option = *(CURLSHoption *) args[1];
    __auto_type param = *(void **) args[2];
    __auto_type ret_ref = (CURLSHcode *) ret;
    switch (option) {
        case CURLSHOPT_LOCKFUNC: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_lock_function_alloc);
            *ret_ref = ___HTP_curl_share_setopt(handle, option, param);
            break;
        }
        case CURLSHOPT_UNLOCKFUNC: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_unlock_function_alloc);
            *ret_ref = ___HTP_curl_share_setopt(handle, option, param);
            break;
        }
        default: {
            *ret_ref = ___HTP_curl_share_setopt(handle, option, param);
            break;
        }
    }
}

// EASY
static CURLcode ___HTP_curl_easy_setopt(CURL *handle, CURLoption option, void *param) {
    CURLcode ret;
    ret = LORELIB_API(curl_easy_setopt)(handle, option, param);
    return ret;
}

static void __HTP_curl_easy_setopt(void **args, void *ret, void **metadata) {
    __auto_type handle = *(CURL **) args[0];
    __auto_type option = *(CURLoption *) args[1];
    __auto_type param = *(void **) args[2];
    __auto_type ret_ref = (CURLcode *) ret;
    switch ((int) option) {
        case CURLOPT_WRITEFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_write_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_READFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_read_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_IOCTLFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_ioctl_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_SEEKFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_seek_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_HEADERFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_write_callback_alloc); // header = write
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_PROGRESSFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_progress_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_XFERINFOFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_xferinfo_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_DEBUGFUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_debug_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_SSL_CTX_FUNCTION: {
            struct LoreLib_CallbackContext ctx;
            LoreLib_CallbackContext_init(ctx, param, curl_ssl_ctx_callback_alloc);
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_SSL_CTX_DATA: {
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
        case CURLOPT_SOCKOPTFUNCTION:
        case CURLOPT_OPENSOCKETFUNCTION:
        case CURLOPT_CLOSESOCKETFUNCTION:
        case CURLOPT_CONV_TO_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_UTF8_FUNCTION:
        case CURLOPT_INTERLEAVEFUNCTION:
        case CURLOPT_CHUNK_BGN_FUNCTION:
        case CURLOPT_CHUNK_END_FUNCTION:
        case CURLOPT_FNMATCH_FUNCTION:
        case CURLOPT_RESOLVER_START_FUNCTION:
        case CURLOPT_TRAILERFUNCTION:
        case _CURLOPT_HSTSREADFUNCTION:
        case _CURLOPT_HSTSWRITEFUNCTION:
        case _CURLOPT_PREREQFUNCTION:
        case _CURLOPT_SSH_HOSTKEYFUNCTION: {
            *ret_ref = CURLE_UNKNOWN_OPTION;
            break;
        }
        default: {
            *ret_ref = ___HTP_curl_easy_setopt(handle, option, param);
            break;
        }
    }
}

#endif

#ifdef curl_multi_socket
#  undef curl_multi_socket
#endif

#ifdef curl_easy_getinfo
#  undef curl_easy_getinfo
#endif

#ifdef curl_easy_setopt
#  undef curl_easy_setopt
#endif

#ifdef curl_share_setopt
#  undef curl_share_setopt
#endif

#ifdef curl_multi_setopt
#  undef curl_multi_setopt
#endif