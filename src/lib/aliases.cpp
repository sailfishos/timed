#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>

using namespace std ;

#include "log.h"
#include "aliases.h"

#define ZONE_ALIAS "/usr/share/tzdata-timed/zone.alias"

static map<string, int> alias_to_zone ;
static vector<string> zones ;
static bool loaded = false ;

static void read_tz_list() ;
static const char *read_file(const char *file) ;
static void free_file_memory(const char *buffer) ;

using namespace Maemo::Timed ;

void read_tz_list()
{
  if (loaded)
    free_tz_list() ;

  const char *txt = read_file(ZONE_ALIAS) ;

  if (txt==NULL)
    return ;

  bool new_line = true ;
  for(const char *p=txt; *p != '\0'; ++p)
  {
    if (isspace(*p))
    {
      if (*p=='\n')
        new_line = true ;
    }
    else
    {
      const char *w = p ;
      while (not isspace(*p) and *p!='\0')
        ++ p ;
      string word(w, p-w) ;
      if (new_line)
      {
        zones.push_back(word) ;
        new_line = false ;
      }
      alias_to_zone[word] = zones.size() - 1 ;
    }
  }

  free_file_memory(txt) ;
}

void Maemo::Timed::free_tz_list()
{
  zones.resize(0) ;
  alias_to_zone.clear() ;
}

bool Maemo::Timed::is_tz_name(const string &tz)
{
  if (not loaded)
    read_tz_list() ;

  if (not loaded)
    return false ;

  map<string, int>::const_iterator it = alias_to_zone.find(tz) ;
  return it != alias_to_zone.end() ;
}

string Maemo::Timed::tz_alias_to_name(const string &tz)
{
  if (not loaded)
    read_tz_list() ;

  if (not loaded)
    return "" ;

  map<string, int>::const_iterator it = alias_to_zone.find(tz) ;
  if (it==alias_to_zone.end())
    return "" ;

  return zones[it->second] ;
}

void free_file_memory(const char *buffer)
{
  delete[] buffer ;
}

const char *read_file(const char *file)
{
  int fd = open(file, O_RDONLY) ;

  if(fd < 0)
    return NULL ;

  struct stat st ;
  if(fstat(fd, &st) < 0)
  {
    int errno_copy = errno ;
    close(fd) ;
    errno = errno_copy ;
    return NULL ;
  }

  int size = st.st_size ;

  if(size<=0)
  {
    close(fd) ;
    errno = EIO ; // TODO find a better one?
    return NULL ;
  }

  int done = 0 ;
  char *buffer = new char[size+1] ;
  log_assert(buffer) ;

  while(done < size)
  {
    ssize_t bytes = read (fd, buffer + done, size - done) ;
    if(bytes>0)
      done += bytes ;
    else if(bytes==0 || errno!=EINTR) // EOF or error (not interrupt)
      break ;
    else if(lseek(fd, done, SEEK_SET)!=done) // fix the position, if interrupted
      break ;
  }

  int errno_copy = errno ;
  close(fd) ; // failed? who cares, we got data already

  if(done < size)
  {
    delete[] buffer ;
    errno = errno_copy ;
    return NULL ;
  }

  buffer[size] = '\0' ;

  if(strlen(buffer)!=(unsigned)size) // some '\0' inside ?
  {
    delete[] buffer ;
    errno = EILSEQ ;
    return NULL ;
  }

  return buffer ;
}
