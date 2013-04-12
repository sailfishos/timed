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
#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include <QtDebug>

namespace Maemo
{
  namespace Timed
  {
    class Event ;
  } ;
} ;

QDebug operator<<(QDebug dbg, const Maemo::Timed::Event &event) ;

class Indent
{
public:
  static void setStep(int s) ;
public:
  Indent() ;
  ~Indent() ;
  Indent & operator++() ;
  Indent & operator--() ;
private:
  Indent(int l, int s) ;
  static Indent &global() ;
private:
  static QList<Indent*> stack ;
  static int step ;
private:
  int local ;
  friend QDebug operator<<(QDebug dbg, const Indent &ind) ;
} ;

#endif
