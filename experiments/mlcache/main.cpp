#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QSharedPointer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QtDebug>
class Transaction {
public:
    Transaction(QSqlDatabase db) {
        m_db = db;
        m_db.transaction();
    }
    ~Transaction() {
        rollback();
    }
    bool commit() {
        m_active = !m_db.commit();
        return !m_active;
    }
    bool transaction() {
        if (m_active) return false;
        m_active = m_db.transaction();
        return m_active;
    }
    bool rollback() {
        if (!m_active) return true;
        m_active = !m_db.rollback();
        return !m_active;
    }

    bool isActive() const { return m_active; }

private:
    QSqlDatabase m_db;
    bool m_active = true;
};

class MLCacheFileInfo : public QFileInfo {

};

using MLCacheFileInfoList = QList<MLCacheFileInfo>;

class MLCacheBackend {
public:
    class Id {
    public:
        Id() {}
        ~Id() {}
        Id(const Id& other) : m_ptr(other.m_ptr) {}
        void* internalPointer() const { return m_ptr; }
        quintptr internalId() const { return (quintptr)m_ptr; }
        QString fileName() const { return m_name; }
        bool isValid() const { return !fileName().isEmpty(); }
        friend class MLCacheBackend;
    private:
        void* m_ptr = nullptr;
        QString m_name;
    };

    MLCacheBackend(const QDir& dir) : m_dir(dir) {}
    QDir dir() const { return m_dir; }
    virtual ~MLCacheBackend(){}
    virtual void expireFiles() = 0;
    virtual Id file(const QString& fileName, size_t duration = 0) = 0;
    virtual QDateTime validTo(const Id& id) const = 0;
protected:
    Id createId(const QString& fileName, void* ptr) {
        Id id;
        id.m_name = fileName;
        id.m_ptr = ptr;
        return id;
    }
    Id createId(const QString& fileName, quintptr v) {
        Id id;
        id.m_name = fileName;
        id.m_ptr = (void*)v;
        return id; }
private:
    QDir m_dir;
};

class MLCacheFile;
class MLCache {
public:
    MLCache(const QDir& dir) {
        init(dir);
    }

    MLCache(const QString& path) {
        init(QDir(path));
    }
    MLCache(const MLCache& other) : m_backend(other.m_backend) {}

    MLCacheFile* file(const QString& filename, size_t duration = 0);

    QString absolutePath() const {
        return dir().absolutePath();
    }
    QDir dir() const { return m_backend->dir(); }

    QStringList entryList() const { return QStringList(); }
    MLCacheFileInfoList entryInfoList() const {
        return MLCacheFileInfoList();
    }
    void gc() { m_backend->expireFiles(); }
private:
    void init(const QDir& dir);
    QSharedPointer<MLCacheBackend> m_backend = nullptr;
};

class MLCacheBackendSqlite : public MLCacheBackend {
public:
    const int current_schema_version = 1;

    MLCacheBackendSqlite(const QDir& dir) : MLCacheBackend(dir) {
        int id = (MLCacheBackendSqlite::m_dbId+=1);
        m_db = QSqlDatabase::addDatabase("QSQLITE", QString("MLCacheBackendSqlite_%1").arg(id));
        m_db.setDatabaseName(dir.absoluteFilePath(".mlcache"));
        m_db.open();

        createTables();
    }
    ~MLCacheBackendSqlite() {
        QString name = m_db.connectionName();
        m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(name);
    }

    virtual void expireFiles() {
        Transaction t(m_db);
        // mark expired files as deleted
        query("UPDATE Files SET deleted=1 WHERE datetime(created, duration) < CURRENT_TIMESTAMP").exec();
        // get all pids in the system
        QStringList procList;
#ifdef Q_OS_LINUX
        QDir proc("/proc");
        QStringList procDirList = proc.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        QRegExp rx("(\\d+)");
        foreach(const QString& procDir, procDirList) {
            if (rx.exactMatch(procDir))
                procList << procDir;
        }
#endif
        if (!procList.isEmpty())
            query("UPDATE Files SET deleted=1 WHERE PID IS NOT NULL AND pid NOT IN (" + procList.join(", ")+")").exec();

        // select all deleted
        QSqlQuery deleted = query("SELECT name FROM Files WHERE deleted = 1");
        if (!deleted.exec()) {
            return;
        }
        while(deleted.next()) {
            QString fileName = deleted.value("name").toString();
            dir().remove(fileName);
        }
        query("DELETE FROM Files WHERE deleted = 1").exec();
        t.commit();
    }

