/***************************************************************************
**                                                                        **
**   Copyright (c) 2009 - 2011 Nokia Corporation.                         **
**   Copyright (c) 2021 Jolla Ltd.                                        **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
**   Author: Simo Piiroinen <simo.piiroinen@jolla.com>                    **
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
#include <unistd.h>

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

using namespace Maemo::Timed ;

static char *slice(char *pos, char **ppos)
{
  while (*pos && isspace(*pos))
    ++pos;
  char *beg = pos;
  while (*pos && !isspace(*pos))
    ++pos;
  if (*pos)
    *pos++ = 0;
  if (ppos)
    *ppos = pos;
  return beg;
}

static void read_tz_list()
{
  if (loaded)
    free_tz_list();
  FILE *input = fopen(ZONE_ALIAS, "r");
  if (input) {
    char *text = nullptr;
    size_t size = 0;
    while (getline(&text, &size, input) >= 0) {
      vector<string> columns;
      char *pos = text;
      char *tok;
      while (*(tok = slice(pos, &pos)))
        columns.push_back(tok);
      for (auto name : columns) {
        string path = "/usr/share/zoneinfo/" + name;
        if (access(path.c_str(), R_OK) == 0) {
          int index = zones.size();
          zones.push_back(name);
          for (auto alias : columns)
            alias_to_zone[alias] = index;
          break;
        }
      }
    }
    free(text);
    fclose(input);
  }
  log_notice("loaded '%s': %d time zone names (%d with aliases)", ZONE_ALIAS, zones.size(), alias_to_zone.size());
  loaded = true;
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
