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

#if 0
namespace Alarm
{
  class Event
  {
    class Button ;
    class Action ;
    class Recurrence ;

    enum Action_State
    {
      State_NEW = 0,
      State_TRIGGERED,
      State_BUTTON_FIRST,
      State_BUTTON_LAST = State_BUTTON_1 + N_BUTTONS - 1 
    } ;
    class Action
    {
      Event *owner ;
      unsigned state_mask ;
      Action(Event *event) ;
    public:
      bool link(Button *button) ;
      bool link(enum Action_State state) ;
    } ;
  public:
    Event() ;
   ~Event() ;
    Button *add_button(QString label) ;
    int find_button(Button *) ;
    Action *add_action_exec(QString cmd) ;
    Action *add_action_dbus(QString service, QString path, QString interface, QString method) ;
  private:
    QVector <Button*> buttons ;
}
#endif

#if 0
#define CUST_FILE "/etc/clockd/clockd-settings.default"
QByteArray customization_settings::get_hash()
{
  QByteArray hash;
  QFile custFile(CUST_FILE);
  if (custFile.open(QIODevice::ReadOnly))
  {
    QByteArray bytes = custFile.read(10*1000);
    hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);
  }
  else
  {
    log_error("Could not open customization file "CUST_FILE);
  }
  return hash.toBase64();
}

iodata::storage* customization_settings::get_storage()
{
    iodata::storage *s = new iodata::storage();
    s->set_primary_path("/etc/timed-cust.rc");
    s->set_validator("/usr/share/timed/typeinfo/timed-cust-rc.type", "timed_cust_rc_t");
    return s;
}

void customization_settings::check_customization(int ac, char **av)
{
    /**
     * If we have --cust, read the values from the script file,
     * and save them with iodata::storage.
     */
    const char cust[] = "--cust";
    for (int i=1; i < ac; i++)
    {
      if (!strncmp(cust, av[i], strlen(cust)))
      {
        log_debug("--cust");

        // Get the envs to a record.
        struct customization_settings settings;
        char *envVal = NULL;

        envVal = getenv("CLOCKD_NET_TIME");
        if (envVal)
        {
            log_debug("CUST CLOCKD_NET_TIME: %s", envVal);
          if (!strncmp(envVal, "yes", 3) ||
              !strncmp(envVal, "no", 2) ||
              !strncmp(envVal, "disabled", 8))
          {
              settings.time_nitz_str = envVal;
          }
          else
          {
            log_error("Invalid CLOCKD_NET_TIME value");
          }
        }
        else
        {
          log_error("no CLOCKD_NET_TIME");
        }

        envVal = getenv("CLOCKD_TIME_FORMAT");
        if (envVal)
        {
          log_debug("CUST CLOCKD_TIME_FORMAT: %s", envVal);
          if (!strncmp(envVal, "R", 1) ||
              !strncmp(envVal, "r", 1))
          {
            settings.format_24_str = envVal;
          }
          else
          {
            log_error("Invalid CLOCKD_TIME_FORMAT value");
          }

        }
        else
        {
          log_error("no CLOCKD_TIME_FORMAT");
        }
        envVal = getenv("CLOCKD_DEFAULT_TZ");
        if (envVal)
        {
            log_debug("CUST CLOCKD_DEFAULT_TZ: %s", envVal);
          settings.default_tz = envVal;
        }

        iodata::record *r = settings.save();

        QByteArray hash = get_hash();
        log_debug("CUST hash: %s", hash.data());
        r->add("hash", hash.data());

        // Save the record to a storage.
        iodata::storage *s = get_storage();
        s->save(r);
        delete s;
        delete r;

        exit(0);
      }
    }

    /**
     * Check that have up to date customization data.
     * If not, fork timed with --cust and do it.
     */
    QByteArray hash = get_hash();
    if (!hash.isEmpty())
    {
      iodata::storage *custStorage = get_storage();
      iodata::record *custRecord = custStorage->load();
      bool newCust = true;
      if (custStorage->source() != -1 && custRecord)
      {
        const iodata::item *sum = custRecord->get("hash");
        if (sum && sum->str() == hash.data())
        {
          newCust = false;
        }
        else
        {
          newCust = true;
        }
      }
      delete custRecord;
      delete custStorage;

      if (newCust)
      {
        log_debug("CUST forking...");
        QString cmd = "source "CUST_FILE;
        cmd += "; ";
        cmd += av[0];
        cmd += " --cust";
        log_debug("CUST %s", cmd.toAscii().data());
        system(cmd.toAscii().data());
      }
    }
}

customization_settings::customization_settings()
{
  log_debug() ;

  net_time_enabled = true;
  time_nitz = true;
  format_24 = false;
  default_tz = "327";
}

void customization_settings::load(const iodata::record * record)
{
  log_debug() ;

  time_nitz_str = record->get("time_nitz")->str().c_str();
  format_24_str = record->get("format_24")->str().c_str();
  default_tz = record->get("default_tz")->str().c_str();

  log_debug("CUST time_nitz_str: %s", time_nitz_str.toAscii().data());
  log_debug("CUST format_24_str: %s", format_24_str.toAscii().data());
  log_debug("CUST default_tz: %s", default_tz.toAscii().data());

  if (time_nitz_str == "disabled")
  {
    net_time_enabled=false;
  }
  else
  {
    net_time_enabled=true;
  }
  if (time_nitz_str == "yes")
  {
    time_nitz = true;
  }
  else if (time_nitz_str == "no")
  {
    time_nitz = false;
  }
  if (format_24_str == "r")
  {
    format_24 = false;
  }
  else if (format_24_str == "R")
  {
    format_24 = true;
  }

  log_debug("CUST time_nitz: %d", time_nitz);
  log_debug("CUST format_24: %d", format_24);
  log_debug("CUST net_time_enabled: %d", net_time_enabled);

  valueMap.insert(QString("CLOCKD_NET_TIME"), time_nitz_str);
  valueMap.insert(QString("CLOCKD_TIME_FORMAT"), format_24_str);
  valueMap.insert(QString("CLOCKD_DEFAULT_TZ"), default_tz);
}

QMap<QString, QVariant> customization_settings::get_values()
{
  return valueMap;
}

void customization_settings::load()
{
    iodata::storage *custStorage = get_storage();
    iodata::record *custRecord = custStorage->load();
    load(custRecord);
    delete custRecord;
    delete custStorage;
}

iodata::record* customization_settings::save()
{
  iodata::record *r = new iodata::record;
  r->add("time_nitz", time_nitz_str.toAscii().data());
  r->add("format_24", format_24_str.toAscii().data());
  r->add("default_tz", default_tz.toAscii().data());
  return r;
}
#endif

#if 0
  QMap<QString, QVariant> customization_values()
  {
    log_debug() ;
    return timed->cust_settings->get_values();
  }
#endif

#if 0
  void machine::call_returned(QDBusPendingCallWatcher *w)
  {
    if(watcher_to_event.count(w)==0)
    {
      log_critical("unknown QDBusPendingCallWatcher %p", w) ;
      return ;
    }
    event_t *e = watcher_to_event[w] ;
    if(e->dialog_req_watcher!=w)
    {
      log_critical("oops, will terminate in a moment...") ;
      log_debug("w=%p, e=%p, e->cookie=%d", w, e, e->cookie.value()) ;
    }
    log_assert(e->dialog_req_watcher==w) ;
    e->process_dialog_ack() ;
  }
#endif

#if 0
  void event_t::process_dialog_ack() // should be move to state_dlg_requ?
  {
    QDBusPendingReply<bool> reply = *dialog_req_watcher ;
    bool ok = reply.isValid() && reply.value() ;
    if(ok)
    {
      st->om->request_state(this, "DLG_USER") ;
      st->om->process_transition_queue() ;
    }
    else
      log_error("Requesting event %d dialog failed: %s", cookie.value(), reply.error().message().toStdString().c_str()) ;
    delete dialog_req_watcher ;
    dialog_req_watcher = NULL ;
  }
#endif

#if 0
    r->add("default_snooze", default_snooze_value) ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    r->add("alarms", flt_alrm->is_open ? 1 : 0) ;
#endif

#if 0
    default_snooze_value = r->get("default_snooze")->value() ;
#endif

#if 0
    for(unsigned i=0; i < a->size(); ++i)
    {
      const iodata::record *ee = a->get(i)->rec() ;
      cookie_t c(ee->get("cookie")->value()) ;
      event_t *e = new event_t ;
      events[e->cookie = c] = e ;

      e->ticker = ticker_t(ee->get("ticker")->value()) ;
      e->t.load(ee->get("t")->rec()) ;

      e->tz = ee->get("tz")->str() ;

      e->attr.load(ee->get("attr")->rec()) ;
      e->flags = ee->get("flags")->decode(event_t::codec) ;
      iodata::load(e->recrs, ee->get("recrs")->arr()) ;
      iodata::load(e->actions, ee->get("actions")->arr()) ;
      iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;
      iodata::load(e->b_attr, ee->get("b_attr")->arr()) ;
      e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
      e->tsz_max = ee->get("tsz_max")->value() ;
      e->tsz_counter = ee->get("tsz_counter")->value() ;
      e->client_creds.load(ee->get("client_creds")->rec()) ;
      e->cred_modifier.load(ee->get("cred_modifier")->arr()) ;

      const char *next_state = "START" ;

      if(e->flags & EventFlags::Empty_Recurring)
        e->invalidate_t() ;

      request_state(e, next_state) ;
    }
#endif

#if 0
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    if(r->get("alarms")->value())
      flt_alrm->open() ;
    else
      flt_alrm->close() ;
    // alarm_gate(true, r->get("alarms")->value()) ;
#endif

#if 0
    int default_snooze(int new_value=0) ;
#endif

#if 0
    void call_returned(QDBusPendingCallWatcher *) ; // rename ?
#endif

#if 0
    int default_snooze_value ;
#endif

#if 0
  int machine::default_snooze(int new_value)
  {
    if(30 <= new_value) // TODO: make it configurierable?
    {
      default_snooze_value = new_value ;
      emit queue_to_be_saved() ;
    }
    return default_snooze_value ;
  }
#endif

#if 0 // old implementation
void Timed::backup()
{
  system("mkdir /tmp/.timed-backup; cp /var/cache/timed/*.data /etc/timed.rc /etc/timed-cust.rc /tmp/.timed-backup; chmod -R 0777 /tmp/.timed-backup");
}
void Timed::backup_finished()
{
  system("rm -rf /tmp/.timed-backup");
}
void Timed::restore()
{
}
void Timed::restore_finished()
{
  system("cp -f /tmp/.timed-backup/*.data /var/cache/timed; cp -f /tmp/.timed-backup/*.rc /etc");
  backup_finished();
  QCoreApplication::exit(1);
}
#endif

#if 0
  dialog_req_watcher = NULL ;
#endif

#if 0
  if(dialog_req_watcher)
  {
    log_debug("dialog_req_watcher still alive, deleting") ;
    delete dialog_req_watcher ;
  }
#endif

#if 0
  QDBusPendingCallWatcher *dialog_req_watcher ;
#endif

#if 0 // TRIGGERED state seems to be better for this
  // Frist get rif of one time trigger info:
  e->ticker = ticker_t() ;
  e->invalidate_t() ;
