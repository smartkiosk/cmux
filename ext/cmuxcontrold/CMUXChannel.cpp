#include <pty.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdexcept>

#include "CMUXChannel.h"
#include "CMUXInstance.h"

#define RECEIVE_AREA 4096

CMUXChannel::CMUXChannel(CMUXInstance *receiver, int id) : m_receiver(receiver),
  m_id(id), m_recvBuf(RECEIVE_AREA) {

  if(openpty(&m_fd, &m_slave, NULL, NULL, NULL) == -1)
    throw std::runtime_error("openpty: " + std::string(strerror(errno)));

  fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | O_NONBLOCK);

  m_device = ttyname(m_slave);
}

CMUXChannel::~CMUXChannel() {
  close(m_fd);
  close(m_slave);
}

int CMUXChannel::fd() const {
  return m_fd;
}

bool CMUXChannel::wantsRead() const {
  return true;
}

bool CMUXChannel::wantsWrite() const {
  return !m_sendBuf.empty();
}

void CMUXChannel::readable() {
  ssize_t bytes = read(m_fd, &m_recvBuf[0], RECEIVE_AREA);

  if(bytes == 0) {
    m_receiver->closePort(m_id);

    return;
  }

  if(bytes == -1 && errno == EWOULDBLOCK)
    return;

  if(bytes == -1) {
    m_receiver->closePort(m_id);

    return;
  }

  m_receiver->deliverToChannel(m_id, &m_recvBuf[0], bytes);
}

void CMUXChannel::writable() {
  int bytes = write(m_fd, &m_sendBuf[0], m_sendBuf.size());
  if(bytes == -1 && errno == EAGAIN)
    return;

  if(bytes == -1) {
    m_receiver->closePort(m_id);

    return;
  }

  m_sendBuf.erase(m_sendBuf.begin(), m_sendBuf.begin() + bytes);
}

void CMUXChannel::abnormal() {
  m_receiver->closePort(m_id);
}

void CMUXChannel::deliverToChannel(const void *data, size_t size) {
  m_sendBuf.resize(m_sendBuf.size() + size);
  std::copy((char *) data, (char *) data + size,
            m_sendBuf.end() - size);
}
