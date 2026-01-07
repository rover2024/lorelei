#ifndef LORE_BASE_RUNTIMEBASE_CALLCHECKER_H
#define LORE_BASE_RUNTIMEBASE_CALLCHECKER_H

#include <string>
#include <type_traits>
#include <cstdint>

namespace lore {

    /// CallChecker - The base class for call checkers.
    ///
    /// This class provides a common interface for call checkers, which are used to determine
    /// whether a given function pointer is a guest function or a host function.
    template <class T = void>
    class CallChecker {
    public:
        /// Check whether a given function pointer is a host function.
        bool isHostAddress(const void *addr) const {
            return get()->isHostAddress_impl(addr);
        }

    private:
        T *get() {
            return static_cast<T *>(this);
        }

        const T *get() const {
            return static_cast<const T *>(this);
        }
    };

    /// MemoryRegionCallChecker - A rapid call checker for identifying the address space, based on
    /// the premise that all emulated functions are in the low address direction of the emulator and
    /// all host functions are in the high address direction of the emulator.
    class MemoryRegionCallChecker : public CallChecker<MemoryRegionCallChecker> {
    public:
        explicit MemoryRegionCallChecker(void *emuAddr = nullptr) : _emuAddr(emuAddr) {
        }

        inline void *emuAddr() const {
            return _emuAddr;
        }

        inline void setEmuAddr(void *emuAddr) {
            _emuAddr = emuAddr;
        }

    protected:
        inline bool isHostAddress_impl(const void *addr) const {
            return reinterpret_cast<uintptr_t>(addr) > reinterpret_cast<uintptr_t>(_emuAddr);
        }

        void *_emuAddr;

        friend class CallChecker<MemoryRegionCallChecker>;
    };

}

#endif // LORE_BASE_RUNTIMEBASE_CALLCHECKER_H