#endif

#if 0
  o->save_time_to_file() ;
#endif

#if 0
  Maemo::Timed::Voland::Interface ifc ;
  if(!ifc.isValid())
  {
    string msg = Maemo::Timed::Voland::bus().lastError().message().toStdString() ;
    log_critical("Can't use voland interface: %s", msg.c_str()) ;
    return ;
  }
  Maemo::Timed::Voland::reminder_pimple_t *p = new Maemo::Timed::Voland::reminder_pimple_t ;
  log_debug("was vergessen?") ;
  p->flags = e->flags & EventFlags::Voland_Mask ;
  p->cookie = e->cookie.value() ;
  map_std_to_q(e->attr.txt, p->attr) ;
  p->buttons.resize(e->b_attr.size()) ;
  for(int i=0; i<p->buttons.size(); ++i)
    map_std_to_q(e->b_attr[i].txt, p->buttons[i].attr) ;
  QDBusPendingCall async = ifc.open_async(Maemo::Timed::Voland::Reminder(p));
  if(e->dialog_req_watcher)
  {
    log_error("orphan dialog_req_watcher=%p, e=%p, cookie=%d", e->dialog_req_watcher, e, e->cookie.value()) ;
    delete e->dialog_req_watcher ;
  }
  e->dialog_req_watcher = new QDBusPendingCallWatcher(async);
  om->watcher_to_event[e->dialog_req_watcher] = e ;
  QObject::connect(e->dialog_req_watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), om, SLOT(call_returned(QDBusPendingCallWatcher*))) ;
#endif

#if 0
  {
    om->watcher_to_event.erase(e->dialog_req_watcher) ;
    delete e->dialog_req_watcher ;
    e->dialog_req_watcher = NULL ;
  }
#endif

#if 0
  save_time_to_file_timer = new QTimer ;
  QObject::connect(save_time_to_file_timer, SIGNAL(timeout()), this, SLOT(save_time_to_file())) ;
  save_time_to_file() ;
#endif

#if 0 // XXX: remove it for ever
  save_time_path = c->get("saved_utc_time_path")->str() ;
#endif

#if 0
  cust_settings = new customization_settings();
  cust_settings->load();
#endif

#if 0
  // we can't do it here:
  //   first get dbus name (as a mutex), then fix the files
#endif

#if 0
void Timed::save_time_to_file()
{
  save_time_to_file_timer->stop() ;

  if(FILE *fp = fopen(save_time_path.c_str(), "w"))
  {
    const int time_length = 32 ;
    char value[time_length+1] ;

    time_t tick = time(NULL) ;
    struct tm tm ;
    gmtime_r(&tick, &tm) ;
    strftime(value, time_length, "%F %T", &tm) ;

    fprintf(fp, "%s\n", value) ;
    if(fclose(fp)==0)
      log_info("current time (%s) saved in %s", value, save_time_path.c_str()) ;
    else
      log_error("can't write to file %s: %m", save_time_path.c_str()) ;
  }
  else
    log_error("can't open file %s: %m", save_time_path.c_str()) ;

  save_time_to_file_timer->start(1000*3600) ; // 1 hour
}
#endif

#if 0
  customization_settings *cust_settings;
#endif

#if 0
  string save_time_path ;
#endif

#if 0
  QTimer *save_time_to_file_timer ;
#endif

#if 0
  void save_time_to_file() ;
#endif

#if 0
    map<QDBusPendingCallWatcher *, event_t *> watcher_to_event ;
#endif

