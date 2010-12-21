#include "memory.h"

int main(int ac, char **av)
{
  memory_t *app = new memory_t(ac, av) ;
  int res = app->exec() ;

  delete app ;

  return res ;
}

