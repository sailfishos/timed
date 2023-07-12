#ifndef MAEMO_TIMED_DSME_MODE_H
#define MAEMO_TIMED_DSME_MODE_H

#include <string>

#include <QDBusPendingCallWatcher>
#include <QObject>

#include "interfaces.h"

struct dsme_mode_t : public QObject
{
    QDBusPendingCallWatcher *request_watcher;
    DsmeReqInterface *dsme_iface;
    bool signal_received;
    std::string mode;
    dsme_mode_t(QObject *parent = 0);

    Q_OBJECT;
Q_SIGNALS:
    void mode_reported(const std::string &mode);
    void mode_is_changing();
public Q_SLOTS:
    void init_request();
private Q_SLOTS:
    void request_finished(QDBusPendingCallWatcher *watcher);
    void dsme_state_signalled(const QString &new_mode);
};

#endif
