/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
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

#include "timed.h"

#include "../common/log.h"
#include "csd.h"
#include "misc.h"

#if OFONO

csd_t::csd_t(Timed *owner)
    : QObject(owner)
{
    timed = owner;
    nt = new NetworkTime;
    op = new NetworkOperator;
    static const char *time_signal1 = SIGNAL(timeInfoChanged(const NetworkTimeInfo &));
    static const char *time_signal2 = SIGNAL(timeInfoQueryCompleted(const NetworkTimeInfo &));
    static const char *time_slot1 = SLOT(csd_time_s(const NetworkTimeInfo &));
    static const char *time_slot2 = SLOT(csd_time_q(const NetworkTimeInfo &));
    bool res1 = QObject::connect(nt, time_signal1, this, time_slot1);
    bool res2 = QObject::connect(nt, time_signal2, this, time_slot2);

    if (res1 && res2)
        log_info("succesfully connected to csd time signals");
    else
        log_error("connection to cellular csd signals failed: %s %s",
                  res1 ? "" : time_signal1,
                  res2 ? "" : time_signal2);

    static const char *operator_signal = SIGNAL(
        operatorChanged(const QString &, const QString &, const QString &));
    static const char *operator_slot = SLOT(
        csd_operator_s(const QString &, const QString &, const QString &));
    bool res_op = QObject::connect(op, operator_signal, this, operator_slot);
    if (res_op)
        log_info("succesfully connected to csd network operator signal");
    else
        log_error("connection to csd network operator signal failed");

    nt->queryTimeInfo();
    QMetaObject::invokeMethod(this, "csd_operator_q", Qt::QueuedConnection);

    timer = new QTimer;
    timer->setSingleShot(true);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(wait_for_operator_timeout()));
}

csd_t::~csd_t()
{
    delete nt;
    delete op;
    delete timer;
    Q_FOREACH (const QString &m, oper_time_offs.keys())
        oper_time_offs[m].cleanup();
}

void csd_t::csd_operator_q()
{
    Q_FOREACH (const QString &modem, op->modems()) {
        QString mcc = op->mcc(modem), mnc = op->mnc(modem);
        log_notice("CSD::csd_operator_q {modem='%s', mcc='%s', mnc='%s'}",
                   modem.toStdString().c_str(),
                   mcc.toStdString().c_str(),
                   mnc.toStdString().c_str());
        process_csd_network_operator(modem, mcc, mnc);
    }
}

void csd_t::csd_operator_s(const QString &modem, const QString &mnc, const QString &mcc)
{
    log_notice("CSD::csd_operator_s {modem='%s', mcc='%s', mnc='%s'}",
               modem.toStdString().c_str(),
               mcc.toStdString().c_str(),
               mnc.toStdString().c_str());
    process_csd_network_operator(modem, mcc, mnc);
}

void csd_t::csd_time_q(const NetworkTimeInfo &nti)
{
    log_notice("CSD::csd_time_q %s", csd_network_time_info_to_string(nti).c_str());
    process_csd_network_time_info(nti);
}

void csd_t::csd_time_s(const NetworkTimeInfo &nti)
{
    log_notice("CSD::csd_time_s %s", csd_network_time_info_to_string(nti).c_str());
    process_csd_network_time_info(nti);
}

void csd_t::input_csd_network_time_info(const NetworkTimeInfo &nti)
{
    timer->stop();
    const QString &modem(nti.modem());
    if (!oper_time_offs.contains(modem))
        oper_time_offs.insert(modem, cellular_operator_time_offset_t(cellular_operator_t(nti)));
    cellular_time_t new_time(nti);
    oper_time_offs[modem].setCellularOffset(new cellular_offset_t(nti));
    if (new_time.is_valid()) {
        oper_time_offs[modem].setCellularTime(new cellular_time_t(new_time));
    }
}

