/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland, Witold Wysota
** Created: 5/2/2016
*******************************************************/

#include "mpdaemoninterface.h"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QProcess>
#include <QTime>
#include <stdio.h>
#include "mpdaemon.h"

#include <QDebug>
#include <QJsonArray>
#include <QSharedMemory>
#include "mlcommon.h"
#include <signal.h>
#include "localserver.h"
#include <QThread>

class MountainProcessClient : public LocalClient::Client {
public:
    MountainProcessClient(QObject* parent = 0)
        : LocalClient::Client(parent)
    {
    }
    QByteArray waitForMessage(int ms = 30000)
    {
        m_waitingForMessage = true;
        while (true) {
            if (!waitForReadyRead(ms))
                return QByteArray(); // failure
            //            qApp->sendPostedEvents(this);
            if (!m_waitingForMessage) {
                QByteArray result = m_msg;
                m_msg.clear();
                return result;
            }
        }
    }

protected:
    void handleMessage(const QByteArray& ba) Q_DECL_OVERRIDE
    {
        if (m_waitingForMessage) {
            m_msg = ba;
            m_waitingForMessage = false;
            return;
        }
    }
    bool m_waitingForMessage = false;
    QByteArray m_msg;
};

MPDaemonClient::MPDaemonClient()
{
    m_client = new MountainProcessClient;
    m_connected = connectToServer();
}

MPDaemonClient::~MPDaemonClient()
{
    delete m_client;
}

QJsonObject MPDaemonClient::state()
{
    if (!isConnected()) {
        return QJsonObject();
    }
    QJsonDocument response = send(commandTemplate("get-daemon-state"));
    return response.object();
}

QJsonArray MPDaemonClient::log()
{
    if (!isConnected()) {
        return QJsonArray();
    }
    QJsonDocument response = send(commandTemplate("get-log"));
    return response.array();
}

void MPDaemonClient::contignousLog()
{
}

bool MPDaemonClient::queueScript(const MPDaemonPript& script)
{
    //if (!ensureDaemonRunning())
    //    return false;
    QJsonObject obj = pript_struct_to_obj(script, FullRecord);
    obj["command"] = "queue-script";
    sendOneWay(obj);
    return true;
}

bool MPDaemonClient::queueProcess(const MPDaemonPript& process)
{
    //if (!ensureDaemonRunning())
    //    return false;
    QJsonObject obj = pript_struct_to_obj(process, FullRecord);
    obj["command"] = "queue-process";
    sendOneWay(obj);
    return true;
}

bool MPDaemonClient::clearProcessing()
{
    if (!isConnected()) {
        return false;
    }
    sendOneWay(commandTemplate("clear-processing"));
    return true;
}

bool MPDaemonClient::start()
{
    if (daemonRunning()) {
        printf("Cannot start: daemon is already running.\n");
        return true;
    }
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "daemon-start" << qgetenv("MP_DAEMON_ID");
    if (!QProcess::startDetached(exe, args)) {
        printf("Unable to startDetached: %s\n", exe.toLatin1().data());
        return false;
    }
    for (int i = 0; i < 10; i++) {
        MPDaemon::wait(100);
        if (daemonRunning())
            return true;
    }
    printf("Unable to start daemon after waiting.\n");
    return false;
}

bool MPDaemonClient::stop()
{
    if (!daemonRunning())
        return true;
    sendOneWay(commandTemplate("stop"));
    QThread::sleep(1);
    return !daemonRunning();
}

/*
bool MPDaemonClient::ensureDaemonRunning() {
    if (daemonRunning())
        return true;
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "daemon-start";mp
    if (!QProcess::startDetached(exe, args)) {
        printf("Unable to startDetached: %s\n", exe.toLatin1().constData());
        return false;
    }
    for (int i = 0; i < 10; i++) {
        MPDaemon::wait(100);
        if (daemonRunning())
            return true;
    }
    return daemonRunning();
}
*/

bool MPDaemonClient::connectToServer()
{
    m_client->connectToServer(socketName());
    return m_client->waitForConnected();
}

void MPDaemonClient::sendOneWay(const QJsonObject& req)
{
    m_client->writeMessage(QJsonDocument(req).toJson());
}

QJsonDocument MPDaemonClient::send(const QJsonObject& req, int ms)
{
    m_client->writeMessage(QJsonDocument(req).toJson());
    QByteArray ret = m_client->waitForMessage(ms);
    if (ret.isEmpty())
        return QJsonDocument();
    QJsonParseError error;
    QJsonDocument result = QJsonDocument::fromJson(ret, &error);
    return result;
}

QJsonObject MPDaemonClient::commandTemplate(const QString& cmd) const
{
    QJsonObject obj;
    obj["command"] = cmd;
    return obj;
}

bool MPDaemonClient::daemonRunning() const
{
    QSharedMemory shm(shmName());
    if (!shm.attach(QSharedMemory::ReadOnly))
        return false;
    shm.lock();
    const MountainProcessDescriptor* desc = reinterpret_cast<const MountainProcessDescriptor*>(shm.constData());
    bool ret = (kill(desc->pid, 0) == 0);
    shm.unlock();
    return ret;
}

QString MPDaemonIface::socketName() const
{
    QString daemon_id = qgetenv("MP_DAEMON_ID");
    return QString("mountainprocess-%1.sock").arg(daemon_id);
}

QString MPDaemonIface::shmName() const
{
    QString daemon_id = qgetenv("MP_DAEMON_ID");
    return QString("mountainprocess-%1").arg(daemon_id);
}