#if 0
  struct state_start : public state
  {
    state_start(machine *am) : state("START",am) {}
    virtual ~state_start() { }
    void enter(event_t *e) ;
  } ;

  struct state_epoch : public gate_state
  {
    Q_OBJECT ;
    static const char *lost ;
  public:
    state_epoch(machine *am) ;
    virtual ~state_epoch() { }
  public Q_SLOTS:
    void open() ; // a virtual slot
  } ;

  struct state_new : public state
  {
    state_new(machine *am) : state("NEW",am) {}
    virtual ~state_new() { }
    void enter(event_t *e) ;
  } ;

  struct state_scheduler : public state
  {
    state_scheduler(machine *am) : state("SCHEDULER",am) {}
    virtual ~state_scheduler() {}
    void enter(event_t *e) ;
  } ;

  struct state_qentry : public state
  {
    state_qentry(machine *am) : state("QENTRY",am) {}
    virtual ~state_qentry() { }
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  } ;

  struct state_queued : public io_state
  {
    int pause_x ;
    state_queued(machine *am) ;
    virtual ~state_queued() ;
    void enter(event_t *e) ;
    void leave(event_t *e) ;
    void timer_start() ;
    void timer_stop() ;
    QTimer *alarm_timer ;
    typedef pair<ticker_t, event_t *> event_pair ;
    ticker_t next_bootup() ;
    ticker_t next_rtc_bootup() ;
    set<event_pair> queue ;
    set<event_pair> bootup ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    Q_OBJECT ;
  public Q_SLOTS:
    void engine_pause(int dx) ;
    void alarm_timeout() ;
    void filter_closed(filter_state *f_st) ;
  Q_SIGNALS:
    void sleep() ;
  } ;


  struct state_missed : public state
  {
    state_missed(machine *am) : state("MISSED",am) {}
    virtual ~state_missed() { }
    void enter(event_t *e) ;
  } ;

  struct state_due : public state
  {
    state_due(machine *am) : state("DUE",am) {}
    virtual ~state_due() { }
    void enter(event_t *e) ;
  } ;

  struct state_skipped : public state
  {
    state_skipped(machine *am) : state("SKIPPED",am) {}
    virtual ~state_skipped() {}
    void enter(event_t *e) { om->request_state(e, "SERVED") ; }
  } ;

  struct state_flt_conn : public filter_state
  {
    state_flt_conn(machine *am) : filter_state("FLT_CONN", "QENTRY", "FLT_ALRM", am) { }
    virtual ~state_flt_conn() { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_flt_alrm : public filter_state
  {
    state_flt_alrm(machine *am) : filter_state("FLT_ALRM", "QENTRY", "FLT_USER", am) { }
    virtual ~state_flt_alrm() { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_flt_user : public filter_state
  {
    state_flt_user(machine *am) : filter_state("FLT_USER", "QENTRY", "QUEUED", am) {}
    virtual ~state_flt_user() { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_snoozed : public state
  {
    state_snoozed(machine *am) : state("SNOOZED",am) {}
    virtual ~state_snoozed() { }
    void enter(event_t *e) ;
  } ;

  struct state_recurred : public state
  {
    state_recurred(machine *am) : state("RECURRED",am) {}
    virtual ~state_recurred() { }
    void enter(event_t *e) ;
    ticker_t apply_pattern(broken_down_t &t, int wday, const recurrence_pattern_t *p) ;
  } ;

  struct state_armed : public gate_state
  {
    state_armed(machine *am) : gate_state("ARMED", "TRIGGERED", am) { }
    virtual ~state_armed() { }
  private:
    Q_OBJECT ;
  } ;

  struct state_triggered : public state
  {
    state_triggered(machine *am) : state("TRIGGERED",am) {}
    virtual ~state_triggered() { }
    void enter(event_t *e) ;
  } ;

  struct state_button : public state
  {
    signed no ;
    static QString init_name(signed n) ;
    state_button(machine *am, signed n) ;
    virtual ~state_button() { }
    void enter(event_t *e) ;
  } ;

  struct state_served : public state
  {
    state_served(machine *am) : state("SERVED",am) {}
    virtual ~state_served() { }
    void enter(event_t *e) ;
  } ;

  struct state_tranquil : public io_state
  {
    state_tranquil(machine *am) : io_state("TRANQUIL", am) { }
    virtual ~state_tranquil() { }
  } ;

  struct state_removed : public state
  {
    state_removed(machine *am) : state("REMOVED",am) {}
    virtual ~state_removed() { }
    void enter(event_t *e) ;
  } ;

  struct state_finalized : public state
  {
    state_finalized(machine *am) : state("FINALIZED",am) {}
    virtual ~state_finalized() { }
    void enter(event_t *e) ;
  } ;

  struct state_aborted : public state
  {
    state_aborted(machine *am) : state("ABORTED",am) {}
    virtual ~state_aborted() { }
    void enter(event_t *e) ;
  } ;

  struct state_dlg_wait : public gate_state
  {
  public:
    state_dlg_wait(machine *am) : gate_state("DLG_WAIT", "DLG_CNTR", am) { }
    virtual ~state_dlg_wait() { }
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
  Q_SIGNALS:
    void voland_needed() ;
  private:
    Q_OBJECT ;
  } ;

  struct state_dlg_cntr : public concentrating_state
  {
    const char *back ;
    state_dlg_cntr(machine *am) : concentrating_state("DLG_CNTR", "DLG_REQU", am), back("DLG_WAIT") { }
    virtual ~state_dlg_cntr() { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void request_voland() ;
  public Q_SLOTS:
    void open() ;
    void send_back() ;
  private:
    Q_OBJECT ;
  } ;

  struct state_dlg_requ : public gate_state
  {
    state_dlg_requ(machine *am) : gate_state("DLG_REQU", "DLG_WAIT", am) { }
    virtual ~state_dlg_requ() { }
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void abort(event_t *e) ;
    Q_OBJECT ;
  public Q_SLOTS:
  } ;

  struct state_dlg_user : public gate_state
  {
    state_dlg_user(machine *am) : gate_state("DLG_USER", "DLG_WAIT", am) { }
    virtual ~state_dlg_user() { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void abort(event_t *e) ;
    Q_OBJECT ;
  public Q_SLOTS:
  } ;

  struct state_dlg_resp : public state
  {
    state_dlg_resp(machine *am) : state("DLG_RESP", am) { }
    virtual ~state_dlg_resp() { }
  } ;
#endif

#if 0
void state_start::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "EPOCH") ;
}

const char *state_epoch::lost = "/var/cache/timed/SYSTEM_TIME_LOST" ;

state_epoch::state_epoch(machine *am) :
  gate_state("EPOCH", "NEW", am)
{
  if(access(lost, F_OK) != 0)
    is_open = true ;
}

void state_epoch::open()
{
  if(!is_open && unlink(lost)<0)
    log_critical("can't unlink '%s': %m", lost) ;
  gate_state::open() ;
}


void state_new::enter(event_t *e)
{
  state::enter(e) ;
  const char *next_state = "ABORTED" ;  // TODO: make a new state "FAILED" for this case

  if(e->flags & EventFlags::In_Dialog)
  {
    ticker_t t_now = om->transition_start_time ;

    bool restore = true ;
    if(e->last_triggered.is_valid() && t_now - e->last_triggered > om->dialog_discard_threshold)
      restore = false ;

    if(restore)
    {
      e->last_triggered = t_now ;
      next_state = "DLG_WAIT" ;
    }
    else
    {
      e->flags &= ~ EventFlags::In_Dialog ;
      next_state = "SERVED" ;
    }
  }
  else if(e->has_ticker() || e->has_time() || e->has_recurrence())
    next_state = "SCHEDULER" ;
  else if(e->to_be_keeped())
    next_state = "TRANQUIL" ;

  om->request_state(e, next_state) ;
  om->process_transition_queue() ;
}

void state_scheduler::enter(event_t *e)
{
  state::enter(e) ;
  log_debug() ;
  const char *next_state = "QENTRY" ;
  if(e->has_ticker())
    e->trigger = ticker_t(e->ticker) ;
  else if(e->has_time())
  {
    struct tm T;
    T.tm_sec = 0 ;
    T.tm_min = e->t.minute ;
    T.tm_hour = e->t.hour ;
    T.tm_mday = e->t.day ;
    T.tm_mon = e->t.month - 1 ;
    T.tm_year = e->t.year - 1900 ;
    T.tm_isdst = -1 ;
    log_debug("%d-%d-%d %d:%d", e->t.year, e->t.month, e->t.day, e->t.hour, e->t.minute) ;
    if (not e->has_timezone())
      e->trigger = mktime_local(&T) ;
    else if(Maemo::Timed::is_tz_name(e->tz))
      e->trigger = mktime_oversea(&T, e->tz) ;
    else
      log_error("can't schedule an event for an unknown time zone '%s'", e->tz.c_str()) ;

    log_debug("now=%ld e->trigger=%ld diff=%ld", time(NULL), e->trigger.value(), e->trigger.value()-time(NULL)) ;
    log_debug("e->has_timezone()=%d", e->has_timezone()) ;
    if(!e->trigger.is_valid())
    {
      log_error("Failed to calculate trigger time for %s", e->t.str().c_str()) ;
      next_state = "ABORTED" ; // TODO: make a new state "FAILED" for this case
    }
  }
  else if(e->has_recurrence())
    next_state = "RECURRED" ;
  log_debug() ;
  om->request_state(e, next_state) ;
  log_debug() ;
}

void state_qentry::enter(event_t *e)
{
  state::enter(e) ;
  log_assert(e->trigger.is_valid()) ;
  om->request_state(e, "FLT_CONN") ;
}

state_queued::state_queued(machine *am) : io_state("QUEUED",am)
{
  pause_x = 0 ;
  alarm_timer = new QTimer ;
  alarm_timer->setSingleShot(true) ;
  connect(alarm_timer, SIGNAL(timeout()), this, SLOT(alarm_timeout())) ;
}

state_queued::~state_queued()
{
  delete alarm_timer ;
}

void state_queued::engine_pause(int dx)
{
  log_debug("dx=%d, current pause value: '%d', new value will be %d", dx, pause_x, pause_x+dx) ;
  if(pause_x==0)
  {
    log_assert(dx>0) ;
    timer_stop() ;
  }
  pause_x += dx ;
  if(pause_x==0)
  {
    log_assert(dx<0) ;
    // log_assert(not "debugging assert -> abort()") ;
    timer_start() ;
  }
}

void state_queued::alarm_timeout()
{
  queue_pause x(om) ;
  log_info("waked up by alarm_timer") ;
  ticker_t time_now = now() ;
  log_debug("time_now=%ld queue.begin=%ld", time_now.value(), queue.begin()!=queue.end() ? queue.begin()->first.value() : (time_t)-1) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  for(iterator it=queue.begin(); it!=queue.end() && it->first <= time_now; ++it, event_found=true)
    om->request_state(it->second, "DUE") ;
  if(event_found)
    om->process_transition_queue() ;
  else
    log_info("No due event found, need to sleep more") ;
}

void state_queued::enter(event_t *e)
{
  io_state::enter(e) ;
  queue_pause x(om) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.insert(p) ;
  if(e->flags & EventFlags::Boot)
    bootup.insert(p) ;
}

void state_queued::leave(event_t *e)
{
  queue_pause x(om) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.erase(p) ;
  bootup.erase(p) ;
  io_state::leave(e) ;
}

void state_queued::timer_start()
{
  if(queue.empty())
  {
    log_info("go to sleep, no alarm queued") ;
    om->send_queue_context() ;
    emit sleep() ;
    return ;
  }

  nanotime_t time_to_wait = nanotime_t(queue.begin()->first.value(), 0) - nanotime_t::systime_now() ;
  bool no_sleep = false ;
  if(time_to_wait<0)
  {
    log_info("no sleep today: an alarm is %s seconds overdue", (-time_to_wait).str().c_str()) ;
    no_sleep = true ;
  }
  else
    log_info("go to sleep, next alarm in %s seconds", time_to_wait.str().c_str()) ;

  if(no_sleep)
    alarm_timer->start(0) ;
  else
  {
    static const int threshold = 3600 ; // One hour
    int milisec ;
    if(time_to_wait<threshold)
      milisec = time_to_wait.sec() * 1000 + time_to_wait.nano() / 1000000 ;
    else
      milisec = (threshold-1) * 1000 ;
    log_debug("starting alarm_timer for %d milisec", milisec) ;
    alarm_timer->start(milisec) ;
    om->send_queue_context() ;
    emit sleep() ;
  }
}

void state_queued::timer_stop()
{
  alarm_timer->stop() ;
}

ticker_t state_queued::next_bootup()
{
  ticker_t tick ; // default value: invalid
  set<event_pair>::const_iterator it = bootup.begin() ;
  if(it!=bootup.end())
    tick = it->first ;
  return tick ;
}

ticker_t state_queued::next_rtc_bootup()
{
  ticker_t current_time = om->transition_start_time ;
  if(!current_time.is_valid()) // not in transition
    current_time = now() ;

  ticker_t threshold = current_time + RenameMeNameSpace::Dsme_Poweroff_Threshold ;

  ticker_t tick ; // default value: invalid
  typedef set<event_pair>::const_iterator iterator ;
  for(iterator it = bootup.begin(); it != bootup.end(); ++it)
  {
    if(it->first <= threshold)
      continue ;
    tick = it->first ;
    break ;
  }

  return tick ;
}

void state_queued::filter_closed(filter_state *f_st)
{
  log_assert(!f_st->is_open) ;
  queue_pause x(om) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  log_debug("event_found=%d", event_found) ;
  for(iterator it=queue.begin(); it!=queue.end(); ++it)
  {
    if(! f_st->filter(it->second))
      continue ;
    event_found = true ;
    log_debug("event_found=%d", event_found) ;
    log_debug("event [%u] found in state '%s', requesting staet '%s'", it->second->cookie.value(), name(), f_st->name()) ;
    om->request_state(it->second, f_st) ;
  }
  log_debug("event_found=%d", event_found) ;
  if(event_found)
    om->process_transition_queue() ;
}

bool state_flt_conn::filter(event_t *e)
{
  return (e->flags & EventFlags::Need_Connection) != 0 ;
}

bool state_flt_alrm::filter(event_t *e)
{
  return (e->flags & EventFlags::Alarm) != 0 ;
}

bool state_flt_user::filter(event_t *e)
{
  return (e->flags & EventFlags::User_Mode) != 0 ;
}

void state_missed::enter(event_t *e)
{
  state::enter(e) ;
  const char *next_state = "SKIPPED" ;

  e->flags |= EventFlags::Missed ;

  if(e->flags & EventFlags::Trigger_If_Missed)
    next_state = "ARMED" ;
  om->request_state(e, next_state) ;
}

void state_due::enter(event_t *e)
{
  state::enter(e) ;

  // assume it's not missed, this flag will be set in MISSED state
  e->flags &= ~ EventFlags::Missed ;

  const char *next_state = "ARMED" ;
  if(e->flags & EventFlags::Empty_Recurring)
    next_state = "RECURRED" ;
  else if(om->transition_started() - e->trigger > RenameMeNameSpace::Missing_Threshold)
    next_state = "MISSED" ;
  om->request_state(e, next_state) ;
  log_debug("event [%d]: e->trigger=%ld, now=%ld, state->%s", e->cookie.value(), e->trigger.value(), om->transition_started().value(), next_state) ;
  e->last_triggered = om->transition_started() ;
  e->flags &= ~ EventFlags::Snoozing ;
}

void state_snoozed::enter(event_t *e)
{
  state::enter(e) ;
  log_assert(e->to_be_snoozed > 0) ;
  // compute next trigger time and jump back to queue

  if(e->flags & EventFlags::Aligned_Snooze)
    e->ticker = ticker_align(e->last_triggered, e->to_be_snoozed, om->transition_started());
  else
    e->ticker = om->transition_started() + e->to_be_snoozed ;

  e->flags |= EventFlags::Snoozing ;
  e->to_be_snoozed = 0 ; // doesn't need it anymore
  om->request_state(e, "SCHEDULER") ;
  om->invoke_process_transition_queue() ;
}

ticker_t state_recurred::apply_pattern(broken_down_t &t, int wday, const recurrence_pattern_t *p)
{
  unsigned nxt_year = t.year + 1 ;
  if(broken_down_t::YEARX <= nxt_year)
    -- nxt_year ;
  broken_down_t started = t ;
  for(bool today_flag=true;  t.find_a_good_day(p, wday, today_flag, nxt_year) ; today_flag=false)
  {
    log_debug() ;
    broken_down_t td = t ;
    if(!today_flag)
      td.hour = td.minute = 0 ;
    while(td.find_a_good_minute(p))
    {
      struct tm tm ;
      td.to_struct_tm(&tm) ;
      log_debug("td=(%s)", td.str().c_str()) ;
      log_debug("tm=%s", tm_str(&tm).c_str()) ;
      time_t time = mktime(&tm) ;
      log_debug("time=%d", (int)time) ;
      if(time==time_t(-1))
        continue ;
      log_debug() ;
      if(time <= om->transition_started().value())
      {
        log_debug() ;
        td.increment_min(1) ;
        log_debug() ;
        continue ;
      }
      log_debug() ;
      if(!td.same_struct_tm(&tm))
      {
        td.increment_min(1) ;
        continue ;
      }
      log_debug() ;
      t = td ;
      return ticker_t(time) ;
    }
  }
  log_debug() ;
  return ticker_t(0) ;
}

void state_recurred::enter(event_t *e)
{
  state::enter(e) ;
  switch_timezone x(e->tz) ;
  broken_down_t best, now ;
  int now_wday ;
  now.from_time_t(om->transition_started(), &now_wday) ;
  ticker_t best_ticker = ticker_t(0) ;
  for(unsigned i=0; i<e->recrs.size(); ++i)
  {
    broken_down_t t = now ;
    ticker_t res = apply_pattern(t, now_wday, &e->recrs[i]) ;
    if(res.is_valid() && (!best_ticker.is_valid() || res<best_ticker))
      best = t, best_ticker = res ;
  }
  const char *next_state = "QENTRY" ;
  if(best_ticker.is_valid())
  {
    e->flags &= ~ EventFlags::Empty_Recurring ;
    e->trigger = best_ticker ;
  }
  else
  {
    e->flags |= EventFlags::Empty_Recurring ;
    e->t.year = now.year+1, e->t.month = 12, e->t.day = 31, e->t.hour = 0, e->t.minute = 42 ;
    if(e->t.is_valid()) // it valid from until 2036
      next_state = "SCHEDULER" ; // back to scheduler
    else
      next_state = "ABORTED" ; // TODO: FAILED is better here
  }
  om->request_state(e, next_state) ;
}

void state_triggered::enter(event_t *e)
{
  state::enter(e) ;

  // Frist get rid of one time trigger info:
  e->ticker = ticker_t() ;
  e->invalidate_t() ;

  if(e->flags & EventFlags::Single_Shot)
    e->recrs.resize(0) ; // no recurrence anymore
  om->request_state(e, e->flags & EventFlags::Reminder ? "DLG_WAIT" : "SERVED") ;
  om->process_transition_queue() ;
}

QString state_button::init_name(signed no)
{
  QString template_str = "BUTTON_%1_%2" ;
  return QString(template_str).arg(no>0 ? "APP" : "SYS").arg(no>0 ? no : -no) ;
}

state_button::state_button(machine *am, signed n) : state(init_name(n).toStdString().c_str(), am)
{
  no = n ;
  set_action_mask(no>0 ? ActionFlags::app_button(no) : ActionFlags::sys_button(-no)) ;
}

void state_button::enter(event_t *e)
{
  log_debug("Voland-response: cookie=%d, button-value=%d", e->cookie.value(), no) ;
  state::enter(e) ;
  int snooze_length = 0 ;

  if (e->tsz_max)
  {
    log_debug("processing restricted timeout snooze event: cookie=%d, max=%d, count=%d", e->cookie.value(), e->tsz_max, e->tsz_counter) ;
    if (no!=0)
      e->tsz_counter=0 ;
    else if (e->tsz_counter < e->tsz_max)
    {
      ++ e->tsz_counter ;
      snooze_length = e->snooze[0] ;
    }
    log_debug("processed: cookie=%d, max=%d, count=%d, snooze=%d", e->cookie.value(), e->tsz_max, e->tsz_counter, snooze_length) ;
  }
  else if (no==0)
    snooze_length = e->snooze[0] ;

  if(no==-1) // system wide default snooze button #1
    snooze_length = +1 ;
  else if(no>0)
  {
    log_assert((unsigned)no < e->snooze.size()) ;
    snooze_length = e->snooze[no] ;
  }

  // handle special value, +1 means default snooze
  // It's hardcoded for 5min for now...
  if(snooze_length==+1)
    snooze_length = om->owner->settings->default_snooze() ;

  if(snooze_length > 0)
  {
    e->to_be_snoozed = snooze_length ;
    om->request_state(e, "SNOOZED") ;
  }
  else
  {
    om->request_state(e, "SERVED") ;
  }
}

void state_served::enter(event_t *e)
{
  state::enter(e) ;
  bool recu = e->has_recurrence() ;
  bool keep = e->to_be_keeped() ;
  om->request_state(e, recu ? "RECURRED" : keep ? "TRANQUIL" : "REMOVED") ;
  om->process_transition_queue() ;
}

void state_removed::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "FINALIZED") ;
  om->process_transition_queue() ;
}

void state_finalized::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, (state*)NULL) ;
  om->process_transition_queue() ;
}

void state_aborted::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "FINALIZED") ;
  om->process_transition_queue() ;
}

