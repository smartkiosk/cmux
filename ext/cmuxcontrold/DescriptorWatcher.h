#ifndef __DESCRIPTOR_WATCHER__H__
#define __DESCRIPTOR_WATCHER__H__

class DescriptorWatcher {
public:
    virtual ~DescriptorWatcher() {}

    virtual int fd() const = 0;
    virtual bool wantsRead() const = 0;
    virtual bool wantsWrite() const = 0;

    virtual void readable() = 0;
    virtual void writable() = 0;
    virtual void abnormal() = 0;
};

#endif
