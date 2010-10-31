/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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
#include <qmlog>

#include "misc.h"

using namespace std ;

QString string_std_to_q(const string &str)
{
  return QString::fromUtf8(str.c_str()) ;
}

std::string string_q_to_std(const QString &str)
{
  return (string) str.toUtf8().constData() ;
}

void map_q_to_std(const QMap<QString, QString> &from, map<string, string> &to)
{
  for(QMap<QString, QString>::const_iterator it=from.begin(); it!=from.end(); ++it)
  {
    string k = string_q_to_std(it.key()) ;
    string v = string_q_to_std(it.value()) ;
    if(k.empty() || v.empty())
    {
      log_warning("Ignoring empty key or value: '%s'=>'%s'", k.c_str(), v.c_str()) ;
      continue ;
    }
    map<string, string>::const_iterator old = to.find(k) ;
    if(old != to.end())
      log_warning("Duplicate key: '%s'=>'%s', using new value '%s'", k.c_str(), old->second.c_str(), v.c_str()) ;
    to[k] = v ;
  }
}

void map_std_to_q(const map<string, string> &from, QMap<QString, QString> &to)
{
  for(map<string, string>::const_iterator it=from.begin(); it!=from.end(); ++it)
  {
    QString k = string_std_to_q(it->first) ;
    QString v = string_std_to_q(it->second) ;
    if(k.isEmpty() || v.isEmpty())
    {
      log_warning("Ignoring empty key or value: '%s'=>'%s'", it->first.c_str(), it->second.c_str()) ;
      continue ;
    }
    QMap<QString, QString>::const_iterator old = to.find(k) ;
    if(old != to.end())
      log_warning("Duplicate key: '%s'=>'%s', using new value '%s'", it->first.c_str(), string_q_to_std(old.value()).c_str(), it->second.c_str()) ;
    to.insert(k, v) ;
  }
}

string str_printf(const char *format, ...)
{
  const int buf_len = 1024, max_buf_len = buf_len*1024 ;
  char buf[buf_len], *p = buf ;
  va_list varg ;
  va_start(varg, format) ;
  int iteration = 0, printed = false ;
  string formatted ;
  do
  {
    int size = buf_len << iteration ;
    if(size>max_buf_len)
    {
      log_error("Can't format string, the result is too long") ;
      return format ;
    }
    if(iteration>0)
      p = new char[size] ;
    int res = vsnprintf(p, size, format, varg) ;
    if(res < 0)
    {
      log_error("Can't format string, vsnprintf() failed") ;
      return format ;
    }
    if(res < size)
    {
      printed = true ;
      formatted = p ;
    }
    if(iteration > 0)
      delete[] p ;
  } while(!printed) ;

  return formatted ;
}
