#ifndef __CMUX__CONNECTION__H__
#define __CMUX__CONNECTION__H__

#include <string>

namespace CMUX {

enum Request {
  RequestOpen = 0,
  RequestActivate,
  RequestOpenPort,
  RequestClosePort
};

class Package;

class Connection {
public:
  Connection();
  ~Connection();

  void open(const std::string &device);
  void activate();

  std::string openPort(int port);
  void closePort(int port);

private:
  bool exchange(Package &in, Package &out);

  static bool writeFully(int fd, const void *data, size_t size);
  static bool readFully(int fd, void *data, size_t size);

  int m_fd;
};

}

#endif