    virtual Id file(const QString &fileName, size_t duration)
    {
        Transaction t(m_db);
        int id = 0;
        QSqlQuery q = query("SELECT ROWID FROM Files WHERE name = :name", {{":name", fileName}});
        if (!q.exec()) return Id();
        if (q.next()) {
            id = q.value(0).toLongLong();
        } else {
            q = query("INSERT INTO Files (name, duration) VALUES (:name, :duration)");
            q.bindValue(":name", fileName);
            q.bindValue(":duration", QString("+%1 seconds").arg((qulonglong)duration));
            if (!q.exec()) return Id();
            id = q.lastInsertId().toLongLong();
        }
        q = query("SELECT * FROM Files WHERE ROWID=:id");
        q.bindValue(":id", id);
        if (!q.exec()) {
            return Id();
        }
        if (!t.commit())
            return Id();
        qDebug() << "Returning:" << id;
        return createId(fileName, id);
    }
    virtual QDateTime validTo(const Id &id) const {
        if (!id.isValid()) return QDateTime();
        int fileId = id.internalId();
        QSqlQuery q = query("SELECT datetime(created, duration) FROM Files WHERE ROWID=:id");
        q.bindValue(":id", fileId);
        if (!q.exec() || !q.next()) return QDateTime();
        return q.value(0).toDateTime();
    }

private:
    bool createTables() {
        Transaction t(m_db);
        int schema_version = 0;
        qDebug() << "Tables:" << m_db.tables();
        if (!m_db.tables().contains("Information")) {
            qDebug() << "Creating info table";
            if (!query("CREATE TABLE Information (key TEXT NOT NULL UNIQUE, value TEXT)").exec()) {
                return false;
            }
            setInformation("dirPath", dir().absolutePath());
        } else {
            schema_version = getInformation("version", 0).toInt();
            if (schema_version == current_schema_version)
                return true; // here we're doing a rollback but nothing was changed so that's ok
        }
        qDebug() << "schema:" << schema_version;
        if (schema_version == 0) {
            qDebug() << "Creating Files table";
            if(!query("CREATE TABLE IF NOT EXISTS Files ( \
                      name TEXT NOT NULL UNIQUE, \
                      created INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP, \
                      duration TEXT DEFAULT NULL, \
                      pid INTEGER DEFAULT NULL, \
                      deleted INTEGER NOT NULL DEFAULT 0 \
                      )")
                    .exec())
                return false;
            schema_version = 1;
        }
        if (schema_version == 1) {
            // update to schema_version = 2
            // etc.
        }

        setInformation("version", 1);
        t.commit();
        return true;
    }

    QSqlQuery query(const QString &statement, const QVariantMap& binds = QVariantMap()) const{
        QSqlQuery q = QSqlQuery(m_db);
        if (!q.prepare(statement)) return QSqlQuery();
        for(QVariantMap::const_iterator iter = binds.begin(); iter != binds.end(); ++iter) {
            q.bindValue(iter.key(), iter.value());
        }
        return q;
    }
    QVariant getInformation(const QString& key, const QVariant &defaultValue = QVariant()) const {
        QSqlQuery q = query("SELECT value FROM Information WHERE key = :key", {{":key", key}});
        if (!q.exec() || !q.next()) return defaultValue;
        return q.value(0);
    }
    void setInformation(const QString& key, const QVariant& value) {
        QSqlQuery q = query("INSERT OR REPLACE INTO Information (key, value) VALUES (:key, :value)");
        q.bindValue(":key", key);
        q.bindValue(":value", value.toString());
        q.exec();
    }

    QSqlDatabase m_db;
    static QAtomicInt m_dbId;

};

QAtomicInt MLCacheBackendSqlite::m_dbId;

class MLCacheFile : public QFile {
public:
    void setDuration(size_t duration);
    size_t duration() const;
    friend class MLCache;
private:
    MLCacheFile(MLCache c, const MLCacheBackend::Id& id)
        : QFile(c.dir().absoluteFilePath(id.fileName())),
          m_cache(c),
          m_id(id) {

    }
    MLCache m_cache;
    MLCacheBackend::Id m_id;
};

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    MLCache cache("/tmp/testcache");
    MLCacheFile* f = cache.file("abc", 6);
    f->open(QIODevice::WriteOnly|QIODevice::Text);
    f->write("ABC");
    f->close();
    delete f;
    cache.gc();
    //    return app.exec();
    return 0;
}

MLCacheFile* MLCache::file(const QString &filename, size_t duration) {
    MLCacheBackend::Id id = m_backend->file(filename, duration);
    return new MLCacheFile(*this, id);
}

void MLCache::init(const QDir& dir) {
    m_backend.reset(new MLCacheBackendSqlite(dir));
}
