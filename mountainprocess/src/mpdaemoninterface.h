/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#ifndef MPDAEMONINTERFACE_H
#define MPDAEMONINTERFACE_H

#include <QJsonDocument>
#include <QJsonObject>
//#include "mpdaemon.h"

class MPDaemonPript;
class MPDaemonIface {
public:
    virtual ~MPDaemonIface() {}
    virtual QJsonObject state() = 0;
    virtual QJsonArray log() = 0;
    virtual void contignousLog() = 0;
    virtual bool queueScript(const MPDaemonPript& script) = 0;
    virtual bool queueProcess(const MPDaemonPript& process) = 0;
    virtual bool clearProcessing() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;

protected:
    QString socketName(QString daemon_id) const;
    QString shmName(QString daemon_id) const;

    /*
    static QString substitute(const QString& tpl)
    {
#ifdef Q_OS_UNIX
        QString username = qgetenv("USER");
#elif defined(Q_OS_WIN)
        QString username = qgetenv("USERNAME");
#else
        QString username = "unknown";
#endif
        QString result = tpl;
        result.replace("%u", username);
        return result;
    }
    */
};

class MountainProcessClient;
class MPDaemonClient : public MPDaemonIface {
public:
    MPDaemonClient(QString daemon_id);
    ~MPDaemonClient();

    QJsonObject state() override;
    QJsonArray log() override;
    void contignousLog() override;
    bool queueScript(const MPDaemonPript& script) override;
    bool queueProcess(const MPDaemonPript& process) override;
    bool clearProcessing() override;
    bool start() override;
    bool stop() override;

    //bool ensureDaemonRunning();
    bool isConnected() const { return m_connected; }

protected:
    bool connectToServer();
    void sendOneWay(const QJsonObject& req);

    QJsonDocument send(const QJsonObject& req, int ms = 30000);
    QJsonObject commandTemplate(const QString& cmd) const;
    bool daemonRunning() const;

private:
    QString m_daemon_id;
    MountainProcessClient* m_client;
    bool m_connected = false;
};

#if 0
class MPDaemonServer : public MPDaemonIface {
public:
    QJsonObject state() override
    {
        QJsonObject ret;

        ret["is_running"] = m_is_running;
        QJsonObject scripts;
        QJsonObject processes;
        QStringList keys = m_pripts.keys();
        foreach (QString key, keys) {
            if (m_pripts[key].prtype == ScriptType)
                scripts[key] = pript_struct_to_obj(m_pripts[key], AbbreviatedRecord);
            else
                processes[key] = pript_struct_to_obj(m_pripts[key], AbbreviatedRecord);
        }
        ret["scripts"] = scripts;
        ret["processes"] = processes;
        return ret;

    }
    QJsonArray log() override
    {
    }
    void contignousLog() override
    {
    }
    bool queueScript(const MPDaemonPript &script) override
    {

    }
    bool queueProcess(const MPDaemonPript &process) override
    {
    }
    bool clearProcessing() override
    {
        QStringList keys = d->m_pripts.keys();
        foreach (QString key, keys) {
            d->stop_or_remove_pript(key);
        }
    }
    bool start() override
    {

    }
    bool stop() override
    {
        m_is_running = false;
        qApp->quit();
    }
};
#endif

#if 0
class MPDaemonInterfacePrivate;
class MPDaemonInterface {
public:
    friend class MPDaemonInterfacePrivate;
    MPDaemonInterface();
    virtual ~MPDaemonInterface();
    bool start();
    bool stop();
    QJsonObject getDaemonState();
    bool daemonIsRunning();
    bool queueScript(const MPDaemonPript& script);
    bool queueProcess(const MPDaemonPript& process);
    bool clearProcessing();

private:
    MPDaemonInterfacePrivate* d;
};
#endif

#endif // MPDAEMONINTERFACE_H
