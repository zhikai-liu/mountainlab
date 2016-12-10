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

#if 0

class MPDaemonInterfacePrivate {
public:
    MPDaemonInterfacePrivate(MPDaemonInterface* qq)
        : q(qq)
    {
        client = new MountainProcessClient;
    }
    ~MPDaemonInterfacePrivate()
    {
        delete client;
    }

    MPDaemonInterface* q;
    MountainProcessClient* client;

    bool daemon_is_running();
    bool send_daemon_command(QJsonObject obj, qint64 timeout_msec);
    QDateTime get_time_from_timestamp_of_fname(QString fname);
    QJsonObject get_last_daemon_state();
    QJsonObject get_log();
    void log_loop();

    QString shmName() const
    {
        QString tpl = QStringLiteral("mountainprocess-%1");
#ifdef Q_OS_UNIX
        QString username = qgetenv("USER");
#elif defined(Q_OS_WIN)
        QString username = qgetenv("USERNAME");
#else
        QString username = "unknown";
#endif
        return tpl.arg(username);
    }

    QString socketName() const
    {
        QString tpl = QStringLiteral("mountainprocess-%1.sock");
#ifdef Q_OS_UNIX
        QString username = qgetenv("USER");
#elif defined(Q_OS_WIN)
        QString username = qgetenv("USERNAME");
#else
        QString username = "unknown";
#endif
        return tpl.arg(username);
    }
};

MPDaemonInterface::MPDaemonInterface()
{
    d = new MPDaemonInterfacePrivate(this);
}

MPDaemonInterface::~MPDaemonInterface()
{
    delete d;
}

bool MPDaemonInterface::start()
{
    if (d->daemon_is_running()) {
        printf("Cannot start: daemon is already running.\n");
        return true;
    }
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "daemon-start-internal";
    if (!QProcess::startDetached(exe, args)) {
        printf("Unable to startDetached: %s\n", exe.toLatin1().data());
        return false;
    }
    MPDaemon::wait(500);
    for (int i = 0; i < 10; i++) {
        printf(".");
        if (d->daemon_is_running()) {
            printf("\n");
            printf("Daemon is now running.\n");
            return true;
        }
        MPDaemon::wait(1000);
    }
    printf("\n");
    printf("Unable to start daemon after waiting.\n");
    return false;
}

bool MPDaemonInterface::stop()
{
    if (!d->daemon_is_running()) {
        printf("Cannot stop: daemon is not running.\n");
        return true;
    }
    QJsonObject obj;
    obj["command"] = "stop";
    if (!d->send_daemon_command(obj, 5000)) {
        printf("Unable to send daemon command after waiting.\n");
        return false;
    }
    QTime timer;
    timer.start();
    while (d->daemon_is_running()) {
        MPDaemon::wait(100);
        //QThread::sleep(1);
        if (timer.elapsed() > 10000) {
            printf("Failed to stop daemon after waiting.\n");
            return false;
        }
    }
    printf("daemon has been stopped.\n");
    return true;
}

QJsonObject MPDaemonInterface::getDaemonState()
{
    return d->get_last_daemon_state();
}

bool MPDaemonInterface::daemonIsRunning()
{
    return d->daemon_is_running();
}

QJsonObject MPDaemonInterface::getLog()
{
    return d->get_log();
}

void MPDaemonInterface::logLoop()
{
    d->log_loop();
}

static QString daemon_message = "Open a terminal and run [mountainprocess daemon-start], and keep that terminal open. Alternatively use tmux to run the daemon in the background.";

bool MPDaemonInterface::queueScript(const MPDaemonPript& script)
{
    if (!d->daemon_is_running()) {
        if (!this->start()) {
            printf("Problem in queueScript: Unable to start daemon.\n");
            return false;
        }
    }
    QJsonObject obj = pript_struct_to_obj(script, FullRecord);
    obj["command"] = "queue-script";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterface::queueProcess(const MPDaemonPript& process)
{
    if (!d->daemon_is_running()) {
        if (!this->start()) {
            printf("Problem in queueProcess: Unable to start daemon.\n");
            return false;
        }
    }
    QJsonObject obj = pript_struct_to_obj(process, FullRecord);
    obj["command"] = "queue-process";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterface::clearProcessing()
{
    QJsonObject obj;
    obj["command"] = "clear-processing";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterfacePrivate::daemon_is_running()
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

bool MPDaemonInterfacePrivate::send_daemon_command(QJsonObject obj, qint64 msec_timeout)
{
    if (!msec_timeout)
        msec_timeout = 1000;
    if (!client->isConnected()) {
        client->connectToServer(socketName());
    }
    if (!client->waitForConnected())
        return false;
    client->writeMessage(QJsonDocument(obj).toJson());
    QByteArray msg = client->waitForMessage();
    if (msg.isEmpty())
        return false;
    // TODO: parse message
    return true;
}

QJsonObject MPDaemonInterfacePrivate::get_last_daemon_state()
{
    QJsonObject ret;
    if (!daemon_is_running()) {
        ret["is_running"] = false;
        return ret;
    }
    client->connectToServer(socketName());
    if (!client->waitForConnected()) {
        qWarning() << "Can't connect to daemon";
        return ret;
    }
    QJsonObject obj;
    obj["command"] = "get-daemon-state";
    client->writeMessage(QJsonDocument(obj).toJson());
    QByteArray msg = client->waitForMessage();
    if (msg.isEmpty())
        return ret;

    QJsonParseError error;
    ret = QJsonDocument::fromJson(msg, &error).object();
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error in get_last_daemon_state parsing json";
    }
    return ret;
}

QJsonObject MPDaemonInterfacePrivate::get_log()
{
    QJsonObject ret;
    client->connectToServer(socketName());
    if (!client->waitForConnected()) {
        qWarning() << "Can't connect to daemon";
        ret["error"] = "Can't connect to daemon";
        return ret;
    }
    QJsonObject obj;
    obj["command"] = "get-log";
    client->writeMessage(QJsonDocument(obj).toJson());
    QByteArray msg = client->waitForMessage();
    if (msg.isEmpty()) return ret;
    QJsonParseError error;
    ret = QJsonDocument::fromJson(msg, &error).object();
    return ret;
}

void MPDaemonInterfacePrivate::log_loop()
{
    QJsonObject ret;
    client->connectToServer(socketName());
    if (!client->waitForConnected()) {
        qWarning() << "Can't connect to daemon";
        ret["error"] = "Can't connect to daemon";
        return;
    }
    QJsonObject obj;
    obj["command"] = "log-listener";
    client->writeMessage(QJsonDocument(obj).toJson());
    while(1) {
        QByteArray msg = client->waitForMessage(-1);
        if (msg.isEmpty())
            return;
        QTextStream(stdout) << msg << endl;
    }
}

QDateTime MPDaemonInterfacePrivate::get_time_from_timestamp_of_fname(QString fname)
{
    QStringList list = QFileInfo(fname).fileName().split(".");
    return MPDaemon::parseTimestamp(list.value(0));
}

#endif

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
    args << "daemon-start";
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
    args << "daemon-start";
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
    return substitute("mountainprocess-%u.sock");
}

QString MPDaemonIface::shmName() const
{
    return substitute("mountainprocess-%u");
}
