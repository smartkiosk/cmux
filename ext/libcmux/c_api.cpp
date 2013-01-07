#include <cmux/c_api.h>
#include <cmux/Connection.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <exception>

using namespace CMUX;

struct CMUX_CTX {
  Connection *connection;
  char *error;
};

CMUX_CTX *cmux_create(char **error) {
  CMUX_CTX *cmux = (CMUX_CTX *) malloc(sizeof(CMUX_CTX));
  if(cmux == NULL) {
    *error = strdup(strerror(errno));
    return NULL;
  }

  try {
    cmux->connection = new Connection;
    cmux->error = NULL;

    return cmux;

  } catch(const std::exception &e) {
    *error = strdup(e.what());

    return NULL;
  }
}

void cmux_destroy(CMUX_CTX *cmux) {
  free(cmux->error);
  delete cmux->connection;
  free(cmux);
}

const char *cmux_error(CMUX_CTX *cmux) {
  return cmux->error;
}

int cmux_open(CMUX_CTX *cmux, const char *device) {
  try {
    cmux->connection->open(device);

    return 0;
  } catch(const std::exception &e) {
    free(cmux->error);
    cmux->error = strdup(e.what());

    return -1;
  }
}

int cmux_activate(CMUX_CTX *cmux) {
  try {
    cmux->connection->activate();

    return 0;
  } catch(const std::exception &e) {
    free(cmux->error);
    cmux->error = strdup(e.what());

    return -1;
  }
}

int cmux_open_port(CMUX_CTX *cmux, int port, char **device) {
  try {
    std::string path = cmux->connection->openPort(port);
    *device = strdup(path.c_str());

    return 0;
  } catch(const std::exception &e) {
    free(cmux->error);
    cmux->error = strdup(e.what());

    return -1;
  }
}

int cmux_close_port(CMUX_CTX *cmux, int port) {
  try {
    cmux->connection->closePort(port);

    return 0;
  } catch(const std::exception &e) {
    free(cmux->error);
    cmux->error = strdup(e.what());

    return -1;
  }
}


void cmux_free(void *data) {
  free(data);
}

int cmux_open_device(const char *filename, char **error) {
  struct termios attr;

  int fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);
  if(fd == -1) {
    *error = strdup(strerror(errno));

    return -1;
  }

  if(tcgetattr(fd, &attr)) {
    *error = strdup(strerror(errno));
    close(fd);;

    return -1;
  }

  attr.c_iflag = IGNBRK | IGNPAR;
  attr.c_oflag = 0;
  attr.c_cflag = CS8 | HUPCL | CRTSCTS | CREAD | CLOCAL;
  attr.c_lflag = 0;

  cfsetispeed(&attr, B115200);
  cfsetospeed(&attr, B115200);

  if(tcsetattr(fd, TCSANOW, &attr)) {
    *error = strdup(strerror(errno));
    close(fd);

    return -1;
  }

  return fd;
}
