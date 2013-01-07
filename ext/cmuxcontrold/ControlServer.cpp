#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <grp.h>

#include "ControlServer.h"
#include "ControlConnection.h"
#include "EventLoop.h"

ControlServer::ControlServer(EventLoop *loop) : m_loop(loop) {
  static const struct sockaddr_un address = {
    AF_UNIX,
    "/var/run/cmuxcontrold.sock"
  };

  unlink(address.sun_path);

  m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(m_fd == -1) {
    perror("socket");

    abort();
  }

  if(fcntl(m_fd, F_SETFD, FD_CLOEXEC) == -1) {
    perror("fcntl");

    abort();
  }

  if(fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | O_NONBLOCK) == -1) {
    perror("fcntl");

    abort();
  }

  if(bind(m_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
    perror("bind");

    abort();
  }

  if(chmod(address.sun_path, 0660) == -1) {
    perror("chmod");

    abort();
  }

  struct group *group = getgrnam("dialout");
  if(group) {
    if(chown(address.sun_path, 0, group->gr_gid) == -1) {
      perror("chown");

      abort();
    }
  }



  if(listen(m_fd, SOMAXCONN) == -1) {
    perror("listen");

    abort();
  }
}

ControlServer::~ControlServer() {
  close(m_fd);

  while(!m_connections.empty())
    delete *m_connections.begin();
}


int ControlServer::fd() const {
  return m_fd;
}

bool ControlServer::wantsRead() const {
  return true;
}

bool ControlServer::wantsWrite() const {
  return false;
}

void ControlServer::readable() {
  while(1) {
    int fd = accept(m_fd, NULL, NULL);

    if(fd == -1 && errno == EWOULDBLOCK)
      break;

    if(fd == -1 && errno == ECONNABORTED)
      continue;

    if(fd == -1) {
      perror("accept");

      abort();
    }

    if(fcntl(m_fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
      perror("fcntl");

      abort();
    }

    new ControlConnection(this, fd);
  }
}

void ControlServer::writable() {

}

void ControlServer::registerConnection(ControlConnection *connection) {
  m_loop->addWatcher(connection);
  m_connections.insert(connection);
}

void ControlServer::unregisterConnection(ControlConnection *connection) {
  m_loop->removeWatcher(connection);
  m_connections.erase(connection);
}

void ControlServer::abnormal() {
  abort();
}
