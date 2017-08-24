#ifndef MPDAEMONMONITORINTERFACE_H
#define MPDAEMONMONITORINTERFACE_H

#include <QString>

struct MPDaemonMonitorEvent {
    MPDaemonMonitorEvent(QString event_type_in)
    {
        type = event_type_in;
    }
    QString type;
    QString process_id;
    QString processor_name;
    QString process_error;
};

class MPDaemonMonitorInterface {
public:
    static void setMPDaemonMonitorUrl(QString url);
    static void setDaemonName(QString name);
    static void setDaemonSecret(QString secret);
    static void sendEvent(MPDaemonMonitorEvent event);
};

#endif // MPDAEMONMONITORINTERFACE_H
