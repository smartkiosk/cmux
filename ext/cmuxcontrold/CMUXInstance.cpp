#include <cmux/c_api.h>
#include <stdexcept>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "CMUXInstance.h"
#include "CMUXManager.h"
#include "CMUXChannel.h"
#include "ControlRequestHandler.h"

#define RECEIVE_AREA 4096

CMUXInstance::CMUXInstance(CMUXManager *parent, ControlRequestHandler *owner) :
  m_parent(parent), m_fd(-1), m_active(false), m_shutdown(false),
  m_recvBuf(RECEIVE_AREA), m_owner(owner) {

  gsm0710_initialize(&m_ctx);

  m_ctx.user_data = this;
  m_ctx.read = CMUXInstance::gsm0710_read;
  m_ctx.write = CMUXInstance::gsm0710_write;
  m_ctx.deliver_data = CMUXInstance::gsm0710_deliver_data;
  m_ctx.deliver_status = CMUXInstance::gsm0710_deliver_status;
  m_ctx.debug_message = CMUXInstance::gsm0710_debug_message;
  m_ctx.open_channel = CMUXInstance::gsm0710_open_channel;
  m_ctx.close_channel = CMUXInstance::gsm0710_close_channel;
  m_ctx.terminate = CMUXInstance::gsm0710_terminate;
}

CMUXInstance::~CMUXInstance() {
  if(m_owner)
    m_owner->multiplexerTerminated();

  if(m_active) {
    m_parent->uninstallWatcher(this);
  }

  for(unsigned int i = 0; i < m_channels.size(); i++) {
    CMUXChannel *channel = m_channels[i];

    if(channel) {
      m_parent->uninstallWatcher(channel);
      delete channel;
    }
  }

  if(m_fd != -1) {
    syslog(LOG_INFO, "%p: closed", this);
    close(m_fd);
  }
}

void CMUXInstance::shutdown() {
  m_owner = 0;

  if(!m_active)
    delete this;
  else {
    syslog(LOG_INFO, "%p: shutting down", this);

    m_shutdown = true;
    gsm0710_shutdown(&m_ctx);
  }
}

void CMUXInstance::open(const std::string &device) {
  char *error;
  m_fd = cmux_open_device(device.c_str(), &error);
  if(m_fd == -1) {
    std::string err_str = error;
    free(error);

    throw std::runtime_error(err_str);
  }

  syslog(LOG_INFO, "%p: open %s", this, device.c_str());
}

void CMUXInstance::activate() {
  if(m_fd == -1)
    throw std::runtime_error("device is not open yet");

  if(m_active)
    throw std::runtime_error("device is already activated");

  m_parent->installWatcher(this);
  m_active = true;

  syslog(LOG_INFO, "%p: activated", this);

  gsm0710_startup(&m_ctx, 0);
}

std::string CMUXInstance::openPort(int port, uid_t owner) {
  if(!m_active)
    throw std::runtime_error("device is not activated yet");

  if(!::gsm0710_open_channel(&m_ctx, port))
    throw std::runtime_error("channel is unavailable");

  doOpenChannel(port);

  CMUXChannel *channel = m_channels[port];

  chmod(channel->device().c_str(), 0600);
  chown(channel->device().c_str(), owner, 0);

  return channel->device();
}

void CMUXInstance::closePort(int port) {
  doCloseChannel(port);

  gsm0710_close_channel(&m_ctx, port);
}

void CMUXInstance::doOpenChannel(int port) {
  if(m_channels.size() <= (unsigned int) port)
    m_channels.resize(port + 1, 0);

  CMUXChannel *channel = m_channels[port];
  if(channel == NULL) {
    channel = new CMUXChannel(this, port);
    m_parent->installWatcher(channel);
    m_channels[port] = channel;
  }
}

void CMUXInstance::doCloseChannel(int port) {
  if(m_channels.size() <= (unsigned int) port)
    m_channels.resize(port + 1, 0);

  CMUXChannel *channel = m_channels[port];
  if(channel != NULL) {
    m_parent->uninstallWatcher(channel);
    delete channel;
    m_channels[port] = 0;
  }
}

