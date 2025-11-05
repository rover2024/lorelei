#ifndef PROCMESSAGE_H
#define PROCMESSAGE_H

namespace TLC {

    /// ProcMessage - The abstract base class for all proc messages that should be generated at the
    /// test phase.
    class ProcMessage {
    public:
        virtual ~ProcMessage() = default;
    };

}

#endif // PROCMESSAGE_H
