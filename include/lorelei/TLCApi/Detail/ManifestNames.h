// SPDX-License-Identifier: MIT

#ifndef LORE_TLCAPI_MANIFESTNAMES_H
#define LORE_TLCAPI_MANIFESTNAMES_H

namespace lore::tool::TLC::names {

    // Fully-qualified ThunkInterface template names.
    inline constexpr const char *ProcFn = "lore::thunk::ProcFn";
    inline constexpr const char *ProcCb = "lore::thunk::ProcCb";
    inline constexpr const char *ProcFnDesc = "lore::thunk::ProcFnDesc";
    inline constexpr const char *ProcCbDesc = "lore::thunk::ProcCbDesc";

    // Unqualified template names (matched by their short name).
    inline constexpr const char *ProcArgFilter = "ProcArgFilter";
    inline constexpr const char *ProcReturnFilter = "ProcReturnFilter";
    inline constexpr const char *PassTagList = "PassTagList";

    // Namespace TLC generates callback type aliases into; matched as a qualified-name prefix.
    inline constexpr const char *BridgeNamespacePrefix = "lore::thunk::__bridge__::";

    // Static members of a ProcFnDesc / ProcCbDesc specialization.
    inline constexpr const char *OverlayType = "overlay_type";
    inline constexpr const char *NameHint = "name";
    inline constexpr const char *BuilderPass = "builder_pass";
    inline constexpr const char *Passes = "passes";

    // Static member of a pass tag carrying its PassID.
    inline constexpr const char *PassID = "ID";

}

#endif // LORE_TLCAPI_MANIFESTNAMES_H