void state_dlg_wait::enter(event_t *e)
{
  e->flags |= EventFlags::In_Dialog ;
  if(!is_open)
    emit voland_needed() ;
  gate_state::enter(e) ;
}

void state_dlg_cntr::open()
{
  log_debug() ;
  if (not events.empty())
    request_voland() ;
  concentrating_state::open() ;
  log_debug() ;
}

void state_dlg_cntr::send_back()
{
  log_debug() ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    om->request_state(*it, back) ;
  if (not events.empty())
    om->process_transition_queue() ;
  log_debug() ;
}

void state_dlg_cntr::request_voland()
{
  if (events.empty())
    return ; // avoid a memory leak for 'w' below.

  QList<QVariant> reminders ;
  request_watcher_t *w = new request_watcher_t(om) ;
  Maemo::Timed::Voland::Interface ifc ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
  {
    event_t *e = *it ;
    w->attach(e) ;
    Maemo::Timed::Voland::reminder_pimple_t *p = new Maemo::Timed::Voland::reminder_pimple_t ;
    log_debug("was soll ich schon vergessen haben?") ;
    p->flags = e->flags & EventFlags::Voland_Mask ;
    log_debug() ;
    p->cookie = e->cookie.value() ;
    log_debug() ;
    map_std_to_q(e->attr.txt, p->attr) ;
    log_debug() ;
    p->buttons.resize(e->b_attr.size()) ;
    log_debug() ;
    for(int i=0; i<p->buttons.size(); ++i)
      map_std_to_q(e->b_attr[i].txt, p->buttons[i].attr) ;
    log_debug() ;
    log_debug() ;
    Maemo::Timed::Voland::Reminder R(p) ;
    reminders.push_back(QVariant::fromValue(R)) ;
#if 1 // GET RID OF THIS PIECE SOON !
    log_debug() ;
    Maemo::Timed::Voland::Reminder RR = R ;
    ifc.open_async(RR); // fire and forget
    log_debug() ;
#endif
    log_debug() ;
  }
  log_debug() ;
  QDBusPendingCall async = ifc.open_async(reminders) ;
  w->watch(async) ;
}

void state_dlg_requ::enter(event_t *e)
{
  gate_state::enter(e) ;
}

void state_dlg_requ::abort(event_t *e)
{
  if (e->request_watcher)
    e->request_watcher->detach(e) ;

  Maemo::Timed::Voland::Interface ifc ;
  ifc.close_async(e->cookie.value()) ;

  io_state::abort(e) ;
}

void state_dlg_user::abort(event_t *e)
{
  Maemo::Timed::Voland::Interface ifc ;
  ifc.close_async(e->cookie.value()) ;

  io_state::abort(e) ;
}

void cluster_queue::enter(event_t *e)
{
  log_debug() ;
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  if(alarm && !empty_r)
  {
    QString key = QString("%1").arg(e->cookie.value()) ;
    uint64_t value = (uint64_t)nanotime_t::NANO * e->trigger.value() ;
    alarm_triggers.insert(key, value) ;
    log_debug("inserted %s=>%lld, state=%s", key.toStdString().c_str(), value, e->st->name()) ;
    om->context_changed = true ;
  }
}

void cluster_queue::leave(event_t *e)
{
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  log_debug() ;
  if(alarm && !empty_r)
  {
    log_debug() ;
    QString key = QString("%1").arg(e->cookie.value()) ;
    alarm_triggers.remove(key) ;
    log_debug("removed %s=>'' state=%s", key.toStdString().c_str(), e->st ? e->st->name() : "null") ;
    om->context_changed = true ;
  }
}

void cluster_dialog::enter(event_t *e)
{
  bool is_bootup = e->flags & EventFlags::Boot ;
  if(is_bootup)
    log_debug("insertng [%d]  to  cluster_dialog", e->cookie.value()) ;
  if(is_bootup)
    bootup_events.insert(e) ;
}

void cluster_dialog::leave(event_t *e)
{
  if(bootup_events.count(e))
    log_debug("removing [%d] from cluster_dialog", e->cookie.value()) ;
  bootup_events.erase(e) ;
}

bool cluster_dialog::has_bootup_events()
{
  return !bootup_events.empty() ;
}
#endif

#if 0
  struct state
  {
    state(const char *, machine *) ;
    machine *om ;
    string sname ;
    uint32_t action_mask ;
    const char *name() const { return sname.c_str() ; }
    uint32_t get_action_mask() { return action_mask ; }
    void set_action_mask(uint32_t a) { action_mask = a ; }
    virtual void enter(event_t *) ;
    virtual void leave(event_t *) ;
    virtual uint32_t cluster_bits() { return 0 ; }
    virtual ~state() ;
  } ;

  struct io_state : public QObject, public state
  {
    set <event_t *> events ;
    io_state(const char *, machine *, QObject *p = NULL) ;
    void enter(event_t *) ;
    void leave(event_t *) ;
    virtual void abort(event_t *) ;
    // virtual bool save_in_due_state() = 0 ;
    virtual ~io_state() { }
    Q_OBJECT ;
  } ;

  struct gate_state : public io_state
  {
    gate_state(const char *name, const char *nxt, machine *, QObject *p=NULL) ;
    virtual ~gate_state() { }
    string nxt_state ;
    bool is_open ;
    void enter(event_t *) ;
    Q_OBJECT ;
  public Q_SLOTS:
    void close() ;
    virtual void open() ;
  Q_SIGNALS:
    void closed() ;
    void opened() ;
  } ;

  struct concentrating_state : public gate_state
  {
    concentrating_state(const char *name, const char *nxt, machine *m, QObject *p=NULL) : gate_state(name, nxt, m, p) { }
    virtual ~concentrating_state() { }
    Q_OBJECT ;
  public Q_SLOTS:
    void open() ;
  } ;

  struct filter_state : public gate_state
  {
    filter_state(const char *name, const char *retry, const char *nxt, machine *, QObject *p=NULL) ;
    virtual ~filter_state() { }
    string next ;
    virtual bool filter(event_t *) = 0 ;
    void enter(event_t *) ;
    Q_OBJECT ;
  Q_SIGNALS:
    void closed(filter_state *state) ;
  private Q_SLOTS:
    void emit_close() { emit closed(this) ; }
  } ;
#endif

#if 0
  struct abstract_cluster
  {
    machine *om ;
    uint32_t bit ;
    string name ;
    abstract_cluster(machine *m, uint32_t b, const char *n) : om(m), bit(b), name(n) { }
    virtual ~abstract_cluster() { }
    virtual void enter(event_t *e) = 0 ;
    virtual void leave(event_t *) { }
  } ;
#endif

#if 0
  struct machine : public QObject
  {
    machine(const Timed *daemon) ;
    virtual ~machine() ;

    const Timed *owner ;
    int next_cookie ;
    uint32_t flags ;
    map<string, state*> states ;
    map<cookie_t, event_t*> events ;
    map<uint32_t, abstract_cluster*> clusters ;
    deque <pair <event_t*, state*> > transition_queue ;
    ticker_t transition_start_time ;
    bool context_changed ;
    map<int, state*> button_states ;
    int dialog_discard_threshold ;
    int32_t signalled_bootup ;
    state_epoch *epoch ;

    bool transition_in_progress() { return transition_start_time.is_valid() ; }
    ticker_t transition_started() { return transition_start_time ; }
    uint32_t attr(uint32_t mask) ;
    cookie_t add_event(const Maemo::Timed::event_io_t *, bool process_queue, const credentials_t *pcreds, const QDBusMessage *message) ;
    void add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message) ;
    void query(const QMap<QString,QVariant> &words, QList<QVariant> &res ) ;
    void get_event_attributes(cookie_t c, QMap<QString,QVariant> &a) ;
    bool cancel_by_cookie(cookie_t c) ;
    void cancel_event(event_t *e) ;
    event_t *find_event(cookie_t c) ;
    void alarm_gate(bool value) ;
    bool dialog_response(cookie_t c, int value) ;
    Q_INVOKABLE void process_transition_queue() ;
    void update_rtc_alarm() ;
    ticker_t calculate_bootup() ;
    void send_bootup_signal() ;
    void invoke_process_transition_queue() ;
    void reshuffle_queue(const nanotime_t &back) ;
    void request_state(event_t *e, state *st) ;
    void request_state(event_t *e, const char *state_name) ;
    void request_state(event_t *e, const string &state_name) ;
    void send_queue_context() ;
    Q_OBJECT ;
  public Q_SLOTS:
  Q_SIGNALS:
    void engine_pause(int dx) ;
    void voland_registered() ;
    void voland_unregistered() ;
    void queue_to_be_saved() ;
    void voland_needed() ;
    void next_bootup_event(int) ;
    void child_created(unsigned, int) ;
  public:
    void emit_child_created(unsigned cookie, int pid) { emit child_created(cookie, pid) ; }
    queue_pause *initial_pause ;
    void emit_engine_pause(int dx) { emit engine_pause(dx) ; }
    void start() ; //  { delete initial_pause ; initial_pause = NULL ; process_transition_queue() ; }
    void device_mode_detected(bool user_mode) ;
    bool is_epoch_open() ;
    void open_epoch() ;
    iodata::record *save(bool for_backup) ;
    void load(const iodata::record *) ;
    void load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies) ;
    void cancel_backup_events() ;
  private:
    string s_states() ;
    string s_transition_queue() ;
  } ;

  struct queue_pause
  {
    machine *om ;
    queue_pause(machine *m) : om(m) { om->emit_engine_pause(+1) ; }
   ~queue_pause() { om->emit_engine_pause(-1) ; }
  } ;

