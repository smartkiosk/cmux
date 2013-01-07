#include <syslog.h>

#include "EventLoop.h"
#include "ControlServer.h"

int main(int argc, char *argv[]) {
  openlog("cmuxcontrold", LOG_PERROR | LOG_PID, LOG_DAEMON);

  syslog(LOG_INFO, "CMUX Control Daemon started");

  EventLoop loop;
  ControlServer control(&loop);

  return loop.exec();
}
