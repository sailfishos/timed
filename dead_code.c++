#if 0
  bool service3_result = Voland::bus().registerService(Voland::ta_service()) ;
  if(!service3_result)
  {
    log_critical("can't register service '%s' on bus '%s': %s", Voland::ta_service(), Voland::bus().name().QC, Voland::bus().lastError().message().QC) ;
    exit(1) ;
  }
#endif

#if 0
fake_dialog_ui::fake_dialog_ui(int ac, char **av) : QCoreApplication(ac, av)
{
  new fake_dialog_ui_adapter(this) ;
  bool result = Alarm::register_dialogui_service(this) ;
  if(!result)
  {
    cout << av[0] << ": can't register, bye..." ;
    exit(1) ;
  }
  bool res_serv = ALARM_DIALOG_CONNECTION.registerService(DIALOG_DBUS_SERVICE) ;
  qDebug() << "registerService" << DIALOG_DBUS_SERVICE << res_serv ;

  bool res_obj = QDBusConnection::s.....Bus().registerObject(DIALOG_DBUS_OBJPATH, this) ;
  qDebug() << "registerObject" << DIALOG_DBUS_OBJPATH<< res_obj ;
#endif

#if 0
struct fake_dialog_ui_control : public QDBusAbstractAdaptor
{
  fake_dialog_ui *owner ;

  fake_dialog_ui_control(fake_dialog_ui *o) : QDBusAbstractAdaptor(o), owner(o) { }

  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.notification_fakeuser") ;

public slots:
  void quit()
  {
    qApp->quit() ;
  }
  bool response(uint cookie, int value)
  {
    return owner->response(cookie, value) ;
  }
  bool response_last(int value)
  {
    return owner->response_last(value) ;
  }
} ;
#endif

#if 0
#include <QDebug>

static struct event_initializer_t
{
  event_initializer_t()
  {
    qDebug() << __PRETTY_FUNCTION__ ;
    register_qtdbus_metatype(Maemo::Timed::attribute_io_t) ;
    register_qtdbus_metatype(Maemo::Timed::action_io_t) ;
    register_qtdbus_metatype(Maemo::Timed::button_io_t) ;
    register_qtdbus_metatype(Maemo::Timed::recurrence_io_t) ;
    register_qtdbus_metatype(Maemo::Timed::event_io_t) ;
  }
} event_initializer ;
#endif

#if 0
 ~event_io_t() { }
#define _x(a) a(x.a)
  event_io_t(const event_io_t &x) :
    _x(ticker), _x(t_year), _x(t_month), _x(t_day), _x(t_hour), _x(t_minute), _x(t_zone),
    _x(attr), _x(flags), _x(buttons), _x(actions), _x(recrs)
  {
    qDebug() << __PRETTY_FUNCTION__ ;
  }
#undef _x
#endif

#if 0
  string err ;
  if(check_std(&err))
    throw Maemo::Timed::Exception(pretty, err.toStdString()) ;
#endif

#if 0
int Maemo::Timed::Event::which_button(const Maemo::Timed::EventButton &B) const
{
  int res = -1 ;
  for(unsigned i=0; res<0 && i<b.size(); ++i)
    if(b[i]==&B)
      res = i ;
  return res ;
}
#endif

#if 0
struct Maemo::Timed::signal_receiver : public QObject
{
  Q_OBJECT ;
public:
  signal_receiver()
  {
    const char *s = Maemo::Timed::service() ;
    const char *o = Maemo::Timed::objpath() ;
    const char *i = Maemo::Timed::interface() ;
    Maemo::Timed::bus().connect(s,o,i,"settings_changed_1",SIGNAL(settings_1(bool))) ;
  }
public Q_SIGNALS:
  void settings_1(bool) ;
} ;
#endif

#if 0
  receiver = new signal_receiver ;
#endif
#if 0
  // bus().connect(service(), objpath(), interface(), "settings_changed", this, ) ;
  // bus().connect(service(), objpath(), interface(), "settings_changed_1", this, SIGNAL(settings_changed_1(bool))) ;
  int res = QDBusConnection::sessionBus().connect(
#if 0
      service(), objpath(), interface(),
#else
      "com.nokia.time", "/com/nokia/time", "com.nokia.time",
#endif
      "settings_changed_1", this, SLOT(xxx(bool))) ;
  qDebug() << __PRETTY_FUNCTION__ ;
  qDebug() << ( isValid() ? "VALID" : "NOT VALID" ) << "res" << res ;
  qDebug() << service() << objpath() << interface() ;
#endif

#if 0
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventAction &a)
{
}
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventButton &b)
{
}
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventRecurrence &r)
{
}
#endif

#if 0
template<typename T>
static inline QDBusArgument & operator<<(QDBusArgument &out, const std::vector<T*> &v)
{
  out.beginArray(qMetaTypeId<T>()) ;
  for(typename std::vector<T*>::const_iterator it=v.begin(); it!=v.end(); ++it)
    out << **it ;
  out.endArray() ;
  return out ;
}
#endif

#if 0
QDBusArgument & operator>>(const QDBusArgument &in, Maemo::Timed::Event &)
{
  // should be never called
  return in ;
}
#endif

#if 0
#include <QDebug>

static struct nanotime_initializer_t
{
  nanotime_initializer_t()
  {
    qDebug() << __PRETTY_FUNCTION__ ;
    register_qtdbus_metatype(nanotime_t) ;
  }
} nanotime_initializer ;
#endif

#if 0
#define qtdbus_print_class_name(type_name) qDebug() << __PRETTY_FUNCTION__ << #type_name <<
#endif

#if 0
#define register_qtdbus_metatype(type_name) do { qDBusRegisterMetaType<type_name> () ; } while(0)
#endif

#if 0
#include <QDebug>

static struct nanotime_initializer_t
{
  nanotime_initializer_t()
  {
    qDebug() << __PRETTY_FUNCTION__ ;
    register_qtdbus_metatype(nanotime_t) ;
  }
} nanotime_initializer ;
#endif

#if 0
  uint add_event(const Alarm::event_io_t &x)
  {
    return timed->add_event(x).value() ;
  }
#endif

#if 0
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
#endif

#if 0
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
#endif

#if 0
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
#endif

#if 0
  uint32_t update_event(uint32_t c, const Alarm::event_t &a)
  {
    return timed->update_event(Alarm::cookie_t(c), a).value() ;
  }

  const Alarm::event_t &get_event(uint32_t c)
  {
    return timed->get_event(Alarm::cookie_t(c)) ;
  }

  bool del_event(uint32_t c)
  {
    return timed->del_event(Alarm::cookie_t(c)) ;
  }

  QVector<int> query_event(int start_time, int stop_time, int flag_mask, int flag_want)
  {
    return timed->query_event(start_time,stop_time, flag_mask, flag_want) ;
  }
  bool set_snooze(uint snooze) { return timed->set_snooze(snooze) ; }
  uint get_snooze() { return timed->get_snooze() ; }

signals:
  void queue_status_ind(int alarms, int desktop, int actdead, int noboot) ;
#endif

#if 0
      log_debug("e=%p", e) ;
      log_debug("cookie=%d", e->cookie.value()) ;
      log_debug("From '%p'", old_state) ;
      log_debug("To '%p'", new_state) ;
      log_debug("From '%s'", state_name(old_state)) ;
      log_debug("To '%s'", state_name(new_state)) ;
#endif

#if 0
    ostringstream os ;
    typedef deque <pair <event_t*, state*> >::const_iterator iterator ;
    bool start = true ;
    for(iterator it=transition_queue.begin(); it!=transition_queue.end(); ++it, start=false)
    {
      cookie_t cookie=it->first->cookie ;
      state *current = it->first->st, *requested = it->second ;
      // const char *current = it->first->st->name ;
      // const char *requested = it->second->name ;
      if(!start)
        os << ", " ;
#define state_name(p) (p?p->name:"null")
      os << "[" << cookie.value() << "," << state_name(current) << "]->" << state_name(requested) ;
#undef state_name
    }
    log_debug("request queue: %s", os.str().c_str()) ;
