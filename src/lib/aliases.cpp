/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
**                                                                        **
**     This file is part of Timed                                         **
**                                                                        **
**     Timed is free software; you can redistribute it and/or modify      **
**     it under the terms of the GNU Lesser General Public License        **
**     version 2.1 as published by the Free Software Foundation.          **
**                                                                        **
**     Timed is distributed in the hope that it will be useful, but       **
**     WITHOUT ANY WARRANTY;  without even the implied warranty  of       **
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               **
**     See the GNU Lesser General Public License  for more details.       **
**                                                                        **
**   You should have received a copy of the GNU  Lesser General Public    **
**   License along with Timed. If not, see http://www.gnu.org/licenses/   **
**                                                                        **
***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <cstring>

using namespace std ;

#include "../common/log.h"
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
  {
    log_error("can't load olson name list (%s): %m", ZONE_ALIAS) ;
    return ;
  }

  // log_debug("loaded %d bytes from '%s'", strlen(txt), ZONE_ALIAS) ;

  bool new_line = true ;
  for(const char *p=txt; *p != '\0'; )
  {
    if (isspace(*p))
    {
      if (*p++=='\n')
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
      // log_debug("word: '%s' is alias for zone %d (name '%s')", word.c_str(), alias_to_zone[word], zones[alias_to_zone[word]].c_str()) ;
    }
  }

  log_notice("loaded '%s': %d time zone names (%d with aliases)", ZONE_ALIAS, zones.size(), alias_to_zone.size()) ;

  loaded = true ;

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
