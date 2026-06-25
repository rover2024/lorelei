#ifndef LORE_CLANGEXTRAS_REWRITEINSERTION_H
#define LORE_CLANGEXTRAS_REWRITEINSERTION_H

#include <iterator>
#include <set>
#include <string>

#include <clang/Basic/SourceLocation.h>

namespace lore::tool {

    /// RewriteInsertion - A pending text insertion at a single source location.
    ///
    /// Records text to splice into a source buffer at a location. When insertAfter is true the text
    /// is placed after any insertion already queued at the same location, otherwise before it,
    /// matching clang::Rewriter's insertion ordering.
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

    /// RewriteRangePackerSet - An ordered set of non-overlapping source ranges, each wrapped with
    /// surrounding text.
    ///
    /// Entries are kept sorted by the start of their range. insert() maintains the non-overlapping
    /// invariant by rejecting a range that would collide with an existing one, so the whole set can
    /// later be replayed into a rewriter without producing conflicting edits.
    struct RewriteRangePackerSet {
    public:
        /// RangePacker - A source range together with the text to wrap around it on rewrite.
        struct RangePacker {
            clang::CharSourceRange range;
            std::string textBefore; ///< inserted immediately before the range
            std::string textAfter;  ///< inserted immediately after the range

            /// Orders packers by the start location of their range.
            inline bool operator<(const RangePacker &RHS) const {
                return range.getBegin() < RHS.range.getBegin();
            }
        };

        inline RewriteRangePackerSet() = default;

        /// Inserts a packer, keeping the set sorted and non-overlapping.
        ///
        /// Returns false and inserts nothing when the packer overlaps the range that precedes it.
        /// Only the preceding range is tested, so a packer whose range starts after an existing one
        /// but extends into a later range is not currently rejected.
        inline bool insert(RangePacker packer) {
            auto it = m_ranges.upper_bound(packer);
            if (it != m_ranges.begin()) {
                auto prev = std::prev(it);
                if (prev->range.getEnd() >= packer.range.getBegin()) {
                    return false;
                }
            }
            m_ranges.insert(it, std::move(packer));
            return true;
        }

        inline const std::set<RangePacker> &ranges() const {
            return m_ranges;
        }

    protected:
        std::set<RangePacker> m_ranges;
    };

}

#endif // LORE_CLANGEXTRAS_REWRITEINSERTION_H