void csd_t::output_csd_network_time_info(const QString &modem)
{
    timer->stop(); // paranoia
    if (!oper_time_offs.contains(modem)) {
        log_notice("cannot output csd network time info for unknown modem: %s",
                   modem.toStdString().c_str());
        return;
    }

    if (oper_time_offs[modem].offs) {
        emit csd_cellular_offset(*oper_time_offs[modem].offs);
        oper_time_offs[modem].setCellularOffset(NULL);
    }
    if (oper_time_offs[modem].time) {
        emit csd_cellular_time(*oper_time_offs[modem].time);
        oper_time_offs[modem].setCellularTime(NULL);
    }
}

const nanotime_t csd_t::old_nitz_threshold(2, 0);

void csd_t::process_csd_network_time_info(const NetworkTimeInfo &nti)
{
    if (not nti.isValid()) {
        log_notice("empty time info, ignoring it");
        return;
    }

    if (!oper_time_offs.contains(nti.modem())) {
        log_notice("ignoring time info for unknown modem: %s", nti.modem().toStdString().c_str());
        return;
    }

    nanotime_t actuality = nanotime_t::monotonic_now()
                           - nanotime_t::from_timespec(*nti.timestamp());
    input_csd_network_time_info(nti);

    // Decide if the data is to be sent immediately or to wait for operator change signal
    const QString &modem(nti.modem());
    bool current_operator = oper_time_offs[modem].offs->oper.mcc == oper_time_offs[modem].oper.mcc
                            and oper_time_offs[modem].offs->oper.mnc
                                    == oper_time_offs[modem].oper.mnc;
    bool empty_operator = oper_time_offs[modem].offs->oper.mcc.empty()
                          and oper_time_offs[modem].offs->oper.mnc.empty();
    bool input_is_old = actuality > old_nitz_threshold;
    bool send_now = input_is_old or (current_operator and not empty_operator);

    log_debug("offs->oper=%s, oper=%s, actuality=%s",
              oper_time_offs[modem].offs->oper.str().c_str(),
              oper_time_offs[modem].oper.str().c_str(),
              actuality.str().c_str());
    log_debug("current_operator=%d, empty_operator=%d, input_is_old=%d",
              current_operator,
              empty_operator,
              input_is_old);
    log_debug("send_now=%d", send_now);

    if (send_now) {
        output_csd_network_time_info(modem);
    } else {
        timer->setProperty("modem", modem);
        timer->start(operator_wait_ms);
    }
}

void csd_t::process_csd_network_operator(const QString &modem,
                                         const QString &mcc,
                                         const QString &mnc)
{
    timer->stop();
    if (!oper_time_offs.contains(modem)) {
        oper_time_offs.insert(modem, cellular_operator_time_offset_t(cellular_operator_t(mcc, mnc)));
    } else {
        oper_time_offs[modem].oper = cellular_operator_t(mcc, mnc);
        if (oper_time_offs[modem].offs) {
            oper_time_offs[modem].offs->oper = oper_time_offs[modem].oper;
        }
    }
    output_csd_network_time_info(modem); // if needed
    emit csd_cellular_operator(oper_time_offs[modem].oper, modem);
}

void csd_t::wait_for_operator_timeout() // timer slot
{
    timer->stop(); // paranoia
    QString modem = timer->property("modem").toString();
    output_csd_network_time_info(modem); // probably needed
}

string csd_t::csd_network_time_info_to_string(const NetworkTimeInfo &nti)
{
    if (not nti.isValid())
        return "{invalid}";

    ostringstream os;

    os << "{zone=" << nti.offsetFromUtc();

    QDateTime t = nti.dateTime();
    if (t.isValid()) {
        string utc = str_printf("%04d-%02d-%02d,%02d:%02d:%02d",
                                t.date().year(),
                                t.date().month(),
                                t.date().day(),
                                t.time().hour(),
                                t.time().minute(),
                                t.time().second());
        os << ", utc=" << utc;
    }

    int dst = nti.daylightAdjustment();
    if (dst != -1)
        os << ", dst=" << dst;

    os << ", mcc='" << nti.mcc().toStdString() << "'";
    os << ", mnc='" << nti.mnc().toStdString() << "'";
    os << ", modem='" << nti.modem().toStdString() << "'";

    os << ", received="
       << str_printf("%lld.%09lu", (long long) nti.timestamp()->tv_sec, nti.timestamp()->tv_nsec);

    os << "}";

    return os.str();
}
#endif //OFONO
