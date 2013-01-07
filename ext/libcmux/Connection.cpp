#include <cmux/Connection.h>
#include <cmux/Package.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <unistd.h>

namespace CMUX {

Connection::Connection() {
  static const struct sockaddr_un address = {
    AF_UNIX,
    "/var/run/cmuxcontrold.sock"
  };

  m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(m_fd == -1)
    throw std::runtime_error(strerror(errno));


  if(::connect(m_fd, (const struct sockaddr *) &address, sizeof(address)) == -1) {
    close(m_fd);
    throw std::runtime_error(strerror(errno));
  }
}

Connection::~Connection() {
  close(m_fd);
}

bool Connection::exchange(Package &in, Package &out) {

  std::vector<unsigned char> inData = in.marshal(), outData;

  MessageHeader header;
  header.length = inData.size();

  if(!writeFully(m_fd, &header, sizeof(header)))
    return false;

  if(!writeFully(m_fd, &inData[0], inData.size()))
    return false;

  if(!readFully(m_fd, &header, sizeof(header)))
    return false;

  outData.resize(header.length);
  if(!readFully(m_fd, &outData[0], outData.size()))
    return false;

  out.demarshal(outData);
  return true;
}

bool Connection::writeFully(int fd, const void *data, size_t size) {
  const unsigned char *cdata = (const unsigned char *) data;

  while(size) {
    ssize_t bytes = write(fd, cdata, size);

    if(bytes == -1 && errno == EINTR)
      continue;

    if(bytes == -1)
      return false;

    size -= bytes;
    cdata += bytes;
  }

  return true;
}

bool Connection::readFully(int fd, void *data, size_t size) {
  unsigned char *cdata = (unsigned char *) data;

  while(size) {
    ssize_t bytes = read(fd, cdata, size);

    if(bytes == -1 && errno == EINTR)
      continue;

    if(bytes < 1) {
      if(bytes == 0) {
        errno = EIO;
      }

      return false;
    }

    size -= bytes;
    cdata += bytes;
  }

  return true;
}

void Connection::open(const std::string &device) {
  Package in, out;

  in.writeByte(RequestOpen);
  in.writeString(device);
  if(!exchange(in, out))
    throw std::runtime_error("communication error");

  int status = out.readByte();
  if(status == 0) {
    std::string error = out.readString();

    throw std::runtime_error(error);
  }
}

void Connection::activate() {
  Package in, out;

  in.writeByte(RequestActivate);
  if(!exchange(in, out))
    throw std::runtime_error("communication error");

  int status = out.readByte();
  if(status == 0) {
    std::string error = out.readString();

    throw std::runtime_error(error);
  }
}

std::string Connection::openPort(int port) {
  Package in, out;

  in.writeByte(RequestOpenPort);
  in.writeInt(port);
  if(!exchange(in, out))
    throw std::runtime_error("communication error");

  int status = out.readByte();
  if(status == 0) {
    std::string error = out.readString();

    throw std::runtime_error(error);
  } else {
    return out.readString();
  }
}

void Connection::closePort(int port) {
  Package in, out;

  in.writeByte(RequestClosePort);
  in.writeInt(port);
  if(!exchange(in, out))
    throw std::runtime_error("communication error");

  int status = out.readByte();
  if(status == 0) {
    std::string error = out.readString();

    throw std::runtime_error(error);
  }
}


}