struct request_watcher_t : public QObject
{
  request_watcher_t(machine *) ;
 ~request_watcher_t() ;
  void watch(const QDBusPendingCall &async_call) ;
  void attach(event_t *e) ;
  void detach(event_t *e) ;
private:
  void detach_not_destroy(event_t *e) ;
  Q_OBJECT ;
  set<event_t*> events ;
  machine *om ;
  QDBusPendingCallWatcher *w ;
private Q_SLOTS:
  void call_returned(QDBusPendingCallWatcher *w) ;
} ;
#endif // 0

#if 0
  state::state(const char *n, machine *m) : om(m)
  {
    log_assert(n!=NULL) ;
    sname = n ;
    action_mask = 0 ;
  }

  state::~state()
  {
  }

  void state::enter(event_t *e)
  {
    uint32_t cluster_before = e->flags & EventFlags::Cluster_Mask ;
    uint32_t cluster_after = cluster_bits() ;
    uint32_t off = cluster_before & ~cluster_after ;
    uint32_t on = cluster_after & ~cluster_before ;
    log_debug("[%d]->%s before=0x%08X after=0x%08X off=0x%08X on=0x%08X", e->cookie.value(), name(), cluster_before, cluster_after, off, on) ;
    for(uint32_t b; b = (off ^ (off-1)), b &= off ; off ^= b)
      om->clusters[b]->leave(e) ;

    e->flags &= ~ EventFlags::Cluster_Mask ;
    e->flags |= cluster_after ;
    for(uint32_t b; b = (on ^ (on-1)), b &= on ; on ^= b)
      om->clusters[b]->enter(e) ;
  }

  void state::leave(event_t *)
  {
  }

  io_state::io_state(const char *n, machine *m, QObject *parent)
    : QObject(parent), state(n, m)
  {
  }

  void io_state::enter(event_t *e)
  {
    state::enter(e) ; // process clusters
    events.insert(e) ;
  }

  void io_state::leave(event_t *e)
  {
    events.erase(e) ;
    state::leave(e) ; // does nothing
  }

  void io_state::abort(event_t *e)
  {
    om->request_state(e, "ABORTED") ;
    om->process_transition_queue() ;
  }

  void gate_state::enter(event_t *e)
  {
    if(is_open)
    {
      state::enter(e) ;
      om->request_state(e, nxt_state) ;
      om->process_transition_queue() ;
    }
    else
      io_state::enter(e) ;
  }

  gate_state::gate_state(const char *name, const char *nxt, machine *m, QObject *p) :
    io_state(name, m, p),
    nxt_state(nxt),
    is_open(false)
  {
  }

  void gate_state::close()
  {
    is_open = false ;
    emit closed() ;
  }

  void gate_state::open()
  {
    log_debug() ;
    is_open = true ;
    log_debug() ;
    for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, nxt_state) ;
    log_debug("events.empty()=%d", events.empty()) ;
    if (not events.empty())
      om->process_transition_queue() ;
    log_debug() ;
    emit opened() ;
  }

  void concentrating_state::open()
  {
    // Not setting "is_open" to true: it's always closed
    // Not emit any opened() / closed() signals
    log_debug() ;
    for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, nxt_state) ;
    if (not events.empty())
      om->process_transition_queue() ;
    log_debug() ;
  }

  filter_state::filter_state(const char *name, const char *retry, const char *nxt, machine *m, QObject *p) :
    gate_state(name, retry, m, p),
    next(nxt)
  {
    QObject::connect(this, SIGNAL(closed()), this, SLOT(emit_close())) ;
  }

  void filter_state::enter(event_t *e)
  {
    if(is_open || ! filter(e))
    {
      om->request_state(e, next) ;
      om->process_transition_queue() ;
    }
    else
      gate_state::enter(e) ;
  }
#endif

