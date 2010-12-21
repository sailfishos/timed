#ifndef TIMED_MEMORY_H
#define TIMED_MEMORY_H

#include <QCoreApplication>
#include <QTimer>

class memory_t : public QCoreApplication
{
  Q_OBJECT ;
  QTimer *timer ;
public:
  memory_t(int &ac, char ** &av) : QCoreApplication(ac, av)
  {
    timer = new QTimer ;
    timer->setSingleShot(true) ;
    connect(timer, SIGNAL(timeout()), this, SLOT(quit())) ;
    timer->start(1000) ;
  }
  virtual ~memory_t()
  {
    delete timer ;
  }
} ;

#endif
