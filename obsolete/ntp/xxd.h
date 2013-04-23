#include <vector>
#include <string>

#ifndef MAEMO_TIMED_XXD_H
#define MAEMO_TIMED_XXD_H

void xxd(const char *data, unsigned len, unsigned bpl, std::vector<std::string> &res) ;
std::string str_printf(const char *format, ...) ;

#endif//MAEMO_TIMED_XXD_H