#if 0
  void machine::start()
  {
    delete initial_pause ;
    initial_pause = NULL ;
    process_transition_queue() ;
  }

  machine::machine(const Timed *daemon) : owner(daemon)
  {
    log_debug() ;
    // T = transition state
    // IO = waiting for i/o state
    // G = gate state
    // C = concentrating gate state
    // F = filtering state
    // A = actions allowed
    // -->NEW loaded as new
    // -->DUE loaded as due
    state *S[] =
    {
      new state_start(this),          // T
        epoch =
      new state_epoch(this),          // T
      new state_new(this),            // T
      new state_scheduler(this),      // T
      new state_qentry(this),         // T
      new state_flt_conn(this),       // IO G F -->NEW
      new state_flt_alrm(this),       // IO G F -->NEW
      new state_flt_user(this),       // IO G F -->NEW
      new state_queued(this),         // IO A   -->NEW

      new state_due(this),            // T

      new state_missed(this),         // T A
//      new state_postponed(this),      // T
      new state_skipped(this),        // T

      new state_armed(this),          // IO G
      new state_triggered(this),      // T A

      new state_dlg_wait(this),       // IO G   -->DUE
      new state_dlg_cntr(this),       // IO C   -->DUE
      new state_dlg_requ(this),       // IO G   -->DUE
      new state_dlg_user(this),       // IO G   -->DUE
      new state_dlg_resp(this),       // T

      /* state_button: below */       // T A

      new state_snoozed(this),        // T
      new state_recurred(this),       // T
      new state_served(this),         // T
      new state_tranquil(this),       // IO A -->DUE

      new state_removed(this),        // T
      new state_aborted(this),        // T
      new state_finalized(this),      // T A
      NULL
    } ;
    log_debug() ;
    for(int i=0; S[i]; ++i)
      states[S[i]->name()] = S[i] ;


    log_debug() ;
    for(int i=0; i<=Maemo::Timed::Number_of_Sys_Buttons; ++i)
    {
      state *s = new state_button(this, -i) ;
      states[s->name()] = s ;
      button_states[-i] = s ;
    }

    log_debug() ;
    for(int i=1; i<=Maemo::Timed::Max_Number_of_App_Buttons; ++i)
    {
      state *s = new state_button(this, i) ;
      states[s->name()] = s ;
      button_states[i] = s ;
    }

    log_debug() ;
    states["TRIGGERED"]->set_action_mask(ActionFlags::State_Triggered) ;
    states["QUEUED"]->set_action_mask(ActionFlags::State_Queued) ;
    states["MISSED"]->set_action_mask(ActionFlags::State_Missed) ;
    states["TRANQUIL"]->set_action_mask(ActionFlags::State_Tranquil) ;
    states["FINALIZED"]->set_action_mask(ActionFlags::State_Finalized) ;
    states["DUE"]->set_action_mask(ActionFlags::State_Due) ;
    states["SNOOZED"]->set_action_mask(ActionFlags::State_Snoozed) ;
    states["SERVED"]->set_action_mask(ActionFlags::State_Served) ;
    states["ABORTED"]->set_action_mask(ActionFlags::State_Aborted) ;
    if(0) states["FAILED"]->set_action_mask(ActionFlags::State_Failed) ;

    log_debug() ;
    io_state *queued = dynamic_cast<io_state*> (states["QUEUED"]) ;
    log_assert(queued!=NULL) ;

    gate_state *armed = dynamic_cast<gate_state*> (states["ARMED"]) ;
    log_assert(armed!=NULL) ;
    armed->open() ; // will be closed in some very special situations

    log_debug() ;
    gate_state *dlg_wait = dynamic_cast<gate_state*> (states["DLG_WAIT"]) ;
    gate_state *dlg_requ = dynamic_cast<gate_state*> (states["DLG_REQU"]) ;
    gate_state *dlg_user = dynamic_cast<gate_state*> (states["DLG_USER"]) ;
    gate_state *dlg_cntr = dynamic_cast<gate_state*> (states["DLG_CNTR"]) ;
    log_assert(dlg_wait!=NULL) ;
    log_assert(dlg_requ!=NULL) ;
    log_assert(dlg_user!=NULL) ;
    log_assert(dlg_cntr!=NULL) ;

    QObject::connect(dlg_wait, SIGNAL(voland_needed()), this, SIGNAL(voland_needed())) ;

    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_requ, SLOT(open())) ;
    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_user, SLOT(open())) ;
    QObject::connect(dlg_requ, SIGNAL(closed()), dlg_wait, SLOT(open())) ;

    QObject::connect(this, SIGNAL(voland_registered()), dlg_requ, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_registered()), dlg_user, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_unregistered()), dlg_wait, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_unregistered()), dlg_cntr, SLOT(send_back())) ;

    QObject::connect(queued, SIGNAL(sleep()), dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;
    QObject::connect(dlg_wait, SIGNAL(opened()), dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;

    log_debug() ;
    filter_state *flt_conn = dynamic_cast<filter_state*> (states["FLT_CONN"]) ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
    log_assert(flt_conn) ;
    log_assert(flt_alrm) ;
    log_assert(flt_user) ;

    QObject::connect(flt_conn, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_alrm, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_user, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;


    log_debug() ;
    QObject::connect(this, SIGNAL(engine_pause(int)), queued, SLOT(engine_pause(int))) ;
    log_debug() ;
    initial_pause = new queue_pause(this) ;
    log_debug() ;

    cluster_queue *c_queue = new cluster_queue(this) ;
    log_debug() ;
    clusters[c_queue->bit] = c_queue ;
    log_debug() ;

    cluster_dialog *c_dialog = new cluster_dialog(this) ;
    log_debug() ;
    clusters[c_dialog->bit] = c_dialog ;
    log_debug() ;
    signalled_bootup = -1 ; // no signal sent yet
    log_debug() ;

    log_debug("owner->settings->alarms_are_enabled=%d", owner->settings->alarms_are_enabled) ;
    log_debug() ;
    alarm_gate(owner->settings->alarms_are_enabled) ;
    log_debug() ;

    transition_start_time = ticker_t(0) ;
    log_debug() ;
    next_cookie = 1 ;
    log_debug() ;
    context_changed = false ;
    log_debug("last line") ;
  }

  machine::~machine()
  {
    log_debug() ;

    log_notice("deleting events") ;
    for (map<cookie_t, event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      delete it->second ;

    log_notice("deleting clusters") ;
    for (map<uint32_t, abstract_cluster*>::iterator it=clusters.begin(); it!=clusters.end(); ++it)
      delete it->second ;

    log_notice("deleting states") ;
    for (map<string, state*>::iterator it=states.begin(); it!=states.end(); ++it)
      delete it->second ;

    log_debug() ;
  }

  void machine::device_mode_detected(bool user_mode)
  {
    filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
    log_assert(flt_user) ;
    if(user_mode)
      flt_user->open() ;
    else
      flt_user->close() ;
    log_info("device_mode_detected: %s", user_mode ? "USER" : "ACT_DEAD") ;
  }

  bool machine::is_epoch_open()
  {
    return epoch->is_open ;
  }

  void machine::open_epoch()
  {
    epoch->open() ;
  }

  string machine::s_states()
  {
    ostringstream os ;
    map<string, set<event_t*> > s2e ;
    for (map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    {
      event_t *e = it->second ;
      string name = e->get_state()==NULL ? "<null>" : e->get_state()->name() ;
      s2e[name].insert(e) ;
    }

    for (map<string, set<event_t*> >::const_iterator it=s2e.begin(); it!=s2e.end(); ++it)
      for (set<event_t*>::const_iterator q=it->second.begin(); q!=it->second.end(); ++q)
        os << (q==it->second.begin() ? string(it==s2e.begin()?"":" ")+it->first+":" : ",") << (*q)->cookie.value() ;

    return os.str() ;
  }

  string machine::s_transition_queue()
  {
    ostringstream os ;
    for(deque<pair<event_t*, state*> >::const_iterator it=transition_queue.begin(); it!=transition_queue.end(); ++it)
    {
      cookie_t c = it->first->cookie ;
      state *s = it->second ;
      bool first = it==transition_queue.begin() ;
      os << (first ? "" : ", ") << c.value() << "->" << (s?s->name():"null") ;
    }
    return os.str() ;
  }

  void machine::process_transition_queue()
  {
    if(transition_in_progress())
    {
      log_debug("process_transition_queue() is already in progress, returning") ;
      return ; // never do it recursively
    }
    // log_debug("begin processing, states: %s tqueue: %s" , s_states().c_str(), s_transition_queue().c_str()) ;
    transition_start_time = ticker_t(now()) ;
    bool queue_changed = false ;
    for(; !transition_queue.empty(); queue_changed = true, transition_queue.pop_front())
    {
      event_t *e = transition_queue.front().first ;
      state *old_state = e->get_state() ;
      state *new_state = transition_queue.front().second ;
      log_assert(new_state!=old_state, "New state is the same as the old one (%s)", old_state->name()) ;
#define state_name(p) (p?p->name():"null")
      log_info("State transition %d:'%s'->'%s'", e->cookie.value(), state_name(old_state), state_name(new_state)) ;
#undef state_name
      if(old_state)
        old_state->leave(e) ;
      e->set_state(new_state) ;
      if(new_state)
      {
        new_state->enter(e) ;
        e->sort_and_run_actions(new_state->get_action_mask()) ;
      }
      else
      {
        log_info("Destroying the event %u (event object %p)", e->cookie.value(), e) ;
        events.erase(e->cookie) ;
        delete e ;
      }
    }
    // log_debug("processing done,  states: %s tqueue: %s" , s_states().c_str(), s_transition_queue().c_str()) ;
    update_rtc_alarm() ;
    transition_start_time = ticker_t(0) ;
    if(queue_changed)
      emit queue_to_be_saved() ;
    if(context_changed)
      send_queue_context() ;
    send_bootup_signal() ;
  }

  void machine::update_rtc_alarm()
  {
    state_queued *q = dynamic_cast<state_queued*> (states["QUEUED"]) ;
    log_assert(q!=NULL) ;
    time_t tick = q->next_rtc_bootup().value() ;
    if(tick>0) // valid
    {
      tick -= RenameMeNameSpace::Bootup_Length ;
      if(tick < transition_start_time.value())
        tick = -1 ;
    }
    struct rtc_wkalrm rtc ;
    memset(&rtc, 0, sizeof(struct rtc_wkalrm)) ;

    if(tick>0) // still valid
      rtc.enabled = 1 ;
    else
    {
      rtc.enabled = 0 ;
      tick = transition_start_time.value() ; // need a valid timestamp
    }

    log_debug("rtc.enabled=%d", rtc.enabled) ;
    struct tm tm ;
    gmtime_r(&tick, &tm) ;
    memset(&rtc.time, -1, sizeof rtc.time) ;
#define cp(name) rtc.time.tm_##name = tm.tm_##name
    cp(sec), cp(min), cp(hour), cp(mday), cp(mon), cp(year) ;
#undef cp

    int fd = open("/dev/rtc0", O_RDONLY) ;
    if(fd<0)
    {
      log_error("Can't open real time clock: %s", strerror(errno)) ;
      return ;
    }
    int res = ioctl(fd, RTC_WKALM_SET, &rtc) ;
    if(res<0)
      log_error("Can't set real time clock alarm: %s", strerror(errno)) ;
    else
      log_info("Real time clock alarm: %s", !rtc.enabled ? "OFF"
        : str_printf("ON (%04d-%02d-%02d %02d:%02d:%02d)",
          rtc.time.tm_year+1900, rtc.time.tm_mon+1, rtc.time.tm_mday,
          rtc.time.tm_hour, rtc.time.tm_min, rtc.time.tm_sec).c_str()) ;
    close(fd) ;
  }

  ticker_t machine::calculate_bootup()
  {
    cluster_dialog *d = dynamic_cast<cluster_dialog*> (clusters[EventFlags::Cluster_Dialog]) ;
    log_assert(d!=NULL) ;
    if(d->has_bootup_events())
      return ticker_t(1) ; // right now
    state_queued *q = dynamic_cast<state_queued*> (states["QUEUED"]) ;
    log_assert(q!=NULL) ;
    return q->next_bootup() ;
  }

  void machine::send_bootup_signal()
  {
    log_debug() ;
    ticker_t tick = calculate_bootup() ;
    int32_t next_bootup = 0 ;
    if(tick.is_valid())
      next_bootup = tick.value() ;
    log_debug("signalled_bootup=%d, next_bootup=%d", signalled_bootup, next_bootup) ;
    if(signalled_bootup<0 || signalled_bootup!=next_bootup)
      emit next_bootup_event(signalled_bootup=next_bootup) ;
    log_debug() ;
  }

  void machine::invoke_process_transition_queue()
  {
    int methodIndex = this->metaObject()->indexOfMethod("process_transition_queue()");
    QMetaMethod method = this->metaObject()->method(methodIndex);
    method.invoke(this, Qt::QueuedConnection);
  }

  void machine::reshuffle_queue(const nanotime_t &back)
  {
    // log_assert(false, "not implemented") ;
    state *sch = states["SCHEDULER"] ;
    const char *from[] = {"QUEUED","FLT_CONN","FLT_ALRM","FLT_USER",NULL} ;
    for(const char **name=from; *name; ++name)
    {
      io_state *s = dynamic_cast<io_state*> (states[*name]) ;
      log_assert(s) ;
      for(set<event_t*>::iterator it=s->events.begin(); it!=s->events.end(); ++it)
      {
        event_t *e = *it ;
        bool snoozing = !! (e->flags & EventFlags::Snoozing) ;
        bool system_time_changing = not back.is_zero() ;

        if (snoozing and system_time_changing)
        {
          e->ticker = e->trigger + (- back.to_time_t()) ;
          request_state(e, sch) ;
        }

        if (snoozing)
          continue ;

        if (e->has_ticker() and not system_time_changing)
          continue ;

        if(e->flags & EventFlags::Empty_Recurring) // TODO: rewrite recurrence calculation and get rid of "Empty_Recurring"
          e->invalidate_t() ;

        request_state(e, sch) ;
      }
    }
    process_transition_queue() ;
  }

  void machine::request_state(event_t *e, state *st)
  {
    // log_info("request_state(%u,%s)", e->cookie.value(), st?st->name:"null") ;
    transition_queue.push_back(make_pair(e, st)) ;
    // log_info("done; transition_queue: %s; states: %s", s_transition_queue().c_str(), s_states().c_str()) ;
  }

  void machine::request_state(event_t *e, const char *state_name)
  {
    request_state(e, string(state_name)) ;
  }

  void machine::request_state(event_t *e, const string &state_name)
  {
    state *new_state = NULL ;
    if(not state_name.empty())
    {
      map<string, state*>::iterator it = states.find(state_name) ;
      log_assert(it!=states.end(), "Unknown state: '%s'", state_name.c_str()) ;
      new_state = it->second ;
    }
    request_state(e, new_state) ;
  }

  void machine::send_queue_context()
  {
    log_debug() ;
    static ContextProvider::Property alarm_triggers_p("Alarm.Trigger") ;
    log_debug() ;
    static ContextProvider::Property alarm_present_t("Alarm.Present") ;
    log_debug() ;
    cluster_queue *c_queue = dynamic_cast<cluster_queue*> (clusters[EventFlags::Cluster_Queue]) ;
    log_debug() ;
    alarm_triggers_p.setValue(QVariant::fromValue(c_queue->alarm_triggers)) ;
    log_debug() ;
    alarm_present_t.setValue(!c_queue->alarm_triggers.isEmpty()) ;
    log_debug() ;
    context_changed = false ;
    log_debug() ;
  }

  cookie_t machine::add_event(const Maemo::Timed::event_io_t *eio, bool process_queue, const credentials_t *p_creds, const QDBusMessage *p_message)
  {
    // The credentials for the event are either already known (p_creds)
    //   or have to be established by the QDBusMessage structure (from dbus daemon)
    // Using pointers instead of usual C++ references, just because a NULL-reference
    //   usually confuses people (though working just fine)
    if(event_t *e = event_t::from_dbus_iface(eio))
    {
      if(e->actions.size() > 0)
        e->client_creds = p_creds ? *p_creds : credentials_t::from_dbus_connection(*p_message) ;
      request_state(events[e->cookie = cookie_t(next_cookie++)] = e, "START") ;
      if(process_queue)
        invoke_process_transition_queue() ;
      log_info("new event: cookie=%d, object=%p", e->cookie.value(), e) ;
      return e->cookie ;
    }

    return cookie_t(0) ;
  }

  void machine::add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message)
  {
    // Here we're asking credentials immediately, not like in add_event
    // TODO:
    // But may be it's reasonable first to check, if we really have actions?
    credentials_t creds = credentials_t::from_dbus_connection(message) ;
    bool valid = false ;
    for(int i=0; i<lst.ee.size(); ++i)
    {
#if 1
      unsigned cookie = add_event(&lst.ee[i], false, &creds, NULL).value() ;
#else // make some testing here
      unsigned cookie = add_event(&lst.ee[i], false, NULL  , &message).value() ;
#endif
      res.push_back(cookie) ;
      if (cookie)
      {
        valid = true ;
        log_debug("event[%d]: accepted, cookie=%d", i, cookie) ;
      }
      else
        log_warning("event[%d]: rejected", i) ;
    }
    if(valid)
      invoke_process_transition_queue() ;
  }

  void machine::query(const QMap<QString,QVariant> &words, QList<QVariant> &res)
  {
    vector<string> qk, qv ;
    for(QMap<QString,QVariant>::const_iterator it=words.begin(); it!=words.end(); ++it)
    {
      qk.push_back(string_q_to_std(it.key())) ;
      qv.push_back(string_q_to_std(it.value().toString())) ;
    }
    unsigned N = qk.size() ;
    for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    {
      bool text_matches = true ;
      for(unsigned i=0; text_matches && i<N; ++i)
      {
        map<string,string>::const_iterator F = it->second->attr.txt.find(qk[i]) ;
        if(F==it->second->attr.txt.end())
          text_matches = qv[i]=="" ;
        else
          text_matches = qv[i]==F->second ;
      }
      if(text_matches)
        res.push_back(it->second->cookie.value()) ;
    }
  }

  void machine::get_event_attributes(cookie_t c, QMap<QString,QVariant> &a)
  {
    map<cookie_t,event_t*>::iterator it = events.find(c) ;
    if(it==events.end())
      return ;
    event_t *e = it->second ;
    a.insert("STATE", e->st->name()) ;
    a.insert("COOKIE", QString("%1").arg(c.value())) ;
    for(attribute_t::const_iterator at=e->attr.txt.begin(); at!=e->attr.txt.end(); at++)
    {
      QString key = string_std_to_q(at->first) ;
      QString val = string_std_to_q(at->second) ;
      a.insert(key,val) ;
    }
  }

  bool machine::dialog_response(cookie_t c, int value)
  {
    if(events.count(c)==0)
    {
      log_error("Invalid cookie [%d] in respond", c.value()) ;
      return false ;
    }
    event_t *e = events[c] ;
    if(e->st != states["DLG_USER"])
    {
      log_error("Unexpected response for event [%d] in state %s", c.value(), e->st->name()) ;
      return false ;
    }
    if(value < -Maemo::Timed::Number_of_Sys_Buttons)
    {
      log_error("Invalid negative value in respond: event [%d], value=%d", c.value(), value) ;
      return false ;
    }
    int b = e->b_attr.size() ;
    if(value > b)
    {
      log_error("Invalid value in respond: event [%d], value=%d, number of buttons: %d", c.value(), value, b) ;
      return false ;
    }
    log_assert(button_states.count(value)>0, "value=%d, count=%d", value,button_states.count(value) ) ;
    request_state(e, button_states[value]) ;
    e->flags &= ~ EventFlags::In_Dialog ;
    invoke_process_transition_queue() ;
    return true ;
  }

  event_t *machine::find_event(cookie_t c)
  {
    log_assert(!transition_in_progress()) ;
    process_transition_queue() ;

    map<cookie_t,event_t*>::iterator it = events.find(c) ;
    return it==events.end() ? NULL : it->second ;
  }

  bool machine::cancel_by_cookie(cookie_t c) // XXX need some clean up here?
  {
    queue_pause x(this) ;

    if(event_t *e = find_event(c))
    {
      cancel_event(e) ;
      return true ;
    }
    else
    {
      log_error("[%d]: cookie not found", c.value()) ;
      return false ;
    }
  }

  void machine::cancel_event(event_t *e)
  {
    // TODO: assert (queue is paused)
    io_state *s = dynamic_cast<io_state*> (e->st) ;
    log_assert(s) ;
    s->abort(e) ;
  }

  void machine::alarm_gate(bool value)
  {
    filter_state *filter = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    log_assert(filter) ;
    value ? filter->open() : filter->close() ;
  }

  uint32_t machine::attr(uint32_t mask)
  {
    return flags & mask ;
  }

  iodata::record *machine::save(bool for_backup)
  {
    iodata::record *r = new iodata::record ;
    iodata::array *q = new iodata::array ;

    for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    {
      event_t *e = it->second ;
      if(for_backup and not (e->flags & EventFlags::Backup))
        continue ;
      q->add(e->save(for_backup)) ;
    }

    r->add("events", q) ;
    if(not for_backup)
      r->add("next_cookie", next_cookie) ;
    return r ;
  }

  void machine::load(const iodata::record *r)
  {
    next_cookie = r->get("next_cookie")->value() ;
    const iodata::array *a = r->get("events")->arr() ;
    load_events(a, true, true) ;
  }

  void machine::load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies)
  {
    for(unsigned i=0; i < events_data->size(); ++i)
    {
      const iodata::record *ee = events_data->get(i)->rec() ;
      unsigned cookie = use_cookies ? ee->get("cookie")->value() : next_cookie++ ;
      event_t *e = new event_t ;
      events[e->cookie = cookie_t(cookie)] = e ;

      e->ticker = ticker_t(ee->get("ticker")->value()) ;
      e->t.load(ee->get("t")->rec()) ;

      e->tz = ee->get("tz")->str() ;

      e->attr.load(ee->get("attr")->rec()) ;
      e->flags = ee->get("flags")->decode(event_t::codec) ;
      iodata::load(e->recrs, ee->get("recrs")->arr()) ;
      iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;
      iodata::load(e->b_attr, ee->get("b_attr")->arr()) ;
      e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
      e->tsz_max = ee->get("tsz_max")->value() ;
      e->tsz_counter = ee->get("tsz_counter")->value() ;
      if(trusted_source)
      {
        iodata::load(e->actions, ee->get("actions")->arr()) ;
        e->client_creds.load(ee->get("client_creds")->rec()) ;
        e->cred_modifier.load(ee->get("cred_modifier")->arr()) ;
      }

      const char *next_state = "START" ;

      if(e->flags & EventFlags::Empty_Recurring)
        e->invalidate_t() ;

      request_state(e, next_state) ;
    }
  }

  void machine::cancel_backup_events()
  {
    // TODO: assert (queue is paused)
    vector<event_t*> backup ;
    for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
      if (it->second->flags & EventFlags::Backup)
        backup.push_back(it->second) ;

    for(vector<event_t*>::const_iterator it=backup.begin(); it!=backup.end(); ++it)
      cancel_event(*it) ;

    log_debug("cancelled all the bacjup events") ;
  }
#endif

#if 0
  // TODO: do it accessible from outside of this file:
  //       too many uncaught exceptions :)
  struct event_exception : public std::exception
  {
    string message ;
    pid_t pid_value ;

    pid_t pid() const { return pid_value ; }
    event_exception(const string &msg) : message(msg), pid_value(getpid()) { }
   ~event_exception() throw() { }
  } ;
#endif

#if 0
  // what a mess... event_t::stuff should be in event.cpp

  void event_t::sort_and_run_actions(uint32_t mask)
  {
    if (mask==0) // TODO: check, if this check is not yet done on state_* layer...
      return ;

    // don't want to call unnecesary constructor here
    vector <unsigned> *r = NULL ;

    for (unsigned i=0; i<actions.size(); ++i)
      if(actions[i].flags & mask) // something to run
      {
        if (r==NULL)
          r = new vector <unsigned> ;
        r->push_back(i) ;
      }

    if (r==NULL) // nothing to do
      return ;

    log_assert(r) ;

    log_debug("Beginning the action list dump") ;
    for(vector<unsigned>::const_iterator it=r->begin(); it!=r->end(); ++it)
      log_debug("Action %d, cred_key: '%s'", *it, actions[*it].cred_key().c_str()) ;
    log_debug("Done:     the action list dump") ;

    log_debug("Beginning the action array sorting") ;
    sort(r->begin(), r->end(), action_comparison_t(this)) ;
    log_debug("Done:     the action array sorting") ;

    unsigned start_index = ~0 ; // invalid value
    string current_key ; // empty key (invalid?)
    for(unsigned i=0, have_exec=false; i < r->size(); ++i)
    {
      action_t &a = actions[(*r) [i]] ;
      bool i_exec = a.flags & ActionFlags::Run_Command ; // current action is an exec action
      bool start_now = a.cred_key() != current_key || (have_exec && i_exec) ; // either new key or second exec
      if (start_now && i>0)
        run_actions(*r, start_index, i) ;
      if (start_now || i==0) // i==0 is probably redundant, as cred_key should never be empty
      {
        have_exec = i_exec ;
        start_index = i ;
        current_key = a.cred_key() ;
      }
    }

    log_assert(r->size()>0) ; // to be sure for the run_actions() call below
    run_actions(*r, start_index, r->size()) ;

    delete r ;
  }

  template <class element_t>
  string print_vector(const vector<element_t> &array, unsigned begin, unsigned end)
  {
    ostringstream os ;

    for(unsigned i=begin, first=true; i<end; ++i)
    {
      os << ( first ? first=false, "[" : ", " ) ;
      os << array[i] ;
    }
    os << "]" ;
    return os.str() ;
  }

  template <class element_t> // an yet another hihgly inefficient function :-(
  void set_change(set<element_t> &x, const set<element_t> &y, bool add)
  {
    for(typename set<element_t>::const_iterator it=y.begin(); it!=y.end(); ++it)
      if(add)
        x.insert(*it) ;
      else
        x.erase(*it) ;
  }

  void event_t::run_actions(const vector<unsigned> &acts, unsigned begin, unsigned end)
  {
    log_debug("begin=%d, end=%d, actions: %s", begin, end, print_vector(acts, begin, end).c_str()) ;
    const action_t &a_begin = actions [acts[begin]] ;

    // we want to detect, it the exeption was thrown in the daemon itself or in a child
    pid_t daemon = getpid() ;

    try
    {
      if (fork_and_set_credentials_v3(a_begin)>0) // parent
        return ;
    }
    catch(const event_exception &e)
    {
      log_error("action list %d%s failed: %s", cookie.value(), print_vector(acts, begin, end).c_str(), e.message.c_str()) ;
      pid_t process = e.pid() ;
      if (process!=daemon)
      {
        log_info("terminating child (pid=%d) of daemon (pid=%d)", process, daemon) ;
        ::exit(1) ;
      }
    }

    // now we're running in the child with proper credentials, so let's execute dbus stuff !

    unsigned exec_action_i = ~0 ; // not found yet
    QDBusConnection *conn[2] = {NULL, NULL} ; // 0->system, 1->session
    const QString cname = "timed-private" ;

    int dbus_fail_counter = 0 ;

    for (unsigned i=begin; i<end; ++i)
    {
      const unsigned acts_i = acts[i] ;
      const action_t &a = actions [acts_i] ;

      if (a.flags & ActionFlags::Run_Command)
        exec_action_i = acts_i ;

      if((a.flags & ActionFlags::DBus_Action) == 0)
        continue ;

      try
      {
        // set up message to be sent
        QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
        QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;

        QDBusMessage message ;

        if (a.flags & ActionFlags::DBus_Method)
        {
          QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
          QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
          message = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
        }
        else
        {
          QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
          message = QDBusMessage::createSignal(path, ifac, sgnl) ;
        }

        QMap<QString,QVariant> param ;

        if (a.flags & ActionFlags::Send_Cookie)
          param["COOKIE"] = QString("%1").arg(cookie.value()) ;

        if (a.flags & ActionFlags::Send_Event_Attributes)
          add_strings(param, attr.txt) ;

        if (a.flags & ActionFlags::Send_Action_Attributes)
          add_strings(param, a.attr.txt) ;

        message << QVariant::fromValue(param) ;

        int bus = (a.flags & ActionFlags::Use_System_Bus) ? 0 : 1 ;
        QDBusConnection * &c = conn[bus] ;

        if (c==NULL) // not used yes, have to create object and connect
        {
          QDBusConnection::BusType ctype = bus==0 ? QDBusConnection::SystemBus : QDBusConnection::SessionBus ;
          c = new QDBusConnection(QDBusConnection::connectToBus(ctype, cname)) ;
        }

        if (c->send(message))
          log_debug("%u[%d]: D-Bus Message sent", cookie.value(), acts_i) ;
        else
          throw event_exception(c->lastError().message().toStdString().c_str()) ;
      }
      catch(const event_exception &e)
      {
        log_error("%u[%d]: dbus-action not executed: %s", cookie.value(), acts_i, e.message.c_str()) ;
        ++ dbus_fail_counter ;
      }
    }

    for(int c=0; c<2; ++c)
      if(QDBusConnection *cc = conn[c])
      {
        // as we are about to exit immediately after queuing
        // and there seems to be no way to flush the connection
        // and be sure that we have actually transmitted the
        // message -> do a dummy synchronous query from dbus
        // daemon and hope that is enough to get the actual
        // message to be delivered ...
        QString connection_name  = cc->baseService() ;
        unsigned owner = get_name_owner_from_dbus_sync(*cc, connection_name) ;

        // it should be us (either pid or uid dependin on feature set) ...
        log_debug("pid=%d, ruid=%d, euid=%d, connection owner is '%u'", getpid(), getuid(), geteuid(), owner) ;
        QDBusConnection::disconnectFromBus(connection_name) ;
        delete cc ;
      }

    if(dbus_fail_counter)
      log_warning("failed to exeute %d out of %u dbus actions", dbus_fail_counter, end-begin) ;

    if(exec_action_i == ~0u) // no exec action in this action package
      ::exit(dbus_fail_counter<100 ? dbus_fail_counter : 100) ; // exit value can't be toooo large

    // All the dbus actions are done. Now we have to execute the only command line action

    const action_t &a = actions [exec_action_i] ;

    string cmd = find_action_attribute("COMMAND", a) ;
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
    log_info("execuing command line action %u[%d]: '%s'", cookie.value(), exec_action_i, cmd.c_str());
    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;
    log_error("execl(/bin/sh -c '%s') failed: %m", cmd.c_str());

    ::exit(101) ; // use dbus_fail_counter here as well?
  }

  bool event_t::accrue_privileges(const action_t &a)
  {
    credentials_t creds = credentials_t::from_current_process() ;

#if F_TOKENS_AS_CREDENTIALS
    const cred_modifier_t &E = cred_modifier, &A = a.cred_modifier ;

    // tokens_to_accrue1 := EVENT.add - ACTION.drop
    set<string> tokens_to_accrue1 = E.tokens_by_value(true) ;
    set_change<string> (tokens_to_accrue1, A.tokens_by_value(false), false) ;

    // tokens_to_accrue2 := ACTION.add - EVENT.drop
    set<string> tokens_to_accrue2 = A.tokens_by_value(true) ;
    set_change<string> (tokens_to_accrue2, E.tokens_by_value(false), false) ;

    // creds += (tokens_to_accrue 1+2)
    set_change<string> (creds.tokens, tokens_to_accrue1, true) ;
    set_change<string> (creds.tokens, tokens_to_accrue2, true) ;
#endif // F_TOKENS_AS_CREDENTIALS

    string uid = find_action_attribute("USER", a, false) ;
    string gid = find_action_attribute("GROUP", a, false) ;

    if (!uid.empty())
      creds.uid = uid ;
    if (!gid.empty())
      creds.gid = gid ;

    return creds.apply_and_compare() ;
  }

  bool event_t::drop_privileges(const action_t &a)
  {
    credentials_t creds = client_creds ;

#if F_TOKENS_AS_CREDENTIALS
    const cred_modifier_t &E = cred_modifier, &A = a.cred_modifier ;

    // tokens_to_remove1 := EVENT.drop - ACTION.add
    set<string> tokens_to_remove1 = E.tokens_by_value(false) ;
    set_change<string> (tokens_to_remove1, A.tokens_by_value(true), false) ;

    // tokens_to_remove2 := ACTION.drop - EVENT.add
    set<string> tokens_to_remove2 = A.tokens_by_value(false) ;
    set_change<string> (tokens_to_remove2, E.tokens_by_value(true), false) ;

    // creds := client_creds - (tokens_to_remove 1+2)
    set_change<string> (creds.tokens, tokens_to_remove1, false) ;
    set_change<string> (creds.tokens, tokens_to_remove2, false) ;
#endif // F_TOKENS_AS_CREDENTIALS

    return creds.apply_and_compare() ;
  }

  string event_t::find_action_attribute(const string &key, const action_t &a, bool exc)
  {
    string value = a.attr(key) ;
    if(value.empty())
      value = attr(key) ;
    if(value.empty() && exc)
      throw event_exception(string("empty attribute '")+key+"'") ;
    return value ;
  }

  void event_t::add_strings(QMap<QString, QVariant> &x, const map<string,string> &y)
  {
    for(map<string,string>::const_iterator it=y.begin(); it!=y.end(); ++it)
      x.insert(string_std_to_q(it->first), QVariant::fromValue(string_std_to_q(it->second))) ;
  }

  pid_t event_t::fork_and_set_credentials_v3(const action_t &action)
  {
    pid_t pid = fork() ;

    if (pid<0) // can't fork
    {
      log_error("fork() failed: %m") ;
      throw event_exception("can't fork");
    }
    else if (pid>0) // parent
    {
      log_info("forked successfully, child pid: '%d'", pid) ;
      st->om->emit_child_created(cookie.value(), pid) ;
      return pid ;
    }
    else // child
    {
      log_info("event [%u]: in child process", cookie.value()) ;

      if (setsid() < 0) // detach from session
      {
        log_error("setsid() failed: %m") ;
        throw event_exception("can't detach from session") ;
      }

      if (!drop_privileges(action))
        throw event_exception("can't drop privileges") ;

      if (!accrue_privileges(action))
        log_warning("can't accrue privileges, still continuing") ;

      // Skipping all the uid/gid settings, because it's part of
      // credentials_t::apply() (or it should be at least)

      // TODO: go to home dir _here_ !

      // That's it then, isn't it?

      return pid ;
    }
  }

  iodata::record *event_t::save(bool for_backup)
  {
    iodata::record *r = new iodata::record ;
    r->add("cookie", cookie.value()) ;
    r->add("ticker", ticker.value()) ;
    r->add("t", t.save()) ;
    r->add("tz", new iodata::bytes(tz)) ;
    r->add("attr", attr.save() ) ;
    r->add("flags", new iodata::bitmask(flags &~ EventFlags::Cluster_Mask, codec)) ;
    r->add("recrs", iodata::save(recrs)) ;
    r->add("snooze", iodata::save_int_array(snooze)) ;
    r->add("b_attr", iodata::save(b_attr)) ;

    r->add("dialog_time", (flags & EventFlags::In_Dialog) ? last_triggered.value() : 0) ;
    r->add("tsz_max", tsz_max) ;
    r->add("tsz_counter", tsz_counter) ;

    if(not for_backup)
      r->add("actions", iodata::save(actions)) ;
    if(not for_backup)
      r->add("client_creds", client_creds.save()) ;
    if(not for_backup)
      r->add("cred_modifier", cred_modifier.save()) ;
    return r ;
  }

  bool event_t::compute_recurrence()
  {
    log_assert(false, "obsolete function") ;
    return false ;
  }
#endif

#if 0
request_watcher_t::request_watcher_t(machine *om)
{
  this->om = om ;
  w = NULL ;
}

request_watcher_t::~request_watcher_t()
{
  delete w ;
  for(set<event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    detach_not_destroy(*it) ;
}

void request_watcher_t::watch(const QDBusPendingCall &async_call)
{
  log_assert(w==NULL, "watch() called more then once") ;
  w = new QDBusPendingCallWatcher(async_call) ;
  QObject::connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(call_returned(QDBusPendingCallWatcher*))) ;
}

void request_watcher_t::attach(event_t *e)
{
  // First, we have to detach a link to an orphaned watcher
  if (e->request_watcher)
  {
    log_assert(e->request_watcher != this, "attaching the same event twice") ;
    e->request_watcher->detach(e) ;
  }

  // Now attach the event to this watcher object
  e->request_watcher = this ;
  this->events.insert(e) ;
}

void request_watcher_t::detach_not_destroy(event_t *e)
{
  // First make sure, it's really attached
  log_assert(e->request_watcher) ;
  log_assert(e->request_watcher==this) ;
  log_assert(this->events.count(e) > 0) ;

  // Now detach:
  e->request_watcher = NULL ;
  this->events.erase(e) ;
}

void request_watcher_t::detach(event_t *e)
{
  detach_not_destroy(e) ;

  // The object should be destroyed if empty
  if (this->events.empty())
    delete this ; // ouch! is it kosher C++ ? I hope so, at least...
}

void request_watcher_t::call_returned(QDBusPendingCallWatcher *w)
{
  log_debug() ;
  log_assert(w==this->w, "oops, somethig is really wrong with qdbus...") ;

  QDBusPendingReply<bool> reply = *w ;

#if 0
  if (reply.isValid() && reply.value())
#else
  log_info("reminder servide replied: %d,%d", reply.isValid(), reply.isValid()&&reply.value()) ;
  if (true) // XXX: remove this after reminders returns TRUE here.
#endif
  {
    for(set<event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, "DLG_USER") ;

    if (events.size()>0)
      om->process_transition_queue() ;
  }
  else
    log_error("DBus call voland::create failed events not listed here...") ;


  // Don't need the watcher any more

  delete this ;
}
#endif

#if 0
    set<event_t*> junk ;
    set<string*> sjunk ;
    for(int i=0; i<10000; ++i)
    {
#if 0
      event_t *ee = event_t::from_dbus_iface(eio) ;
      // ee->client_creds = p_creds ? *p_creds : credentials_t::from_dbus_connection(*p_message) ;
      junk.insert(ee) ;
#endif
#if 0
      string *ss = new string ;
      // *ss = str_printf("string %d", i) ;
      QString q = QString("string %1").arg(i) ;
      *ss = string_q_to_std(q) ;
#endif
    }
    for(set<event_t*>::iterator it=junk.begin(); it!=junk.end(); ++it)
      delete *it ;
    junk.clear() ;
    for(set<string*>::iterator it=sjunk.begin(); it!=sjunk.end(); ++it)
      delete *it ;
    sjunk.clear() ;
#endif
#if 0
    vector<credentials_t> junk ;
    for(int i=0; i<1000; ++i)
    {
      credentials_t cred = Aegis::credentials_from_dbus_connection(*p_message) ;
      junk.push_back(cred) ;
    }
    junk.clear() ;
#endif

#if 0
    request_state(events[e->cookie = cookie_t(next_cookie++)] = e, "START") ;
#else
