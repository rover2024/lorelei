#if defined(_WIN32)
#  define va_int(ap)                0
#  define va_unsigned_int(ap)       0
#  define va_long(ap)               0
#  define va_unsigned_long(ap)      0
#  define va_long_long(ap)          0
#  define va_unsigned_long_long(ap) 0
#  define va_float(ap)              0
#  define va_double(ap)             0
#  define va_string(ap)             0
#  define va_pointer(ap)            0
#else
#  define va_int(ap)                                                                               \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_unsigned_int(ap)                                                                      \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_long(ap)                                                                              \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_unsigned_long(ap)                                                                     \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_long_long(ap)                                                                         \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_unsigned_long_long(ap)                                                                \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_float(ap)                                                                             \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_double(ap)                                                                            \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        0;                                                                                         \
      })
#  define va_string(ap)                                                                            \
      ({                                                                                           \
        char *_val = va_arg(va, char *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        _val;                                                                                      \
      })
#  define va_pointer(ap)                                                                           \
      ({                                                                                           \
        void *_val = va_arg(va, void *);                                                           \
        struct CVargEntry entry = {.type = CVargType_Pointer};                                     \
        entry.p = _val;                                                                            \
        (*entries) = entry;                                                                        \
        entries++;                                                                                 \
        _val;                                                                                      \
      })
#endif
