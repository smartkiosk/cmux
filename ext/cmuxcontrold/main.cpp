#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

#include "EventLoop.h"
#include "ControlServer.h"
#include "CMUXManager.h"

static int parse_opts(int argc, char **argv, int *daemon, char **pidfile) {
  int ch;

  *daemon = 0;
  *pidfile = 0;

  while((ch = getopt(argc, argv, "dp:")) != -1) {
    switch(ch) {
      case 'd':
        *daemon = 1;

        break;

      case 'p':
        *pidfile = optarg;

        break;

      case '?':
      case ':':
        return 0;
    }
  }

  return 1;
}

static int daemonize(void) {
  pid_t pid = fork();
  if(pid == -1)
    return 0;

  if(pid > 0)
    _exit(0);

  setsid();
  umask(0);
  chdir("/");
  int null = open("/dev/null", O_RDWR);
  dup2(STDIN_FILENO, null);
  dup2(STDOUT_FILENO, null);
  dup2(STDERR_FILENO, null);
  close(null);

  pid = fork();
  if(pid == -1)
    return 0;

  if(pid > 0)
    _exit(0);

  return 1;
}

int main(int argc, char *argv[]) {
  int daemon;
  char *pidfile;
  if(!parse_opts(argc, argv, &daemon, &pidfile))
    return 1;

  if(daemon) {
    if(!daemonize())
      return 1;

    openlog("cmuxcontrold", LOG_PID, LOG_DAEMON);

  } else {
    openlog("cmuxcontrold", LOG_PERROR | LOG_PID, LOG_DAEMON);
  }

  if(pidfile) {
    FILE *file = fopen(pidfile, "w");
    if(file == NULL) {
      syslog(LOG_CRIT, "unable to create pidfile: %s", strerror(errno));

      return 1;
    }

    fprintf(file, "%d\n", getpid());
    fclose(file);
  }

  syslog(LOG_INFO, "CMUX Control Daemon started");

  EventLoop loop;
  CMUXManager manager(&loop);
  ControlServer control(&loop);
  loop.addWatcher(&control);

  return loop.exec();
}
