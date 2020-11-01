#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int ignore_SIGPIPE_signals() {
  struct sigaction s = {};
  s.sa_handler = SIG_IGN;
  if (sigaction(SIGPIPE, &s, NULL) != 0) {
    fprintf(stderr, "Failed to ignore SIGPIPE signals\n");
    return -1;
  }
  return 0;
}

