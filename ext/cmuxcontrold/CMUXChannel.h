#ifndef __CMUXCHANNEL__H__
#define __CMUXCHANNEL__H__

#include <string>
#include <vector>

#include "DescriptorWatcher.h"

class CMUXInstance;

class CMUXChannel: public DescriptorWatcher {
public:
  CMUXChannel(CMUXInstance *receiver, int id);
  virtual ~CMUXChannel();

  const std::string &device() const { return m_device; }

  virtual int fd() const;
  virtual bool wantsRead() const;
  virtual bool wantsWrite() const;

  virtual void readable();
  virtual void writable();
  virtual void abnormal();

  void deliverToChannel(const void *data, size_t size);

private:
  CMUXInstance *m_receiver;
  int m_id;
  int m_fd, m_slave;
  std::string m_device;
  std::vector<unsigned char> m_recvBuf, m_sendBuf;
};

#endif
