// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_BASIC_REWRITEINSERTION_H
#define LORE_TOOLS_BASIC_REWRITEINSERTION_H

#include <string>
#include <set>
#include <algorithm>

#include <clang/Basic/SourceLocation.h>

namespace HLR {

    class RewriteInsertion {
    public:
        RewriteInsertion(clang::SourceLocation loc = {}, std::string text = {},
                         bool insertAfter = false)
            : location(loc), text(std::move(text)), insertAfter(insertAfter) {
        }

        clang::SourceLocation location;
        std::string text;
        bool insertAfter;
    };

    struct RewriteRangePackerSet {
    public:
        struct RangePacker {
            clang::CharSourceRange range;
            std::string textBefore;
            std::string textAfter;

            inline bool operator<(const RangePacker &RHS) const {
                return range.getBegin() < RHS.range.getBegin();
            }
        };

        inline RewriteRangePackerSet() = default;

        inline bool insert(RangePacker packer) {
            auto it = _ranges.upper_bound(packer);
            if (it != _ranges.begin()) {
                auto it2 = std::prev(it);
                if (it2->range.getEnd() >= packer.range.getBegin()) {
                    return false;
                }
            }
            _ranges.insert(it, std::move(packer));
            return true;
        }

        inline const std::set<RangePacker> &ranges() const {
            return _ranges;
        }

    protected:
        std::set<RangePacker> _ranges;
    };

}

#endif // LORE_TOOLS_BASIC_REWRITEINSERTION_H
