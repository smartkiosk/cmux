#ifndef __EVENTLOOP__H__
#define __EVENTLOOP__H__

#include <tr1/unordered_set>

class DescriptorWatcher;

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  void addWatcher(DescriptorWatcher *watcher);
  void removeWatcher(DescriptorWatcher *watcher);

  int exec();
  void exit(int code);

private:
  std::tr1::unordered_set<DescriptorWatcher *> m_watchers, m_orphans, m_newcomers;
};

#endif
