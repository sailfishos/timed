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
#include <cstdio>
#include <cstdlib>

struct sss
{
  int a, b ;
  sss() { invalidate() ; }
  sss(const sss &x) { a=x.b, b=x.a ; }
  const sss &operator= (const sss &x) { a=x.a, b=x.b ; return *this ; }
  const sss &operator-=(const sss &x) { a-=x.b, b-=x.a ; return *this ; }
  const sss &operator- (const sss &x) { sss y=*this; return y-=x ; }
  static sss blah1() ;
  static sss blah2() ;
  static sss blah_blah() ;
  void invalidate() { a=~0, b=~0 ; }
} ;

sss sss::blah1()
{
  sss res ;
  res.a = random() ;
  res.b = 239 ;
  return res ;
}

sss sss::blah2()
{
  sss res ;
  res.b = random() ;
  res.a = 57 ;
  return res ;
}

sss sss::blah_blah()
{
  sss s = blah1() ;
  sss m = blah2() ;
  return s-m ;
}

int main()
{
  for(int i=0; i<34; ++i)
    putchar(sss::blah_blah().a < sss::blah_blah().b ? 'a' : 'b') ;
  putchar('\n') ;
  return 0 ;
}