#endif

#if 0
    static ContextProvider::Property x239("UserAlarm.NextCookie") ;
    x239.setValue(239) ;
#endif

#if 0
      e->attr.txt["CREDENTIALS"] = string(credentials.toUtf8().constData());
#endif

#if 0
  event_t::event_t(const event_io_t *eio)
  : ticker(eio->ticker),
    t(eio->t_year, eio->t_month, eio->t_day, eio->t_hour, eio->t_minute),
    attr(eio->attr),
    dialog(eio->dialog),
    actions(eio->actions),
    recrs(eio->recrs),
    trigger(0),
    last_triggered(0),
    to_be_snoozed(0)
  {
    tz = eio->t_zone.isEmpty() ? NULL : strdup(eio->t_zone.toStdString().c_str()) ;
    st = NULL ;
    dialog_req_watcher = NULL ;
  }
#endif

#if 0
  event_t::~event_t()
  {
    if(tz)
      free((void*)tz) ;
  }
#endif

#if 0
  event_t::event_t(const iodata::record *r)
  {
  }
#endif

      // e->trigger = (ticker_t) ee->get("trigger")->value() ;
      // e->snoozed_trigger = (ticker_t) ee->get("snoozed_trigger")->value() ;

#if 0
      e->t_year = ee->get("t_year")->value() ;
      e->t_month = ee->get("t_month")->value() ;
      e->t_day = ee->get("t_day")->value() ;
      e->t_hour = ee->get("t_hour")->value() ;
      e->t_minute = ee->get("t_minute")->value() ;
#endif

#if 0
      string tz_s = ee->get("tz")->str() ;
      e->tz = tz_s.length() > 0 ? strdup(tz_s.c_str()) : NULL ;
#endif

#if 0
      e->dialog.load(ee->get("dialog")->rec()) ;
#endif

#if 0
      e->recr_count = ee->get("recr_count")->value() ;
#endif

#if 0
      log_debug("e=%p", e) ;
      log_debug("cookie=%d", e->cookie.value()) ;
      log_debug("From '%p'", e->get_state()) ;
#endif

#if 0
      request_state(e, ee->get("due_state")->value() ? "DUE" : "NEW") ;
#endif

#if 0
  bool event_t::action_comparison_t::operator()(unsigned i, unsigned j)
  {
    return actions[i].cred_key() < actions[j].cred_key() ;
  }
#endif

#if 0
  bool event_t::operator()(unsigned i, unsigned j)
  {
    log_info("comparison of %u and %u called (keys are '%s' and '%s')", i, j, actions[i].cred_key().c_str(), actions[j].cred_key().c_str()) ;
    return actions[i].cred_key() < actions[j].cred_key() ;
  }
#endif

#if 0
    sort(r->begin(), r->end(), action_comparison) ;
#endif

#if 0
  void event_t::run_actions(uint32_t mask)
  {
    // log_debug("event=%d, actions.size()=%d", cookie.value(), actions.size()) ;
    for(unsigned i=0; i<actions.size(); ++i)
    {
      const action_t &a = actions[i] ;
      if((a.flags & mask)==0)
        continue ;
      log_info("executing action %d[%d]", cookie.value(), i) ;

      // we want to detect, it the exeption was thrown in the daemon itself or in a child
      pid_t daemon = getpid() ;

      try
      {
        if(a.flags & (ActionFlags::DBus_Method | ActionFlags::DBus_Signal))
          execute_dbus(a) ;
        if(a.flags & ActionFlags::Run_Command)
          execute_command(a) ;
      }
      catch(const event_exception &e)
      {
        log_error("action %d[%d] failed: %s", cookie.value(), i, e.message.c_str()) ;
        pid_t process = e.pid() ;
        if (process!=daemon)
        {
          log_info("terminating child (pid=%d) of daemon (pid=%d)", process, daemon) ;
          ::exit(1) ;
        }
      }
    }
  }
#endif

#if 0
    QString action_value = a.attr(key) ;
    return action_value.isEmpty() ? attr(key) : action_value ;
#endif

#if 0
  void event_t::execute_dbus(const action_t &a)
  {
    QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
    QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;
    QDBusMessage m ;
    if(a.flags & ActionFlags::DBus_Method)
    {
      QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
      QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
      m = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
    }
    else
    {
      QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
      m = QDBusMessage::createSignal(path, ifac, sgnl) ;
    }
    QDBusConnection c = (a.flags & ActionFlags::Use_System_Bus) ? QDBusConnection::systemBus() : QDBusConnection::sessionBus() ;
    QMap<QString,QVariant> param ;
    if(a.flags & ActionFlags::Send_Cookie)
      param["COOKIE"] = QString("%1").arg(cookie.value()) ;
    if(a.flags & ActionFlags::Send_Event_Attributes)
      add_strings(param, attr.txt) ;
    if(a.flags & ActionFlags::Send_Action_Attributes)
      add_strings(param, a.attr.txt) ;
    m << QVariant::fromValue(param) ;
    if(c.send(m))
      log_debug("[%d]: D-Bus Message sent", cookie.value()) ;
    else
      log_error("[%d]: Failed to send a message on D-Bus: %s", cookie.value(), c.lastError().message().toStdString().c_str()) ;
  }
  void event_t::execute_dbus(const action_t &a)
  {
    bool error = true;
    int  child = fork_and_set_credentials(a, error);

    if( child != 0 )
    {
      // parent
    }
    else if( error )
    {
      // child init failed, just terminate
      // cause of error logged @ fork_and_set_credentials()
      _exit(1);
    }
    else
    {
      // child init ok

      // set up message to be sent
      QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
      QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;

      QDBusMessage m ;

      if(a.flags & ActionFlags::DBus_Method)
      {
        QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
        QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
        m = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
      }
      else
      {
        QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
        m = QDBusMessage::createSignal(path, ifac, sgnl) ;
      }

      QMap<QString,QVariant> param ;
      if(a.flags & ActionFlags::Send_Cookie)
      {
        param["COOKIE"] = QString("%1").arg(cookie.value()) ;
      }
      if(a.flags & ActionFlags::Send_Event_Attributes)
      {
        add_strings(param, attr.txt) ;
      }
      if(a.flags & ActionFlags::Send_Action_Attributes)
      {
        add_strings(param, a.attr.txt) ;
      }
      m << QVariant::fromValue(param) ;


      // TODO: is it safe to use dbus bindings after fork?
      // TODO: do we really get a fresh private connection or not?
      // TODO: should we use libdbus? or exec a helper?

      // send using fresh private connection
      QString cname = "timed-private";
      QDBusConnection::BusType ctype = QDBusConnection::SessionBus;

      if( a.flags & ActionFlags::Use_System_Bus ) ctype = QDBusConnection::SystemBus;

      QDBusConnection c = QDBusConnection::connectToBus(ctype, cname);

      int xc = 0;

      if( !c.send(m) )
      {
        log_error("[%d]: Failed to send a message on D-Bus: %s", cookie.value(), c.lastError().message().toStdString().c_str()) ;
        xc = 1;
      }
      else
      {
        log_debug("[%d]: D-Bus Message sent", cookie.value()) ;

        // as we are about to exit immediately after queuing
        // and there seems to be no way to flush the connection
        // and be sure that we have actually transmitted the
        // message -> do a dummy synchronous query from dbus
        // daemon and hope that is enough to get the actual
        // message to be delivered ...

        QString name  = c.baseService();
        pid_t   owner = credentials_get_name_owner(c, name);

        // it should be us ...
        log_debug("my pid: %d, connection owner pid: %d", getpid(), owner);
      }

      QDBusConnection::disconnectFromBus(cname);

      _exit(xc);
    }
  }
