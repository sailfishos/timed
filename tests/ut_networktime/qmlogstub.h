/***************************************************************************
**                                                                        **
**  Copyright (C) 2013 Jolla Ltd.                                         **
**  Contact: Petri M. Gerdt <petri.gerdt@jollamobile.com>                 **
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

#ifndef QMLOGSTUB
#define QMLOGSTUB

#include <cstdio>

#define log_debug(msg, ...) \
    do { \
        printf("log_debug: " msg "\n", ##__VA_ARGS__); \
    } while (0)

#define log_warning(msg, ...) \
    do { \
        printf("log_warning: " msg "\n", ##__VA_ARGS__); \
    } while (0)

#define log_error(msg, ...) \
    do { \
        printf("log_error: " msg "\n", ##__VA_ARGS__); \
    } while (0)

#endif
