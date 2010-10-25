#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int main()
{
  char buf[100] ;
  time_t t=time(NULL) ;
  struct tm tm ;
  localtime_r(&t, &tm) ;
  sprintf(buf, "logging-test[%d]: %04d-%02d-%02d %02d-%02d-%02d", getpid(), tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) ;
  fprintf(stderr, "%s\n", buf) ;
  openlog("logging-test", LOG_NOWAIT|LOG_DAEMON, LOG_DAEMON) ;
  syslog(LOG_CRIT, "%s", buf) ;
  closelog() ;
  FILE *fp = fopen("/var/log/logging-test", "a") ;
  if (fp!=NULL)
  {
    fprintf(fp, "%s\n", buf) ;
    fflush(NULL) ;
    fclose(fp) ;
  }
  return 0 ;
}