#endif

#if 0
  void event_t::prepare_command(const action_t &a, string &cmd, string &user)
  {
    user = find_action_attribute("USER", a, false) ;
    if(user.empty())
      user = "user" ;
    cmd = find_action_attribute("COMMAND", a) ;
    if(a.flags & ActionFlags::Send_Cookie)
    {
      log_debug("cmd='%s', COOKIE to be replaced by value", cmd.c_str()) ;
      using namespace pcrecpp ;
      static RE exp("(<COOKIE>)|\\b(COOKIE)\\b", UTF8()) ;
      ostringstream decimal ;
      decimal << cookie.value() ;
      exp.GlobalReplace(decimal.str(), &cmd);
      log_debug("cmd='%s', COOKIE replaced", cmd.c_str()) ;
    }

#if 0
    QString cookie_val = QString("%1").arg(cookie.value()) ;
    if(a.attr(action_flags::Send_Cookie))
    {
      static QRegExp cookie_exp("(<COOKIE>)|\\b(COOKIE)\\b") ;
      while(cookie_exp.indexIn(cmd) != -1)
      {
        for(int i=1; i<=2; i++)
          if(int len = cookie_exp.cap(i).length())
          {
            int pos = cookie_exp.pos(i) ;
            cmd.replace(pos, len, cookie_val) ;
            break ;
          }
      }
    }

#endif


#if 0
    qDebug() << "cmd" << cmd ;
#endif

#if 0
    char *sys_cmd = strdup(cmd.toStdString().c_str()) ;
    log_assert(sys_cmd) ;
    int res = system(sys_cmd) ;// XXX: yes, yes, I know! TODO: fork + su as 'user' + exec
    log_info("Event [%d] in %s, executing: %s", cookie.value(), st->name, sys_cmd) ;
    if(res!=0)
      log_info("Event [%d] in %s, executing '%s' returning %d", cookie.value(), st->name, sys_cmd, res) ;
    free(sys_cmd) ;
#endif
  }
#endif

#if 0
  pid_t event_t::fork_and_set_credentials_v2(const action_t &action, bool &error)
  {
    pid_t pid = fork() ;

    if (pid<0) // can't fork
    {
      log_error("fork() failed: %m") ;
      return error = false, pid ;
    }
    else if (pid>0) // parent
    {
      log_info("forked successfully, child pid: '%d'", pid) ;
      st->om->emit_child_created(cookie.value(), pid) ;
      return error = false, pid ;
    }
    else // child
    {
      log_info("event [%u]: in child process", cookie.value()) ;

      if (setsid() < 0) // detach from session
      {
        log_error("setsid() failed: %m") ;
        return error = true, pid ;
      }

      if(! drop_privileges(action))
      {
        log_error("can't drop privileges") ;
        return error = true, pid ;
      }

      if(! accrue_privileges(action))
        log_warning("can't accrue privileges, still continuing") ;

      // Skipping all the uid/gid settings, because it's part of
      // credentials_t::apply() (or it should be)

      // TODO: go to home dir _here_ !

      // That's it then, isn't it?

      return error = false, pid ;
    }
  }
#endif

#if 0
  int event_t::fork_and_set_credentials(const action_t &action, bool &error)
  {
    // assume failure
    int    pid  = -1;
    bool   err  = true;

    string user = find_action_attribute("USER", action, false) ;
    string cred = attr(string("CREDENTIALS"));

    struct passwd *pw = 0;

    // FIXME: remove debug logging later
    log_debug("as user: %s", user.c_str());
    log_debug("credentials: %s", cred.c_str());

    // bailout if credentials attribute is not set
    if( cred.empty() )
    {
      log_warning("no credential attribute, action skipped") ;
      goto cleanup;
    }

    // get user details if user attribute is set
    if( !user.empty() && !(pw = getpwnam(user.c_str())) )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("getpwnam(%s) failed: %m", user.c_str()) ;
      goto cleanup;
    }

    // FORK: both child and parent will return with the
    // pid returned by fork(), the caller must inspect
    // also the error parameter to know whether the
    // child initialization was succesful or not

    // parent process

    if( (pid = fork()) != 0 )
    {
      if( pid < 0 )
      {
        // TODO: do log_xxx functions preserve errno?
        log_error("fork() failed: %m");
        goto cleanup;
      }

      // child was successfully started
      st->om->emit_child_created(cookie.value(), pid) ;
      err = false;
      goto cleanup;
    }

    // child process

    // detach from session
    if( setsid() < 0 )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("setsid() failed: %m") ;
      goto cleanup;
    }

    {
      creds_t creds1 = creds_gettask(getpid()) ;
      const char *c1 = credentials_to_string(creds1) ;
      log_debug("child old creds: '%s'", c1) ;
      // take stored client credentials in to use
      if( !credentials_set(QString::fromUtf8(cred.c_str())) )
      {
        log_error("credentials_set() failed") ;
        goto cleanup;
      }
      creds_t creds2 = creds_gettask(getpid()) ;
      const char *c2 = credentials_to_string(creds2) ;
      log_debug("child new creds: '%s'", c2) ;
    }

    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;

    ruid = euid = suid = -1;
    rgid = egid = sgid = -1;

    // only effective uid/gid is set, change real
    // and saved ones too

    if( (getresgid(&rgid, &egid, &sgid) < 0) ||
        (getresuid(&ruid, &euid, &suid) < 0) )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("getresgid() / getresuid() failed: %m") ;
      goto cleanup;
    }

    if( setresgid(egid, egid, egid) < 0 )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("setresgid(%d) failed: %m", (int)egid) ;
      log_error("uid was: r=%d, e=%d, s=%d", ruid, euid, suid);
      log_error("gid was: r=%d, e=%d, s=%d", rgid, egid, sgid);
      goto cleanup;
    }
    if( setresuid(euid, euid, euid) < 0 )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("setresuid(%d) failed: %m", (int)euid) ;
      log_error("uid was: r=%d, e=%d, s=%d", ruid, euid, suid);
      log_error("gid was: r=%d, e=%d, s=%d", rgid, egid, sgid);
      goto cleanup;
    }

    // update for later use
    if( (getresgid(&rgid, &egid, &sgid) < 0) ||
        (getresuid(&ruid, &euid, &suid) < 0) )
    {
      // TODO: do log_xxx functions preserve errno?
      log_error("getresgid() / getresuid() failed: %m") ;
      goto cleanup;
    }

    // FIXME: debug block, remove later
    {
      log_debug("uid now: r=%d, e=%d, s=%d", ruid, euid, suid);
      log_debug("gid now: r=%d, e=%d, s=%d", rgid, egid, sgid);
    }

    // if user attribute was not set, we will use the home directory
    // of the user id effective after setting the credentials
    if( pw == 0 )
    {
      uid_t uid = geteuid();

      if( !(pw = getpwuid(uid)) )
      {
        // TODO: do log_xxx functions preserve errno?
        log_error("getpwuid(%d) failed: %m", (int)uid);
        goto cleanup;
      }
    }

    log_debug("workdir: %s", pw->pw_dir);

    // set home directory as current working directory
    if( chdir(pw->pw_dir)<0 )
    {
      const char fallback[] = "/";

      // TODO: do log_xxx functions preserve errno?
      log_warning("chdir(\"%s\") failed: %m, trying \"%s\"", pw->pw_dir, fallback);

      if( chdir(fallback)<0 )
      {
        // TODO: do log_xxx functions preserve errno?
        log_error("chdir(\"%s\") failed: %m", fallback);
        goto cleanup;
      }
    }

    // if user attribute was set, we will try to do the
    // uid and gid setting after setting the credentials
    // this allows actions added from root process to
    // be executed as user, but not the other way around
    if( !user.empty() )
    {
      if( setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) < 0 )
      {
        // TODO: do log_xxx functions preserve errno?
        log_error("setresgid(%d) failed: %m", (int)pw->pw_gid) ;
        log_error("uid was: r=%d, e=%d, s=%d", ruid, euid, suid);
        log_error("gid was: r=%d, e=%d, s=%d", rgid, egid, sgid);
        goto cleanup;
      }

      if( setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) < 0 )
      {
        // TODO: do log_xxx functions preserve errno?
        log_error("setresuid(%d) failed: %m", (int)pw->pw_uid) ;
        log_error("uid was: r=%d, e=%d, s=%d", ruid, euid, suid);
        log_error("gid was: r=%d, e=%d, s=%d", rgid, egid, sgid);
        goto cleanup;
      }

      // FIXME: debug block, remove later
      {
        if( (getresgid(&rgid, &egid, &sgid) < 0) ||
            (getresuid(&ruid, &euid, &suid) < 0) )
        {
          // TODO: do log_xxx functions preserve errno?
          log_error("getresgid() / getresuid() failed: %m") ;
          goto cleanup;
        }
        log_debug("uid now: r=%d, e=%d, s=%d", ruid, euid, suid);
        log_debug("gid now: r=%d, e=%d, s=%d", rgid, egid, sgid);
      }
    }

    // if we got here, all of the child process initialization
    // was succesfully completed
    err = false;

