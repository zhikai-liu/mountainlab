/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#ifndef MPDAEMON_H
#define MPDAEMON_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QJsonObject>
#include <QProcess>
#include <QFile>
#include <QJsonArray>
#include "localserver.h"
#include "mpdaemoninterface.h"
#include "processmanager.h" //for RequestProcessResources

struct ProcessResources {
    double num_threads = 0;
    double memory_gb = 0;
    double num_processes = 0;
};

class MPDaemonPrivate;
class MPDaemon : public QObject {
    Q_OBJECT
public:
    friend class MPDaemonPrivate;
    MPDaemon();
    virtual ~MPDaemon();
    void setTotalResourcesAvailable(ProcessResources PR);
    void setLogPath(const QString& path);
    bool run();
    void clearProcessing();

    static QString daemonPath();
//    static QString makeTimestamp(const QDateTime& dt = QDateTime::currentDateTime());
//    static QDateTime parseTimestamp(const QString& timestamp);
    static bool waitForFileToAppear(QString fname, qint64 timeout_ms = -1, bool remove_on_appear = false, qint64 parent_pid = 0, QString stdout_fname = "");
    static void wait(qint64 msec);
    static bool pidExists(qint64 pid);
    static bool waitForFinishedAndWriteOutput(QProcess* P);

private slots:
    void slot_pript_qprocess_finished();
    void slot_qprocess_output();
    void iterate();

private:
    MPDaemonPrivate* d;
};

class QSharedMemory;
class MountainProcessServer : public LocalServer::Server, public MPDaemonIface {
public:
    MountainProcessServer(QObject* parent = 0);
    void distributeLogMessage(const QJsonObject &msg);
    void registerLogListener(LocalServer::Client* listener);
    void unregisterLogListener(LocalServer::Client* listener);

    QJsonObject state() override;
    QJsonArray log() override;
    void contignousLog() override;
    bool queueScript(const MPDaemonPript &script) override;
    bool queueProcess(const MPDaemonPript &process) override;
    bool clearProcessing() override;
    bool start() override;
    bool stop() override;

protected:
    LocalServer::Client* createClient(QLocalSocket* sock);
    void clientAboutToBeDestroyed(LocalServer::Client* client);

    bool startServer();
    void stopServer();
    bool acquireServer();
    bool releaseServer();
    bool acquireSocket();
    bool releaseSocket();
    void iterate();

    void writeLogRecord(QString record_type, QString key1 = "", QVariant val1 = QVariant(), QString key2 = "", QVariant val2 = QVariant(), QString key3 = "", QVariant val3 = QVariant());
    void writeLogRecord(QString record_type, const QJsonObject& obj);

private:
    QList<LocalServer::Client*> m_listeners;
    bool m_is_running = false;
    QSharedMemory *shm = nullptr;
    QJsonArray m_log;
    QMap<QString, MPDaemonPript> m_pripts;
};

struct ProcessRuntimeOpts {
    ProcessRuntimeOpts()
    {
        num_threads_allotted = 1;
        memory_gb_allotted = 1;
    }
    double num_threads_allotted = 1;
    double memory_gb_allotted = 0;
};

bool is_at_most(ProcessResources needed, ProcessResources available, ProcessResources total_allocated);

enum PriptType {
    ScriptType,
    ProcessType
};

struct MPDaemonPript {
    //Represents a process or a script
    PriptType prtype = ScriptType;
    QString id;
    QString output_fname;
    QString stdout_fname;
    QVariantMap parameters;
    bool is_running = false;
    bool is_finished = false;
    bool success = false;
    QString error;
    QJsonObject runtime_results;
    qint64 parent_pid = 0;
    bool force_run;
    QString working_path;
    QDateTime timestamp_queued;
    QDateTime timestamp_started;
    QDateTime timestamp_finished;
    QProcess* qprocess = 0;
    QFile* stdout_file = 0;

    //For a script:
    QStringList script_paths;
    QStringList script_path_checksums; //to ensure that scripts have not changed at time of running

    //For a process:
    QString processor_name;
    //double num_threads_requested = 1;
    //double memory_gb_requested = 0;
    RequestProcessResources RPR;
    ProcessRuntimeOpts runtime_opts; //defined at run time
};

enum RecordType {
    AbbreviatedRecord,
    FullRecord,
    RuntimeRecord
};

QJsonObject pript_struct_to_obj(MPDaemonPript S, RecordType rt);
MPDaemonPript pript_obj_to_struct(QJsonObject obj);
QStringList json_array_to_stringlist(QJsonArray X);
QJsonObject variantmap_to_json_obj(QVariantMap map);
QJsonObject runtime_opts_struct_to_obj(ProcessRuntimeOpts opts);

// this has to be a POD -> no pointers, no dynamic memory allocation
struct MountainProcessDescriptor {
    uint32_t version = 1;
    pid_t pid;
};

#endif // MPDAEMON_H
