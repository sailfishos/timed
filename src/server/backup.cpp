#include "backup.h"

void Timed::backup()
{
 /* Excerpts from Backup User Guide V0.2 ...
  *
  * 2.1 Backup Start
  *
  * This method will be called just before the actual backup
  * starts. The application can dump all the data in RAM to their
  * persistent storage. If the application wish to exit on getting
  * this call, it can do that after responding DBus calls. (The
  * application which uses temporary backup files can generate the
  * the file on getting this call.)
  */

  system("mkdir /tmp/.timed-backup; cp /var/cache/timed/*.data /etc/timed.rc /etc/timed-cust.rc /tmp/.timed-backup; chmod -R 0777 /tmp/.timed-backup");
}

void Timed::backup_finished()
{
  /* 2.2 Backup Finished
   *
   * The applications are notified that backup is finished. (The
   * applications which uses the temporary backup files can delete
   * the file on getting this call.)
   */

  system("rm -rf /tmp/.timed-backup");
}

void Timed::restore()
{
  /* 2.3 Restore Start
   *
   * The applications are notified that restore operation is going
   * to start. If the application wish to exit on getting this call,
   * it can do that.
   */
}

void Timed::restore_finished()
{
  /* 2.4 Restore Finished
   *
   * The applications are notified that one restore operation is
   * finished. On getting this method call the application should
   * reload all data from their persistent storage and update the
   * UI. (The applications which uses the temporary backup files can
   * import data from temporary files and delete them after it.)
   */

  system("cp -f /tmp/.timed-backup/*.data /var/cache/timed; cp -f /tmp/.timed-backup/*.rc /etc");
  backup_finished();
  QCoreApplication::exit(1);
}

