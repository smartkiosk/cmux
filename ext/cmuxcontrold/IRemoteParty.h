#ifndef __IREMOTE_PARTY__H__
#define __IREMOTE_PARTY__H__

#include <sys/types.h>

namespace CMUX {
    class Package;
};

class IRemoteParty {
public:
    virtual ~IRemoteParty() {}

    virtual void sendMessage(CMUX::Package &package) = 0;
    virtual void close() = 0;

    virtual pid_t pid() = 0;
    virtual uid_t uid() = 0;
    virtual gid_t gid() = 0;
};

#endif
