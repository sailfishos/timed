#include <stdlib.h>
#include <stdarg.h>

#include <qmlog>

#include "xxd.h"

using namespace std ;

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
    ++ iteration ;
  } while(!printed) ;

  return formatted ;
}

static char print_char(unsigned char ch)
{
  if (ch<0x20 or ch>0x7E)
    return '.' ;
  return ch ;
}

void xxd(const char *data, unsigned len, unsigned bpl, vector<string> &res)
{
  for (unsigned k=0; k<len; k+=bpl)
  {
    unsigned remains = len - k ;
    unsigned L = remains < bpl ? remains : bpl ;
    unsigned P = bpl - L ;
    string s1, s2 ;
    for (unsigned ii=0, i; i=k+ii, ii<L; ++ii)
    {
      s1 += str_printf("%02X ", data[i]) ;
      s2 += str_printf("%c", print_char(data[i])) ;
    }
    for (unsigned i=0; i<P; ++i)
    {
      s1 += "   " ;
      s2 += "." ;
    }
    res.push_back(s1 + "| " + s2 + str_printf(" | %2x", k)) ;
  }
}
