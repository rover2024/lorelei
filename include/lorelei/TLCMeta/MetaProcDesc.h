#ifndef PROCDESC_H
#define PROCDESC_H

#define _DESC static constexpr const

namespace lorethunk {

    template <auto F>
    struct MetaProcDesc {};

    template <class F>
    struct MetaProcCBDesc {};

}

#endif // PROCDESC_H
