// SPDX-License-Identifier: MIT

#ifndef LORE_UTILS_RUNTIMEUTILS_NEXTLIBRARY_H
#define LORE_UTILS_RUNTIMEUTILS_NEXTLIBRARY_H

#include <string>

namespace lore::utils {

    /// Resolve a nextLibraryPath (\a next) against a thunk's own resolved path (\a ownPath). A bare
    /// filename (no '/') is returned unchanged, to be found on the loader's search path. A path is used
    /// as is when absolute, or joined with \a ownPath's directory when relative (so a same-directory
    /// target must be written "./name" to count as a path rather than a filename).
    std::string resolveNextLibrary(const std::string &next, const char *ownPath);

    /// The next-library filename derived from a thunk's own path (\a ownPath) when nothing else names
    /// it. A guest thunk (lib<name>.so) falls back to lib<name>_HTL.so; a host thunk (lib<name>_HTL.so,
    /// \a hostThunk true) falls back to lib<name>.so. Both are bare filenames, found on the loader's
    /// search path. A versioned soname such as "libz.so.1" is reduced to "libz" first.
    std::string nextLibraryByName(const char *ownPath, bool hostThunk);

}

#endif // LORE_UTILS_RUNTIMEUTILS_NEXTLIBRARY_H
