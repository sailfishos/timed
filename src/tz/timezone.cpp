#include <iodata/storage>

#include "timezone.h"
using libtz::internal::timezone_t ;

map<string,timezone_t*> libtz::internal::timezone_t::loaded ;
set<string, string> libtz::internal::timezone_t::failed ;

timezone_t *timezone_t::load(const string &s, string &error)
{
  map<string,timezone_t*>::const_iterator known = loaded.find(s) ;
  if (known!=loaded.end())
    return known->second ;

  map<string,string>::const_iterator it = failed.find(s) ;
  if (it!=failed.end())
  {
    error = it->second ;
    return NULL ;
  }

  timezone_t *t = load_from_file(s, error) ;

  if (t==NULL)
    failed[s] = error ;
  else
    loaded[s] = t ;

  return t ;
}

timezone_t *timezone_t::load_from_file(const string &path, string &error)
{

  int fd = open(path.c_str(), O_RDONLY) ;
  if (fd<0)
  {
    error = str_printf("open '%s' for reading failed: %m", path.c_str()) ;
    return NULL ;
  }

  struct stat stat ;
  int stat_res = fstat(fd, &stat) ;
  if (stat_res<0)
  {
    error = str_printf("fstat '%s' failed: %m", path.c_str()) ;
    close(fd) ;
    return NULL ;
  }

  size_t file_size = stat.st_size ;
  if (file_size<=0)
  {
    error = str_printf("file '%s' has zero length", path.c_str()) ;
    close(fd) ;
    return NULL ;
  }

  void *map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0) ;
  if (map==MAP_FAILED)
  {
    error = str_printf("mmap '%s' failed", path.c_str()) ;
    close(fd) ;
    return NULL ;
  }

  timezone_t *p = new timezone_t ;
  try
  {
    struct mapped_file_t file(map, file_size) ;
    p->init(&file) ;
  }
  catch (const string &message)
  {
    delete p ;
    error = message ;
    p = NULL ;
  }

  munmap(map, file_size) ;
  close(fd) ;

  return p ;
}

void timezone_t::init(mapped_file_t *f)
{
  f->get_string() ;
}
