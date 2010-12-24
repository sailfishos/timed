#ifndef CLUSTER_H
#define CLUSTER_H

struct abstract_cluster_t
{
  machine_t *machine ;
  uint32_t bit ;
  string name ;
  abstract_cluster_t(machine_t *owner, uint32_t b, const char *n) : machine(owner), bit(b), name(n) { }
  virtual ~abstract_cluster_t() { }
  virtual void enter(event_t *e) = 0 ;
  virtual void leave(event_t *) { }
} ;
struct cluster_queue_t : public abstract_cluster_t
{
  QMap<QString,QVariant> alarm_triggers ;
  cluster_queue_t(machine_t *m) : abstract_cluster_t(m, EventFlags::Cluster_Queue, "QUEUE") { }
  virtual ~cluster_queue_t() { }
  void enter(event_t *e) ;
  void leave(event_t *e) ;
} ;

struct cluster_dialog_t : public abstract_cluster_t
{
  set<event_t *> bootup_events ;
  cluster_dialog_t(machine_t *m) : abstract_cluster_t(m, EventFlags::Cluster_Dialog, "DIALOG") { }
  virtual ~cluster_dialog_t() { }
  void enter(event_t *e) ;
  void leave(event_t *e) ;
  bool has_bootup_events() ;
} ;

#endif//CLUSTER_H
