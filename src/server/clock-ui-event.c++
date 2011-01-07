e->flags = EventFlags::Alarm |
           EventFlags::Aligned_Snooze |
           EventFlags::Backup |
           EventFlags::Boot |
           EventFlags::Hide1 |
           EventFlags::Keep_Alive |
           EventFlags::Reminder ;

e->attr.txt["APPLICATION"] = "clock" ;
e->attr.txt["PLUGIN"] = "libclockalarm" ;
e->attr.txt["TITLE"] = "Alarm" ;
e->attr.txt["alarmtime"] = "17:08" ;
e->attr.txt["enabled"] = "1" ;
e->attr.txt["recurrence"] = "124560" ;
e->attr.txt["snooze"] = "25" ;
e->attr.txt["sound"] = "/usr/share/sounds/ring-tones/Clock alert 1.aac" ;
e->attr.txt["trigger"] = "1294412880" ;


e->b_attr.resize(1) ;

e->snooze.resize(2) ;
e->snooze[0] = 1500 ;
e->snooze[1] = 1500 ;

e->recrs.resize(1) ;
e->recrs[0].mins = 1<<8 ;
e->recrs[0].hour = 1<<17 ;
e->recrs[0].mday = ~0 ;
e->recrs[0].wday = 0137 ;
e->recrs[0].mons = 07777 ;

e->tsz_max = 3 ;
