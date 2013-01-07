#include <sys/socket.h>
#include <unistd.h>
#include <cmux/Package.h>
#include <errno.h>

#include "ControlConnection.h"
#include "ControlServer.h"
#include "ControlRequestHandler.h"

#define RECEIVE_AREA    4096

using namespace CMUX;

ControlConnection::ControlConnection(ControlServer *server, int fd) : m_server(server), m_fd(fd), m_recvBuf(RECEIVE_AREA) {
  socklen_t len = sizeof(m_cred);

  if(getsockopt(m_fd, SOL_SOCKET, SO_PEERCRED, &m_cred, &len) == -1) {
    ::close(m_fd);
    delete this;

    return;
  }

  m_server->registerConnection(this);

  m_requestHandler = new ControlRequestHandler(this);
}

ControlConnection::~ControlConnection() {
  delete m_requestHandler;

  ::close(m_fd);

  m_server->unregisterConnection(this);
}

int ControlConnection::fd() const {
  return m_fd;
}

bool ControlConnection::wantsRead() const {
  return true;
}

bool ControlConnection::wantsWrite() const {
  return !m_sendBuf.empty();
}

void ControlConnection::readable() {
  size_t size = m_recvBuf.size();

  ssize_t bytes = recv(m_fd, &m_recvBuf[size - RECEIVE_AREA], RECEIVE_AREA, 0);

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

  size_t readPtr = 0;

  while(usedBytes >= sizeof(MessageHeader)) {
    MessageHeader *header = (MessageHeader *) &m_recvBuf[readPtr];

    size_t totalSize = sizeof(MessageHeader) + header->length;

    if(usedBytes < totalSize)
      break;

    std::vector<unsigned char> data;

    data.insert(data.begin(), m_recvBuf.begin() + readPtr + sizeof(MessageHeader),
                m_recvBuf.begin() + readPtr + totalSize);

    readPtr += totalSize;
    usedBytes -= totalSize;

    Package package;
    package.demarshal(data);
    m_requestHandler->handleMessage(package);
  }

  m_recvBuf.erase(m_recvBuf.begin(), m_recvBuf.begin() + readPtr);

  m_recvBuf.resize(usedBytes + RECEIVE_AREA);
}

void ControlConnection::writable() {
  int bytes = send(m_fd, &m_sendBuf[0], m_sendBuf.size(), 0);
  if(bytes == -1 && errno == EAGAIN)
    return;

  if(bytes == -1) {
    delete this;
    return;
  }

  m_sendBuf.erase(m_sendBuf.begin(), m_sendBuf.begin() + bytes);
}

void ControlConnection::close() {
  delete this;
}

void ControlConnection::sendMessage(Package &package) {
  std::vector<unsigned char> data = package.marshal();

  MessageHeader header;
  header.length = data.size();

  const unsigned char *hdr = (const unsigned char *) &header;
  m_sendBuf.insert(m_sendBuf.end(), hdr, hdr + sizeof(MessageHeader));
  m_sendBuf.insert(m_sendBuf.end(), data.begin(), data.end());
}

pid_t ControlConnection::pid() {
  return m_cred.pid;
}

uid_t ControlConnection::uid() {
  return m_cred.uid;
}

gid_t ControlConnection::gid() {
  return m_cred.gid;
}

void ControlConnection::abnormal() {
  delete this;
}