int CMUXInstance::fd() const {
  return m_fd;
}

bool CMUXInstance::wantsRead() const {
  return true;
}

bool CMUXInstance::wantsWrite() const {
  return !m_sendBuf.empty();
}

void CMUXInstance::readable() {
  size_t size = m_recvBuf.size();

  ssize_t bytes = read(m_fd, &m_recvBuf[size - RECEIVE_AREA], RECEIVE_AREA);

  if(bytes == 0) {
    delete this;

    return;
  }

  if(bytes == -1 && errno == EWOULDBLOCK)
    return;

  if(bytes == -1) {
    delete this;

    return;
  }

  size_t usedBytes = size + bytes - RECEIVE_AREA;
  m_recvBuf.resize(usedBytes + RECEIVE_AREA);

  gsm0710_ready_read(&m_ctx);
}

void CMUXInstance::writable() {
  int bytes = write(m_fd, &m_sendBuf[0], m_sendBuf.size());
  if(bytes == -1 && errno == EAGAIN)
    return;

  if(bytes == -1) {
    delete this;

    return;
  }

  m_sendBuf.erase(m_sendBuf.begin(), m_sendBuf.begin() + bytes);

  if(m_shutdown && m_sendBuf.empty()) {
    delete this;
  }
}

void CMUXInstance::abnormal() {
  delete this;
}

void CMUXInstance::deliverToChannel(int id, const void *data, size_t len) {
  gsm0710_write_data(&m_ctx, id, data, len);
}

int CMUXInstance::gsm0710_read(struct gsm0710_context *ctx,
                               void *data, int len) {

  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  int bytes = std::min<int>(instance->m_recvBuf.size() - RECEIVE_AREA, len);
  std::copy(instance->m_recvBuf.begin(),
            instance->m_recvBuf.begin() + bytes,
            (char *) data);
  instance->m_recvBuf.erase(instance->m_recvBuf.begin(),
                            instance->m_recvBuf.begin() + bytes);

  return bytes;

}

int CMUXInstance::gsm0710_write(struct gsm0710_context *ctx, const void *data,
                                int len) {

  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  instance->m_sendBuf.resize(instance->m_sendBuf.size() + len);
  std::copy((char *) data, (char *) data + len,
            instance->m_sendBuf.end() - len);

  return len;
}

void CMUXInstance::gsm0710_deliver_data(struct gsm0710_context *ctx, int channel,
                                        const void *data, int len) {

  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  instance->m_channels[channel]->deliverToChannel(data, len);
}

void CMUXInstance::gsm0710_deliver_status(struct gsm0710_context *ctx, int channel,
                                          int status) {

  (void) ctx;
  (void) channel;
  (void) status;

  /*
   * TODO: deliver line status
   *
   * In theory, we could emulate serial events like CD/DSR drop on PTY,
   * but it will require duplication of a huge chunk of a kernel logic here and
   * it's not strictly necessary for pppd.
   */
}

void CMUXInstance::gsm0710_debug_message(struct gsm0710_context *ctx,
                                         const char *msg) {

  (void) ctx;

  syslog(LOG_DEBUG, "gsm0710: %s", msg);
}

void CMUXInstance::gsm0710_open_channel(struct gsm0710_context *ctx,
                                        int port) {

  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  try {
    instance->doOpenChannel(port);
  } catch(std::exception &e) {
    syslog(LOG_WARNING, "device requested initiation of port %d, but attempt to do that failed: %s",
           port, e.what());
  }
}

void CMUXInstance::gsm0710_close_channel(struct gsm0710_context *ctx,
                                         int port) {

  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  try {
    instance->doCloseChannel(port);
  } catch(std::exception &e) {
    syslog(LOG_WARNING, "device requested termination of port %d, but attempt to do that failed: %s",
           port, e.what());
  }
}

void CMUXInstance::gsm0710_terminate(struct gsm0710_context *ctx) {
  CMUXInstance *instance = static_cast<CMUXInstance *>(ctx->user_data);

  delete instance;
}
