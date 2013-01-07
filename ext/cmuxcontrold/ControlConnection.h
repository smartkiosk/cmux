#ifndef __CONTROL_CONNECTION__H__
#define __CONTROL_CONNECTION__H__

#include <vector>

#include "DescriptorWatcher.h"
#include "IRemoteParty.h"

class ControlServer;
class ControlRequestHandler;

class ControlConnection: public DescriptorWatcher, public IRemoteParty {
public:
  ControlConnection(ControlServer *server, int fd);
  virtual ~ControlConnection();

  virtual int fd() const;
  virtual bool wantsRead() const;
  virtual bool wantsWrite() const;

  virtual void readable();
  virtual void writable();
  virtual void abnormal();

  virtual void sendMessage(CMUX::Package &package);
  virtual void close();

  virtual pid_t pid();
  virtual uid_t uid();
  virtual gid_t gid();

private:
  ControlServer *m_server;
  int m_fd;
  std::vector<unsigned char> m_recvBuf, m_sendBuf;

  struct ucred m_cred;
  ControlRequestHandler *m_requestHandler;
};

#endif
