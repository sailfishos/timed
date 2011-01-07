e->flags = EventFlags::Keep_Alive ;
e->flags |= EventFlags::Aligned_Snooze|EventFlags::Reminder ;

#if 0
e->actions.resize(8) ;
log_debug("size of action_t: %d", sizeof(action_t)) ;
e->actions[0].attr.txt["COMMAND"] = "echo cookie=[COOKIE]=<COOKIE> TRIGGERED $(date) >> /home/user/triggered" ;
e->actions[0].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::State_Triggered ;
e->actions[1].attr.txt["COMMAND"] = "echo [COOKIE] BUTTON #1 $(date) >> /home/user/triggered" ;
e->actions[1].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::app_button(1) ;
e->actions[2].attr.txt["COMMAND"] = "echo [COOKIE] BUTTON #2 $(date) >> /home/user/triggered" ;
e->actions[2].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::app_button(2) ;
e->actions[3].attr.txt["COMMAND"] = "echo [COOKIE] BUTTON #3 $(date) >> /home/user/triggered" ;
e->actions[3].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::app_button(3) ;
e->actions[4].attr.txt["COMMAND"] = "echo [COOKIE] BUTTON #4 $(date) >> /home/user/triggered" ;
e->actions[4].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::app_button(1) ;
e->actions[5].attr.txt["COMMAND"] = "echo [COOKIE] CANCELED BY USER $(date) >> /home/user/triggered" ;
e->actions[5].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::sys_button(0) ;
e->actions[6].attr.txt["COMMAND"] = "echo [COOKIE] FIRST SYSTEM BYTTON $(date) >> /home/user/triggered" ;
e->actions[6].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::sys_button(1) ;
e->actions[7].attr.txt["COMMAND"] = "echo [COOKIE] SECOND SYSTEM BYTTON $(date) >> /home/user/triggered" ;
e->actions[7].flags = ActionFlags::Run_Command|ActionFlags::Send_Cookie|ActionFlags::sys_button(2) ;
#endif

e->attr.txt["APPLICATION"] = "simple_client" ;
e->attr.txt["TITLE"] = "/1000/" ;

#if 0
e->client_creds.gid = "root" ;
e->client_creds.uid = "root" ;

e->client_creds.tokens.insert("AID::com.nokia.maemo.timed-tools.simple-client") ;
e->client_creds.tokens.insert("CAP::audit_control") ;
e->client_creds.tokens.insert("CAP::audit_write") ;
e->client_creds.tokens.insert("CAP::chown") ;
e->client_creds.tokens.insert("CAP::dac_override") ;
e->client_creds.tokens.insert("CAP::dac_read_search") ;
e->client_creds.tokens.insert("CAP::fowner") ;
e->client_creds.tokens.insert("CAP::fsetid") ;
e->client_creds.tokens.insert("CAP::ipc_lock") ;
e->client_creds.tokens.insert("CAP::ipc_owner") ;
e->client_creds.tokens.insert("CAP::kill") ;
e->client_creds.tokens.insert("CAP::lease") ;
e->client_creds.tokens.insert("CAP::linux_immutable") ;
e->client_creds.tokens.insert("CAP::mac_admin") ;
e->client_creds.tokens.insert("CAP::mac_override") ;
e->client_creds.tokens.insert("CAP::mknod") ;
e->client_creds.tokens.insert("CAP::net_admin") ;
e->client_creds.tokens.insert("CAP::net_bind_service") ;
e->client_creds.tokens.insert("CAP::net_broadcast") ;
e->client_creds.tokens.insert("CAP::net_raw") ;
e->client_creds.tokens.insert("CAP::setfcap") ;
e->client_creds.tokens.insert("CAP::setgid") ;
e->client_creds.tokens.insert("CAP::setuid") ;
e->client_creds.tokens.insert("CAP::sys_admin") ;
e->client_creds.tokens.insert("CAP::sys_boot") ;
e->client_creds.tokens.insert("CAP::sys_chroot") ;
e->client_creds.tokens.insert("CAP::sys_module") ;
e->client_creds.tokens.insert("CAP::sys_nice") ;
e->client_creds.tokens.insert("CAP::sys_pacct") ;
e->client_creds.tokens.insert("CAP::sys_ptrace") ;
e->client_creds.tokens.insert("CAP::sys_rawio") ;
e->client_creds.tokens.insert("CAP::sys_resource") ;
e->client_creds.tokens.insert("CAP::sys_time") ;
e->client_creds.tokens.insert("CAP::sys_tty_config") ;
e->client_creds.tokens.insert("SRC::com.nokia.maemo") ;
e->client_creds.tokens.insert("timed-tools::timed-tools") ;
e->client_creds.tokens.insert("timed::TimeBackup") ;
e->client_creds.tokens.insert("timed::TimeControl") ;
#endif

e->b_attr.resize(4) ;

e->snooze.resize(5) ;
e->snooze[0] = 1 ;
e->snooze[1] = 10 ;
e->snooze[2] = 17 ;
e->snooze[3] = 60 ;
e->snooze[4] = 0 ;

e->ticker = ticker_t (time(NULL)+1000);
