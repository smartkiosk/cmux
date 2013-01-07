#include <sys/poll.h>
#include <stdio.h>
#include <syslog.h>

#include "EventLoop.h"
#include "DescriptorWatcher.h"

EventLoop::EventLoop() {

}

EventLoop::~EventLoop() {

}

void EventLoop::addWatcher(DescriptorWatcher *watcher) {
  m_watchers.insert(watcher);
}

void EventLoop::removeWatcher(DescriptorWatcher *watcher) {
  m_orphans.insert(watcher);
}

int EventLoop::exec() {
  while(1) {
    struct pollfd fds[m_watchers.size()], *ptr = fds;

    std::tr1::unordered_set<DescriptorWatcher *>::iterator it = m_watchers.begin(), orphan_it;

    while(it != m_watchers.end()) {

      DescriptorWatcher *watcher = *it;

      orphan_it = m_orphans.find(watcher);
      if(orphan_it != m_orphans.end()) {
        m_orphans.erase(orphan_it);
        it = m_watchers.erase(it);
        continue;
      } else {
        it++;
      }

      ptr->fd = watcher->fd();
      ptr->events = 0;
      if(watcher->wantsRead())
        ptr->events |= POLLIN;

      if(watcher->wantsWrite())
        ptr->events |= POLLOUT;

      ptr++;
    }

    int ret = poll(fds, m_watchers.size(), -1);
    if(ret == -1) {
      perror("poll");

      return -1;
    }

    ptr = fds;

    it = m_watchers.begin();

    while(it != m_watchers.end()) {
      DescriptorWatcher *watcher = *it;

      orphan_it = m_orphans.find(watcher);
      if(orphan_it != m_orphans.end()) {
        m_orphans.erase(orphan_it);
        it = m_watchers.erase(it);
        continue;
      } else {
        it++;
      }

      if(ptr->revents & (POLLIN | POLLHUP)) {
        watcher->readable();
      }

      if(ptr->revents & POLLOUT) {
        watcher->writable();
      }

      if(ptr->revents & (POLLERR | POLLNVAL)) {
        syslog(LOG_CRIT, "Abnormal revents %u of fd %d\n", ptr->revents, ptr->fd);

        return -1;
      }

      ptr++;
    }

    it = m_orphans.begin();
    while(it != m_orphans.end()) {
      DescriptorWatcher *watcher = *it;

      m_watchers.erase(watcher);

      it = m_orphans.erase(it);
    }
  }

  return 0;
}

void EventLoop::exit(int code) {

}
