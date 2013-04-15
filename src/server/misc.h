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
#ifndef MISC_H
#define MISC_H

#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <set>

#include <QMap>
#include <QString>

QString string_std_to_q(const std::string &str) ;
std::string string_q_to_std(const QString &str) ;

void map_q_to_std(const QMap<QString, QString> &from, std::map<std::string, std::string> &to) ;
void map_std_to_q(const std::map<std::string, std::string> &from, QMap<QString, QString> &to) ;

std::string str_iso8601(time_t t) ;
std::string str_printf(const char *format, ...) __attribute__((format(printf,1,2))) ;

inline int n_bits_64(uint64_t x)
{
  // printf("step0: x=%Ld\n", x) ;
  x = (x&0x5555555555555555LL) + (((x&0xAAAAAAAAAAAAAAAALL)>>1)) ;
  // printf("step1: x=%Ld\n", x) ;
  x = (x&0x3333333333333333LL) + (((x&0xCCCCCCCCCCCCCCCCLL)>>2)) ;
  // printf("step2: x=%Ld\n", x) ;
  x = (x&0x0F0F0F0F0F0F0F0FLL) + (((x&0xF0F0F0F0F0F0F0F0LL)>>4)) ;
  // printf("step3: x=%Ld\n", x) ;
  x = (x&0x00FF00FF00FF00FFLL) + (((x&0xFF00FF00FF00FF00LL)>>8)) ;
  // printf("step3: x=%Ld\n", x) ;
  // printf("step4: x=%Ld\n", x%255) ;
  return (int)(x%255) ;
}

inline int n_bits_32(uint32_t x)
{
  x = (x&0x55555555) + (((x&0xAAAAAAAA)>>1)) ;
  x = (x&0x33333333) + (((x&0xCCCCCCCC)>>2)) ;
  x = (x&0x0F0F0F0F) + (((x&0xF0F0F0F0)>>4)) ;
  x = (x&0x00FF00FF) + (((x&0xFF00FF00)>>8)) ;
  return (int)(x%255) ;
}

template <class element_t>
std::string print_vector(const std::vector<element_t> &array, unsigned begin, unsigned end)
{
  std::ostringstream os ;

  for(unsigned i=begin, first=true; i<end; ++i)
  {
    os << ( first ? first=false, "[" : ", " ) ;
    os << array[i] ;
  }
  os << "]" ;
  return os.str() ;
}

template <class element_t> // an yet another hihgly inefficient function :-(
void set_change(std::set<element_t> &x, const std::set<element_t> &y, bool add)
{
  for(typename std::set<element_t>::const_iterator it=y.begin(); it!=y.end(); ++it)
    if(add)
      x.insert(*it) ;
    else
      x.erase(*it) ;
}

#endif
