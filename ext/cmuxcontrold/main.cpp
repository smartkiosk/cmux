#include <syslog.h>

#include "EventLoop.h"
#include "ControlServer.h"
#include "CMUXManager.h"

int main(int argc, char *argv[]) {
  openlog("cmuxcontrold", LOG_PERROR | LOG_PID, LOG_DAEMON);

  syslog(LOG_INFO, "CMUX Control Daemon started");

  EventLoop loop;
  CMUXManager manager(&loop);
  ControlServer control(&loop);
  loop.addWatcher(&control);

  return loop.exec();
}
