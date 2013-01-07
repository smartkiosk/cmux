#ifndef __CONTROL_SERVER__H__
#define __CONTROL_SERVER__H__

#include <tr1/unordered_set>

#include "DescriptorWatcher.h"

class EventLoop;
class ControlConnection;

class ControlServer: public DescriptorWatcher {
public:
  ControlServer(EventLoop *loop);
  virtual ~ControlServer();

  virtual int fd() const;
  virtual bool wantsRead() const;
  virtual bool wantsWrite() const;

  virtual void readable();
  virtual void writable();
  virtual void abnormal();

private:
  friend class ControlConnection;
  void registerConnection(ControlConnection *connection);
  void unregisterConnection(ControlConnection *connection);

private:
  EventLoop *m_loop;
  int m_fd;
  std::tr1::unordered_set<ControlConnection *> m_connections;
};

#endif
