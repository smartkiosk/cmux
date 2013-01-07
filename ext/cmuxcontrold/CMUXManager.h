#ifndef __CMUX_MANAGER__H__
#define __CMUX_MANAGER__H__

#include <set>
#include <sys/types.h>

class CMUXInstance;
class DescriptorWatcher;
class EventLoop;
class ControlRequestHandler;

class CMUXManager {
public:
  CMUXManager(EventLoop *loop);
  ~CMUXManager();

  static inline CMUXManager *instance() { return m_instance; }

  CMUXInstance *createMUX(ControlRequestHandler *owner);

private:
  friend class CMUXInstance;

  void installWatcher(DescriptorWatcher *watcher);
  void uninstallWatcher(DescriptorWatcher *watcher);

private:
  static CMUXManager *m_instance;
  EventLoop *m_loop;
};

#endif
