#ifndef __CMUX_INSTANCE__H__
#define __CMUX_INSTANCE__H__

#include <string>
#include <vector>

#include "DescriptorWatcher.h"
#include "gsm0710.h"

class CMUXManager;
class CMUXChannel;
class ControlRequestHandler;

class CMUXInstance: public DescriptorWatcher {
public:
  CMUXInstance(CMUXManager *parent, ControlRequestHandler *owner);

protected:
  ~CMUXInstance();
public:

  void open(const std::string &device);
  void activate();
  void shutdown();
  std::string openPort(int port, uid_t owner);
  void closePort(int port);

  virtual int fd() const;
  virtual bool wantsRead() const;
  virtual bool wantsWrite() const;

  virtual void readable();
  virtual void writable();
  virtual void abnormal();

  void deliverToChannel(int channel, const void *data, size_t size);

private:
  void doOpenChannel(int channel);
  void doCloseChannel(int channel);

  static int gsm0710_read(struct gsm0710_context *ctx, void *data, int len);
  static int gsm0710_write(struct gsm0710_context *ctx, const void *data,
                           int len);
  static void gsm0710_deliver_data(struct gsm0710_context *ctx, int channel,
                                   const void *data, int len);
  static void gsm0710_deliver_status(struct gsm0710_context *ctx, int channel,
                                     int status);
  static void gsm0710_debug_message(struct gsm0710_context *ctx,
                                    const char *msg);
  static void gsm0710_open_channel(struct gsm0710_context *ctx, int channel);
  static void gsm0710_close_channel(struct gsm0710_context *ctx, int channel);
  static void gsm0710_terminate(struct gsm0710_context *ctx);

  CMUXManager *m_parent;
  int m_fd;
  bool m_active, m_shutdown;
  std::vector<unsigned char> m_recvBuf, m_sendBuf;
  struct gsm0710_context m_ctx;
  std::vector<CMUXChannel *> m_channels;
  ControlRequestHandler *m_owner;
};

#endif