cleanup:
    return error = err, pid;
  }
#endif

#if 0
  void event_t::execute_command(const action_t &a)
  {
    string cmd, user ;
    prepare_command(a, cmd, user) ;
    errno = 0 ;
    struct passwd *pw = getpwnam(user.c_str()) ;
    if(pw==NULL)
      throw event_exception((string)"getpwname() failed"+strerror(errno)) ;
    pid_t pid = fork() ;
    if(pid<0)
      throw event_exception((string)"fork() failed"+strerror(errno)) ;
    log_info("forked successfully, fork() returned '%d'", pid) ;
    if(pid>0)
    {
      st->om->emit_child_created(cookie.value(), pid) ;
      return ;
    }
    // child process
    try
    {
      if(setsid()<0)
        throw event_exception((string)"setsid() failed"+strerror(errno)) ;
      if(user!="root")
      {
        if(chdir(pw->pw_dir)<0)
          throw event_exception((string)"chdir('"+pw->pw_dir+"') failed"+strerror(errno)) ;
        if(setgid(pw->pw_gid)<0)
          throw event_exception((string)"setgid() failed"+strerror(errno)) ;
        if(setuid(pw->pw_uid)<0)
          throw event_exception((string)"setuid() failed"+strerror(errno)) ;
      }
      execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;
      throw event_exception((string)"execl('/bin/sh', '-c', '"+cmd+"') failed"+strerror(errno)) ;
    }
    catch(const event_exception &e)
    {
      log_error("event %d, child process failed: %s", cookie.value(), e.message.c_str()) ;
      exit(1) ;
    }
  }
  void event_t::execute_command(const action_t &a)
  {
    string cmd, user ;

    // TODO: since we no longer allow everybody to request execution
    //       as somebody else it makes little sense to default to
    //       "user" if USER attribute is not set -> the user set
    //       from prepare_command is not used -> remove the whole
    //       parameter? yes...
    prepare_command(a, cmd, user) ;

    log_debug("execute: %s", cmd.c_str());

    bool error = true;
    int  child = fork_and_set_credentials(a, error);

    // parent
    if( child != 0 )
    {
      if( child < 0 )
      {
        log_error("execute: could not create child process");
      }
      else
      {
        log_debug("execute: child pid = %d", child);
      }
      return;
    }

    // child
    if( error )
    {
      log_error("execute: child init failure");
    }
    else
    {
      log_debug("execute: child init OK");
      // exec*() calls return only on failure
      log_debug("execute: %s", cmd.c_str());
      execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;

      // TODO: do log_xxx functions preserve errno?
      log_error("%s: failed: %m", cmd.c_str());
    }
    _exit(1);
  }
#endif

//    r->add("dialog", dialog.save()) ;
    // io_state *iost = dynamic_cast<io_state*> (st) ;
#if 0
    r->add("dialog_time", iost->save_in_due_state() ? 0 : last_triggered.value()) ;
#endif

#if 0
namespace Alarm
{
  namespace event_flags
  {
    enum
    {
      Snoozing        = _last_client_event_flag << 1,
      Recurring       = _last_client_event_flag << 2,
      Empty_Recurring = _last_client_event_flag << 3
    } ;
  }
}
#endif

#if 0
#define SEPARATOR " "

/* ------------------------------------------------------------------------- *
 * credentials_get_name_owner
 * ------------------------------------------------------------------------- */

pid_t
credentials_get_name_owner(const QDBusConnection &bus, const QString &name)
{
  pid_t   result    = -1; // assume failure

  QString service   = "org.freedesktop.DBus";
  QString path      = "/org/freedesktop/DBus";
  QString interface = "org.freedesktop.DBus";
  QString method    = "GetConnectionUnixProcessID";
  // "GetConnectionUnixUser" gives us the client UID as UInt32
  // It seems, we can't get GID just by asking dbus daemon.

  QDBusMessage req  = QDBusMessage::createMethodCall(service,
                                                     path,
                                                     interface,
                                                     method);
  req << name;

  QDBusMessage rsp = bus.call(req);

  // FIXME: there must be more elegant ways to handle error
  // replies and type errors...

  if( rsp.type() != QDBusMessage::ReplyMessage )
  {
    log_error("%s: did not get a valid reply", CSTR(method));
  }
  else
  {
    QList<QVariant> args = rsp.arguments();

    if( args.isEmpty() )
    {
      log_error("%s: reply has no return values", CSTR(method));
    }
    else
    {
      bool ok = false;
      int pid = rsp.arguments().first().toInt(&ok);
      // int or unsigned? toInt or toUInt ?

      if( !ok )
      {
        log_error("%s: return values is not an integer", CSTR(method));
      }
      else
      {
        result = pid;
      }
    }
  }

  return result;
}
#endif // if 0 else

#if 0

/* ------------------------------------------------------------------------- *
 * xrealloc
 * ------------------------------------------------------------------------- */

static
bool
xrealloc(void *pptr, size_t size)
{
  // *pptr is not changed if false is returned

  bool  res  = true;
  void *prev = *(void **)pptr;
  void *curr = 0;

  // if malloc(0) does (and it can) return non-NULL
  // values, realloc(NULL, 0) will too -> explicitly
  // handle new size of 0 as free ...

  if( size == 0 )
  {
    free(prev);
  }
  else if( (curr = realloc(prev, size)) == 0 )
  {
    res = false, curr = prev;
  }

  *(void **)pptr = curr;
  return res;
}

/* ------------------------------------------------------------------------- *
 * xappend
 * ------------------------------------------------------------------------- */

static
bool
xappend(char **pstr, const char *add)
{
  // *pstr is not changed if false is returned

  char  *res = 0;
  char  *cur = *pstr;

  size_t a = cur ? strlen(cur) : 0;
  size_t b = add ? strlen(add) : 0;

  if( (res = (char *)malloc(a + b + 1)) == 0 )
  {
    goto cleanup;
  }

  memcpy(res+0, cur, a);
  memcpy(res+a, add, b);
  res[a+b] = 0;

  *pstr = res;

cleanup:

  return (res != 0);
}

/* ------------------------------------------------------------------------- *
 * credentials_to_string
 * ------------------------------------------------------------------------- */

