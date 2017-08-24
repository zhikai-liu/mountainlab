#include "mpdaemonmonitorinterface.h"
#include <QEventLoop>
#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

Q_LOGGING_CATEGORY(II, "mp.daemon_monitor_interface")

static QString s_mpdaemon_monitor_url = "";
static QString s_mpdaemon_name = "";
static QString s_mpdaemon_secret = "";

void MPDaemonMonitorInterface::setMPDaemonMonitorUrl(QString url)
{
    qCInfo(II) << "Setting mpdaemonmonitor url:" << url;
    s_mpdaemon_monitor_url = url;
}

void MPDaemonMonitorInterface::setDaemonName(QString name)
{
    qCInfo(II) << "Setting mpdaemon name:" << name;
    s_mpdaemon_name = name;
}

void MPDaemonMonitorInterface::setDaemonSecret(QString secret)
{
    qCInfo(II) << "Setting mpdaemon secret (length):" << secret.count();
    s_mpdaemon_secret = secret;
}

QString http_get_text(QString url)
{
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QTimer timeoutTimer;
    timeoutTimer.setInterval(2000); // 2s of inactivity causes us to break the connection
    QObject::connect(&timeoutTimer, SIGNAL(timeout()), reply, SLOT(abort()));
    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), &timeoutTimer, SLOT(start()));
    timeoutTimer.start();
    loop.exec();
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return QString();
    }
    QTextStream stream(reply);
    reply->deleteLater();
    return stream.readAll();
}

void MPDaemonMonitorInterface::sendEvent(MPDaemonMonitorEvent event)
{
    if (s_mpdaemon_monitor_url.isEmpty())
        return;
    if (s_mpdaemon_name.isEmpty())
        return;
    if (s_mpdaemon_secret.isEmpty())
        return;
    QString query = "";
    query += "&daemon_name=" + s_mpdaemon_name;
    query += "&daemon_secret=" + s_mpdaemon_secret;
    query += "&type=" + event.type;
    query += "&process_id=" + event.process_id;
    query += "&processor_name=" + event.processor_name;
    if (!event.process_error.isEmpty()) {
        query += "&process_error=" + event.process_error.toUtf8().toPercentEncoding();
    }
    QString url0 = QString("%1/mpdaemonmonitor?a=add_event" + query).arg(s_mpdaemon_monitor_url);

    qCInfo(II).noquote() << url0;
    QTime timer;
    timer.start();
    QString txt = http_get_text(url0);
    qCInfo(II).noquote() << "RESPONSE:" << txt << "Elapsed:" << timer.elapsed();
}