char *credentials_to_string(creds_t creds)
{
  bool          success = false;
  char         *result  = 0;

  creds_type_t  cr_type = CREDS_BAD;
  creds_value_t cr_val  = CREDS_BAD;
  char         *cr_str  = 0;
  size_t        cr_len  = 32;

  int i,rc;

  if( (cr_str = (char *)malloc(cr_len)) == 0 )
  {
    goto cleanup;
  }

  for( i = 0; (cr_type = creds_list(creds, i,  &cr_val)) != CREDS_BAD ; ++i )
  {
    if( (rc = creds_creds2str(cr_type, cr_val, cr_str, cr_len)) < 0 )
    {
      log_error("%s: failed", "creds_creds2str");
      goto cleanup;
    }

    if( (size_t)rc >= cr_len )
    {
      cr_len = (size_t)rc + 1;

      if( !xrealloc(&cr_str, cr_len) )
      {
        goto cleanup;
      }

      if( creds_creds2str(cr_type, cr_val, cr_str, cr_len) != rc )
      {
        log_error("%s: failed", "creds_creds2str");
        goto cleanup;
      }
    }

    if( result != 0 && !xappend(&result, SEPARATOR) )
    {
      goto cleanup;
    }
    if( !xappend(&result, cr_str) )
    {
      goto cleanup;
    }
  }

  success = true;

cleanup:

  if( !success )
  {
    // all or nothing
    free(result), result = 0;
  }

  free(cr_str);
  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_from_string
 * ------------------------------------------------------------------------- */

static
creds_t
credentials_from_string(const char *input)
{
  bool     error = true;   // assume failure
  creds_t  creds = 0;      // value to return

  char    *work  = 0;      // non const copy of the input string
  char    *now;            // parsing pointers
  char    *zen;

  if( !input || !(work = strdup(input)) )
  {
    goto cleanup;
  }

  creds = creds_init(); // no checking: NULL is valid credential too

  for( now = work; now; now = zen )
  {
    creds_type_t  c_type = CREDS_BAD;
    creds_value_t c_val  = CREDS_BAD;

    if( (zen = strstr(now, SEPARATOR)) != 0 )
    {
      *zen++ = 0;
    }

    if( (c_type = creds_str2creds(now, &c_val)) == CREDS_BAD )
    {
      log_error("%s: %s -> %s", "creds_str2creds", "now", "bad type");
      goto cleanup;
    }
    if( c_val == CREDS_BAD )
    {
      log_error("%s: %s -> %s", "creds_str2creds", "now", "bad value");
      goto cleanup;
    }

    if( creds_add(&creds, c_type, c_val) == -1 )
    {
      log_error("%s: failed", "creds_add");
      goto cleanup;
    }
  }

  error = false;

cleanup:

  // all or nothing
  if( error ) creds_free(creds), creds = 0;

  free(work);

  return creds;
}

/* ------------------------------------------------------------------------- *
 * credentials_get_from_pid
 * ------------------------------------------------------------------------- */

static
QString
credentials_get_from_pid(pid_t pid)
{
  QString  result; // assume null string

  creds_t  creds = 0;
  char    *text  = 0;

  if( (creds = creds_gettask(pid)) == 0 )
  {
    log_error("%s: failed", "creds_gettask");
    goto cleanup;
  }

  if( (text = credentials_to_string(creds)) == 0 )
  {
    log_error("could not convert credentials to text");
    goto cleanup;
  }

  result = QString::fromUtf8(text);

cleanup:

  free(text);
  creds_free(creds);

  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_get_from_dbus
 * ------------------------------------------------------------------------- */

QString
credentials_get_from_dbus(QDBusConnection &bus,
                          const QDBusMessage &msg)
{
  /* FIXME: this makes a synchronous roundtrip to dbus daemon
   * and back during which time the timed process will be blocked.
   *
   * Note: We can't really handle this asynchronously without
   * handling the whole add_event asynchronously and this would
   * require modifications to the timed event state machine and
   * delaying sending add_event replies from QDBusAbstractAdaptor.
   * At the moment I do not know how to handle either of those ...
   * ;-(
   */

  QString  result; // assume null string

  pid_t    owner  = -1;
  QString  sender = msg.service(); /* returns "sender" on inbound messages
                                    * and "service" on outbound messages
                                    * which saves one QString object and
                                    * confuses at least me ... */

  if( (owner = credentials_get_name_owner(bus, sender)) == -1 )
  {
    log_error("could not get owner of dbus name");
    goto cleanup;
  }

  result = credentials_get_from_pid(owner);

cleanup:

  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_set
 * ------------------------------------------------------------------------- */

bool
credentials_set(QString credentials)
{
  // assume failure
  bool          success = false;

  creds_t       cr_want = 0;
  creds_t       cr_have = 0;

  creds_type_t  cr_type = CREDS_BAD;
  creds_value_t cr_val  = CREDS_BAD;

  int i;
  char t[64];

  if( credentials.isEmpty() ) // null string is also empty
  {
    log_error("not setting empty/null credentials");
    goto cleanup;
  }

  if( (cr_want = credentials_from_string(UTF8(credentials))) == 0 )
  {
    log_error("failed to convert string to credentials");
    goto cleanup;
  }

  if( creds_set(cr_want) < 0 )
  {
    log_error("%s: failed", "creds_set");
    goto cleanup;
  }

  if( (cr_have = creds_gettask(0)) == 0 )
  {
    log_error("%s: failed", "creds_gettask");
    goto cleanup;
  }

  // assume success at this point
  success = true;

  // check if we actually have all the credentials requested
  for( i = 0; (cr_type = creds_list(cr_want, i, &cr_val)) != CREDS_BAD ; ++i )
  {
    if( creds_have_p(cr_have, cr_type, cr_val) )
    {
      // remove credential -> the set will be empty if
      // we got only credentials we asked for
      creds_sub(cr_have, cr_type, cr_val);
    }
    else
    {
      // missing a required credential
      success = false;

      // TODO: is creds_creds2str() guaranteed to return valid C-string?
      *t=0, creds_creds2str(cr_type, cr_val, t, sizeof t);
      log_error("failed to acquire credential: %s", t);
    }
  }

  // iterate credentials we have, but did not ask for
  for( i = 0; (cr_type = creds_list(cr_have, i, &cr_val)) != CREDS_BAD ; ++i )
  {
    success = false;

    // TODO: is creds_creds2str() guaranteed to return valid C-string?
    *t=0, creds_creds2str(cr_type, cr_val, t, sizeof t);
    log_error("failed to drop credential: %s", t);
  }

cleanup:

  creds_free(cr_want);
  creds_free(cr_have);

  return success;
}

#endif // DEAD_CODE

#if 0
    log_info("comparison of %u and %u called (keys are '%s' and '%s')",
       i, j, event->actions[i].cred_key().c_str(), event->actions[j].cred_key().c_str()) ;
#endif

#if 0
  int res = QObject::connect(cnt, SIGNAL(dateTimeChanged(QDateTime, int, int)), this, SLOT(old_nitz_signal(QDateTime, int, int))) ;
  if(!res)
    log_error("can't connect to the old nitz signal 'dateTimeChanged(QDateTime,int,int)'") ;
  else
    log_debug("connected to the old nitz signal 'dateTimeChanged(QDateTime,int,int)'") ;
#endif

#if 0
void cellular_handler::send_signal()
{
  // log_debug() ;
  // log_debug("cellular_time: %s", string_q_to_std(cellular_time->dateTime().toString()).c_str()) ;
  // log_debug("cellular_time->timezone(): %d", cellular_time->timezone()) ;
  // log_debug("cellular_time->dst(): %d", cellular_time->dst()) ;
  log_debug("Calling 'old_nitz_signal' manually") ;
  this->old_nitz_signal(cnt->dateTime(), cnt->timezone(), cnt->dst()) ;
  // log_debug() ;
}
#endif

#if 0
void cellular_handler::invoke_signal()
{
  QMetaObject::invokeMethod(this, "send_signal", Qt::QueuedConnection) ;
}
#endif

#if 0
void cellular_handler::old_nitz_signal(QDateTime dt, int timezone, int dst)
{
  log_debug("QDateTime=%ld timezone=%d dst=%d", dt.isValid() ? dt.toTime_t() : -239, timezone, dst) ;
  cellular_info_t ci ;

  // First create a time stamp. No information from cellular daemon,
  // thus we just fake it, taking the current monotonic time.
  ci.timestamp_value = nanotime_t::monotonic_now() ;

  // 1. Offset of the timezone.
  // Invalid means '-1' probably ?
  // Anyway it should divisible by a 15 mins unit
  if(timezone != -1 && timezone % (15*60) == 0)
  {
    ci.flag_offset = true ;
    ci.offset_value = timezone /* 15*60 */ ;

  // 2. UTC time, only possible, of the timezone offset is valid

    if(dt.isValid())
    {
      ci.flag_time = true ;
      nanotime_t sent = nanotime_t::from_time_t(dt.toTime_t()) ;
      ci.time_at_zero_value = sent - ci.timestamp_value ;

  // 3. DST flag, only possible if 1&2 above are given

      if(dst!=-1) // '-1' is probably the 'invalid' value
      {
        ci.flag_dst = true ;
        ci.dst_value = dst ; // probably in hours
      }
    }
  }
  else
    log_error("invalid timezone=%d value from cellular daemon", timezone) ;

  // Now the network information
  Cellular::NetworkOperator no ;
  QString mcc = no.mcc() ;
  QString mnc = no.mnc() ;

  // MCC is being sent as a string (why?)
  // Thus we have to convert it to an integer

  if(!mcc.isEmpty())
  {
    string mcc_str = string_q_to_std(mcc) ;
    int mcc_dec ;
    static pcrecpp::RE integer = "(\\d+)" ;
    if(integer.FullMatch(mcc_str, &mcc_dec))
    {
      ci.flag_mcc = true ;
      ci.mcc_value = mcc_dec ;

      // If we have a valid MCC, let's check for MNC
      if(!mnc.isEmpty())
      {
        ci.flag_mnc = true ;
        ci.mnc_value = string_q_to_std(mnc) ;
      }
    }
    else
      log_error("invalid mcc value from cellular daemon: '%s'", mcc.toStdString().c_str()) ;
  }

  log_debug() ;
  emit cellular_data_received(ci) ;
  log_debug() ;
}
#endif

#if 0
namespace Cellular
{
  struct NetworkTimeInfo
  {
    QDateTime x_dateTime;
    int x_offsetFromUtc;
    int x_daylightAdjustment;
    qlonglong x_timestampSeconds, x_timestampNanoSeconds ;
    QString x_mcc, x_mnc ;

    QDateTime dateTime() const { return x_dateTime; }
    int offsetFromUtc() const { return x_offsetFromUtc; }
    int daylightAdjustment() const { return x_daylightAdjustment; }

    qlonglong timestampSeconds() const { return x_timestampSeconds ; }
    qlonglong timestampNanoSeconds() const { return x_timestampNanoSeconds ; }
    QString mobileCountryCode() const { return x_mcc ; }
    QString mobileNetworkCode() const { return x_mnc ; }
    bool isValid() const { return true ; }
    QString toString() const
    {
      // log_debug() ;
      QString res = "INVALID" ;
      // log_debug() ;
      if(isValid())
      {
        // log_debug() ;
        res = dateTime().toString(Qt::ISODate) ;
        // log_debug() ;
        res += QString(" off: %1 dst: %2").arg(offsetFromUtc()).arg(daylightAdjustment()) ;
        // log_debug() ;
        res += QString(" timestamp: %1/%2").arg(timestampSeconds()).arg(timestampNanoSeconds()) ;
        // log_debug() ;
        res += QString(" mcc: '%1' mnc: '%2'").arg(mobileCountryCode()).arg(mobileNetworkCode()) ;
        // log_debug() ;
      }
      // log_debug() ;
      return res ;
    }
  } ;
}
#endif

#if 0
  Q_INVOKABLE void send_signal() ;
#endif

#if 0
  void invoke_signal() ;
#endif

#if 0
  void old_nitz_signal(QDateTime dt, int timezone, int dst) ;
#endif

#if 0
    void ping()
    {
      // log_debug("timer: %s", timer->status().toStdString().c_str()) ;
      log_debug() ;
      Maemo::Timed::Voland::ActivationInterface ifc ;
      log_debug() ;
      if(!ifc.isValid())
        log_critical("Activation interface is not valid: %s", ifc.lastError().message().toStdString().c_str()) ;
      else
      {
        log_debug("Calling pid() asyncronosly") ;
        ifc.pid_async() ;
        log_debug() ;
      }
    }
#endif

#if 0
string nt_to_str(const nanotime_t &t)
{
  return str_printf("%d.%09d", (int)t.sec(), t.nano()) ;
}
#endif

#if 0
  clear_invokation_flag() ;
#endif

#if 0 // moved to timed
void source_settings::invoke_signal(const nanotime_t &back)
{
  systime_back += back ;
  if(signal_invoked)
    return ;
  signal_invoked = true ;
  int methodIndex = o->metaObject()->indexOfMethod("send_time_settings()") ;
  QMetaMethod method = o->metaObject()->method(methodIndex);
  method.invoke(o, Qt::QueuedConnection);
}
#endif

#if 0
source *source_settings::src_(const string &key) const
{
  map<string,source*>::const_iterator it = src.find(key) ;
  log_assert(it!=src.end(), "the ley %s not found", key.c_str()) ;
  return it->second ;
}
#endif

#if 0
int source_settings::check_timezone(string zone)
{
  return check_target(symlink_target(zone)) ;
}
#endif

// check_offset

#if 0 // NOT NEEDED
    switch(op_time)
    {
      case Op_Set_Time_Nitz:
        signal_needed = signal_needed || ! time_nitz ;
        signal_utc = nitz_utc->available() ;
        break ;
      case Op_Set_Time_Manual:
        signal_needed = signal_needed || time_nitz ;
        break ;
      case Op_Set_Time_Manual_Val:
        signal_needed = signal_utc = true ;
    }
#endif
#if 0 // NOT_DONE_YET
  string tz ;

  tz = tz_single->guess_timezone(&ci) ;
  // XXX TODO -- guess timezone based on mcc

  if(!tz.empty())
  {
    cellular_zone->value = tz ;
    if(local_cellular)
    {
    }
  }
#endif

#if 0 // moved to timed
  bool signal_invoked ;
  nanotime_t systime_back ;
#endif

  // source *src_(const string &) const ;
#if 0 // moved to timed
  void invoke_signal(const nanotime_t &) ;
  void invoke_signal() { nanotime_t zero ; invoke_signal(zero) ; }
  void clear_invokation_flag() { signal_invoked = false ; systime_back.invalidate() ; }
#endif

#if 0
void state_queued::timer_start()
{
  int time_to_wait = -1 ;
  if(queue.empty())
    log_info("go to sleep, no alarm queued") ;
  else
  {
    time_to_wait = queue.begin()->first - now() ;
    if(time_to_wait<0)
    {
      log_info("no sleep today: an alarm is %d seconds overdue", -time_to_wait) ;
      time_to_wait = 0 ;
    }
    else
      log_info("go to sleep, next alarm in %d seconds", time_to_wait) ;
  }
  if(0<=time_to_wait)
    alarm_timer->start(time_to_wait*1000) ;
  if(0!=time_to_wait) // -1=infinity, or >0 -- sleeping
    om->send_queue_context() ;
#if 0
  static unsigned next_user_cookie=0 ;
  next_user_cookie ++ ;
  static /*may be auto as well*/ ContextProvider::Property cookie_property("UserAlarm.NextCookie") ;
  cookie_property.setValue(next_user_cookie) ; // unsetValue() set the value to null, but not deletes it
#endif
}
#endif

#if 0
void state_disabled::enable_user_alarms()
{
  // alarm_engine_pause x(om) ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    om->request_state(*it, "NEW") ;
  om->process_transition_queue() ;
}

void state_waitconn::internet_connection_established()
{
  // alarm_engine_pause x(om) ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    om->request_state(*it, "NEW") ;
  om->process_transition_queue() ;
}
#endif

#if 0
  else if(e->attr(Alarm::event_flags::Postpone_If_Missed))
    next_state = "POSTPONED" ;
#endif

#if 0
void state_postponed::enter(event_t *e)
{
  const char *next_state = "NEW" ;
  if(false) // what should be the condition for that ?
    next_state = "DUE" ;
  om->request_state(e, next_state) ;
}
#endif

#if 0
    e->t = best ;
#endif

#if 0
    // and now we can use sign bit as alignment flag
    if(b.attr(button_flags::Aligned_Snooze))
      snooze_length = - snooze_length ; // if it's zero, nothing happens
#endif

#if 0
    if(!e->snoozed_trigger.is_valid()) // first snooze
      e->snoozed_trigger = e->trigger ;
#endif

#if 0
    e->snooze = 0 ; // no snooze
    e->snoozed_trigger = ticker_t(0) ;
#endif

#if 0
  uint32_t off = e->flags & EventFlags::Cluster_Mask ;
  log_debug("off=0x%08X", off) ;
  for(uint32_t b; b = (off ^ (off-1)), b &= off ; off ^= b)
    om->clusters[b]->leave(e),
    log_debug("b=0x%08X, off=0x%08X", b, off) ;
#endif

// int alarm_engine_pause::counter = 0 ;
//
#if 0
    set<event_pair> queue, user, conn ;
#endif

    // bool save_in_due_state() { return false ; }
    //
#if 0
    void disable_user_alarms() ;
    void internet_connection_lost() ;
#endif

#if 0
  struct state_postponed : public state
  {
    state_postponed(machine *am) : state("POSTPONED",am) {}
    void enter(event_t *e) ;
  } ;
#endif
    // bool save_in_due_state() { return false ; }
    // bool save_in_due_state() { return false ; }
    // bool save_in_due_state() { return false ; }
    // bool save_in_due_state() { return false ; }
    // bool save_in_due_state() { return true ; }
    // bool save_in_due_state() { return true ; }
    // bool save_in_due_state() { return true ; }
    // void enter(event_t *e) ;
#if 0
  ContextProvider::Service *context_service = new ContextProvider::Service(Maemo::Timed::bus()) ; // not deleted at all
  context_service->setAsDefault() ;
  ContextProvider::Property("UserAlarm.NextCookie") ;
  // moved up: (new ContextProvider::Service(Maemo::Timed::bus()))->setAsDefault() ;
#endif

#if 0
void Timed::load_rc_()
{
  iodata::record c ;
  timed_rc_type->check_record_after_read(&c, "const_t") ;

#define USE_NEW_VALIDATOR_FROM_FILE 1

#if 0
  iodata::bytes *location = dynamic_cast<iodata::bytes*> (c.x["rc_location"]) ;
#else
  string rc_file_path = c.get("rc_location")->str() ;
#endif

#if ! USE_NEW_VALIDATOR_FROM_FILE
#if 0
  ifstream if_rc(location->x.c_str()) ;
#else
  ifstream if_rc(rc_file_path.c_str()) ;
#endif

  iodata::record *r = NULL ;
  try
  {
    iodata::parser p = if_rc ;
    p.parse() ;
    r = p.detach() ;
    timed_rc_type->check_record_after_read(r, "timed_rc_t") ;
  }
  catch(iodata::exception &e)
  {
    log_error("Loading rcfile: %s", e.info().c_str()) ;
#if 0
    log_info("The file %s is corrupted or not present, using default values", location->x.c_str()) ;
#else
    log_info("The file %s is corrupted or not present, using default values", rc_file_path.c_str()) ;
#endif
    delete r ;
    r = new iodata::record ;
    timed_rc_type->check_record_after_read(r, "timed_rc_t") ;
  }
#else // USE_NEW_VALIDATOR_FROM_FILE
  string msg ;
  iodata::record *r = timed_rc_type->record_from_file(rc_file_path.c_str(), "timed_rc_t", msg) ;
  log_assert(r!=NULL, "rc file is not loaded") ;
  if(!msg.empty())
    log_info("Loading rc file: %s", msg.c_str()) ;
#endif

  alarm_queue_path = r->get("queue_path")->str() ;
  settings_path = r->get("settings_path")->str() ;
  threshold_period_long = r->get("queue_threshold_long")->value() ;
  threshold_period_short = r->get("queue_threshold_short")->value() ;
  ping_period = r->get("notification_ping_period")->value() ;
  ping_max_num = r->get("notification_ping_max_num")->value() ;
  save_time_path = r->get("saved_utc_time_path")->str() ;

  ostringstream str ;
  iodata::output o(str) ;
  o.output_record(r) ;
  log_debug("Current rc settings: %s", str.str().c_str()) ;

  delete r ;
}
#endif // 0

#if 0
void Timed::load_alarms()
{
  iodata::record *q = NULL ;
  try
  {
    ifstream fs ;
    // fs.exceptions(ifstream::failbit | ifstream::badbit) ;
    fs.open(alarm_queue_path.c_str()) ;
    iodata::parser p(fs) ;
    p.parse() ;
    q = p.detach() ;
    alarm_queue_type->check_record_after_read(q, "alarm_queue_t") ;
  }
  catch(iodata::exception &e)
  {
    log_critical("Loading alarm queue failed: %s", e.info().c_str()) ;
    delete q ;
    return ;
  }
  am->load(q) ;
}
#endif // 0

#if 0

#if ! USE_NEW_VALIDATOR_FROM_FILE
void Timed::save_alarm_queue()
{
  iodata::record *queue = am->save() ;
  alarm_queue_type->check_record_before_write(queue, "alarm_queue_t") ;

  ostringstream os ;
  iodata::output out(os) ;
  out.output_record(queue) ;

  try
  {
    ofstream of ;
    of.exceptions(ofstream::failbit | ofstream::badbit) ;
    of.open(alarm_queue_path.c_str(), ofstream::out) ;
    of << os.str() ;
    of.close() ;
    log_info("alarm queue written") ;
  }
  catch(const ofstream::failure &e)
  {
    log_critical("can't save alarm queue: %s", e.what()) ;
    log_info("alarm queue follows: %s", os.str().c_str()) ;
  }
  delete queue ;
}
#else // USE_NEW_VALIDATOR_FROM_FILE
void Timed::save_alarm_queue()
{
  iodata::record *queue = am->save() ;
  string serialized ;
  bool success = alarm_queue_type->record_to_file(alarm_queue_path.c_str(), "alarm_queue_t", queue, serialized) ;

  if(success)
  {
    log_info("alarm queue written") ;
  }
  else
  {
    log_critical("can't save alarm queue") ;
    log_info("alarm queue follows: %s", serialized.c_str()) ;
  }
  delete queue ;
}
void Timed::save_settings()
{
  iodata::record *r = settings->save() ;
  string serialized ;
  bool success = settings_type->record_to_file(settings_path.c_str(), "settings", r, serialized) ;

  if(success)
  {
    log_info("wall clock settings written") ;
  }
  else
  {
    log_critical("can't save wall clock settings") ;
    log_info("lost settings: %s", serialized.c_str()) ;
  }
  delete r ;
}
#endif // USE_NEW_VALIDATOR_FROM_FILE

#endif // 0

#if 0
void Timed::load_settings()
{
  log_debug() ;
  string msg ;
  iodata::record *r = settings_type->record_from_file(settings_path.c_str(), "settings", msg) ;
  log_assert(r!=NULL, "can't get settings") ;
  if(!msg.empty())
    log_info("Loading rc file: %s", msg.c_str()) ;
  settings = new source_settings(this) ;
  settings->load(r) ;
  delete r ;
  log_debug() ;
}
#endif // 0

#if 0
  int res = old_tz ? unsetenv("TZ") : setenv("TZ", old_tz, true) ;
#endif
#if 0
  iodata::storage *mcc_tz = new iodata::storage ;
#if 0

void tz_state::gmt_offset(time_t now, int new_offset, int new_dst_flag)
{
  if(offset==new_offset && dst_flag==new_dst_flag)
  {
    update_time(now) ;
    return ;
  }

  offset = new_offset ;
  dst_flag = new_dst_flag ;

  compute_candidates(now) ;
}

void tz_state::compute_candidates(time_t now)
{
  candidates.clear() ;
  if(const olson *s=cnifo->sinle_zone())
  {
    candidates.insert(s) ;
    return ;
  }
}

#endif
  mcc_tz->set_validator(validator(), "tz_single_t") ;
  mcc_tz->set_primary_path("/usr/share/tzdata-timed/single.data") ;
  iodata::record *list_r = mcc_tz->load() ;
  delete mcc_tz ;
#endif

#if 0

// If nothing else works, just use Greenwich (why not Moscow, or Asia/Gaza)
struct tz_total_fail_t
{
  // string guess_timezone(const cellular_info_t *) { return "Fixed/UTC" ; }
  // Fixed/* not available yet
  // Anyway, it's a bad idea, because we want to have a _geographic_, not fixed zone.
  // Need some better solution!
  //
  // Maybe just a special case: auto_dst in on, and the zone is Fixed/UTC ?
  string guess_timezone(const cellular_info_t *) { return "Europe/London" ; }
} ;

struct tz_default_t : public tz_oracle_t
{
  // Timo will implement customization settings and put it here
  string guess_timezone(const cellular_info_t *) { return "Europe/Helsinki" ; }
} ;

#endif

#if 0

struct tz_pure_offset_t : public tz_oracle_t
{
  string guess_timezone(const cellular_info_t *) ;
  tz_pure_offset_t() ;
  // map<int, string> mcc_to_tz ;
} ;

// ---------------------------------------------------------

struct tz_mcc_info

struct tz_country
{
  const olson *single_zone() ;
  const olson *main_zone() ;
} ;

struct tz_country
{
  tz_country *cinfo ;
  const olson *tz ;
  set<const olson *> candidates, foreign ;
  vector<const olson *> history ;
  int offset, dst_flag ;
  time_t valid_from, valid_to ;

  bool valid_at(time_t now) { return valid_from <= now && now < valid_to ; }
  bool compute_candidates(time_t now) ;

  void gmt_offset(time_t now, int new_offset, int new_dst_flag) ;
  void user_input(time_t now, const olson *utz) ;
  void update_time(time_t now) ;
} ;

struct tz_cellular_engine
{
  int mcc ;
  bool connected ;
  map<int, tz_local_state*> ;
} ;
#endif

#if 0
  enum Policy { Default, Ignore, Handle } ;
  set_policy(int signo, Policy policy) ;
#endif

#if 0
  int diff(const ticker_t &y) const { return y.value()-value() ; }
  ticker_t add(int diff) const { return ticker_t(value()+diff) ; }
#endif

#if 0
int add_event(const char *title)
{
  Maemo::Timed::Event e ;
  time_t ticker = time(NULL)+8 ;

  Maemo::Timed::Event::Action &a_trig = e.addAction() ;
  a_trig.whenTriggered() ;
  a_trig.setSendCookieFlag() ;
  a_trig.runCommand("echo cookie=[COOKIE]=<COOKIE> TRIGGERED $(date) >> /tmp/aa") ;
  e.setAlarmFlag() ;

  if(title!=NULL)
  {
    e.setAttribute("TITLE", title) ;
    e.setAlignedSnoozeFlag() ;

    if(pcrecpp::RE("boot").PartialMatch(title))
    {
      e.setBootFlag() ;
      ticker += 3*60 - 8 ;
    }

    Maemo::Timed::Event::Button &b1 = e.addButton() ;
    Maemo::Timed::Event::Button &b2 = e.addButton() ;
    Maemo::Timed::Event::Button &b3 = e.addButton() ;
    Maemo::Timed::Event::Button &b4 = e.addButton() ;

    b1.setSnooze(10) ;
    b2.setSnooze(17) ;
    b3.setSnooze(60) ;
    (void)b4 ;
    // b4 doesn't snooze: it closes the dialog

    Maemo::Timed::Event::Action &a1 = e.addAction() ;
    a1.runCommand("echo [COOKIE] BUTTON #1 $(date) >> /tmp/aa") ;
    a1.setSendCookieFlag() ;
    a1.whenButton(b1) ;

    Maemo::Timed::Event::Action &a2 = e.addAction() ;
    a2.runCommand("echo [COOKIE] BUTTON #2 $(date) >> /tmp/aa") ;
    a2.setSendCookieFlag() ;
    a2.whenButton(b2) ;

    Maemo::Timed::Event::Action &a3 = e.addAction() ;
    a3.runCommand("echo [COOKIE] BUTTON #3 $(date) >> /tmp/aa") ;
    a3.setSendCookieFlag() ;
    a3.whenButton(b3) ;

    Maemo::Timed::Event::Action &a4 = e.addAction() ;
    a4.runCommand("echo [COOKIE] BUTTON #4 $(date) >> /tmp/aa") ;
    a4.setSendCookieFlag() ;
    a4.whenButton(b1) ;

    Maemo::Timed::Event::Action &a_0 = e.addAction() ;
    a_0.runCommand("echo [COOKIE] CANCELED BY USER $(date) >> /tmp/aa") ;
    a_0.setSendCookieFlag() ;
    a_0.whenSysButton(0) ;

    Maemo::Timed::Event::Action &a_sys = e.addAction() ;
    a_sys.runCommand("echo [COOKIE] FIRST SYSTEM BYTTON $(date) >> /tmp/aa") ;
    a_sys.setSendCookieFlag() ;
    a_sys.whenSysButton(1) ;
  }
  else // no title, let's add a recurrence
  {
    Maemo::Timed::Event::Recurrence &r = e.addRecurrence() ;
    r.everyMonth() ;
    r.everyDayOfMonth() ;
    r.everyDayOfWeek() ;
    r.addHour(12), r.addMinute(34) ;
  }

  e.setTicker(ticker) ;

  Maemo::Timed::Interface ifc ;
  if(!ifc.isValid())
  {
    qDebug() << "not valid interface:" << ifc.lastError() ;
    return 1 ;
  }

  QDBusReply<uint> res = ifc.add_event_sync(e) ;
  if(!res.isValid())
  {
    qDebug() << "call failed:" << res.error().message() ;
    return 1 ;
  }

  qDebug() << "added event, cookie:" << res.value() ;
  return 0 ;
}
#endif

#if 0
#include <QDebug>
static struct reminder_initializer_t
{
  reminder_initializer_t()
  {
    qDebug() << __PRETTY_FUNCTION__ ;
    register_qtdbus_metatype(Maemo::Timed::Voland::Reminder) ;
    register_qtdbus_metatype(Maemo::Timed::Voland::button_io_t) ;
  }
} reminder_initializer ;
#endif
