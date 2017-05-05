#include "mlcache_sqlite.h"
#include <QVariant>
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

QAtomicInt MLCacheBackendSqlite::m_dbId;

MLCacheBackendSqlite::MLCacheBackendSqlite(const QDir &dir) : MLCacheBackend(dir) {
    int id = (MLCacheBackendSqlite::m_dbId+=1);
    m_db = QSqlDatabase::addDatabase("QSQLITE", QString("MLCacheBackendSqlite_%1").arg(id));
    m_db.setDatabaseName(dir.absoluteFilePath(".mlcache"));
    m_db.open();

    createTables();
}

MLCacheBackendSqlite::~MLCacheBackendSqlite() {
    QString name = m_db.connectionName();
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

void MLCacheBackendSqlite::expireFiles() {
    Transaction t(m_db);
    // mark expired files as deleted
    query("UPDATE Files SET deleted=1 WHERE datetime(created, duration) < CURRENT_TIMESTAMP").exec();

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

MLCacheBackend::Id MLCacheBackendSqlite::file(const QString &fileName, size_t duration)
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
    return createId(fileName, id);
}

QDateTime MLCacheBackendSqlite::validTo(const MLCacheBackend::Id &id) const {
    if (!id.isValid()) return QDateTime();
    int fileId = id.internalId();
    QSqlQuery q = query("SELECT datetime(created, duration) FROM Files WHERE ROWID=:id");
    q.bindValue(":id", fileId);
    if (!q.exec() || !q.next()) return QDateTime();
    return q.value(0).toDateTime();
}

bool MLCacheBackendSqlite::createTables() {
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

QSqlQuery MLCacheBackendSqlite::query(const QString &statement, const QVariantMap &binds) const{
    QSqlQuery q = QSqlQuery(m_db);
    if (!q.prepare(statement)) return QSqlQuery();
    for(QVariantMap::const_iterator iter = binds.begin(); iter != binds.end(); ++iter) {
        q.bindValue(iter.key(), iter.value());
    }
    return q;
}

QVariant MLCacheBackendSqlite::getInformation(const QString &key, const QVariant &defaultValue) const {
    QSqlQuery q = query("SELECT value FROM Information WHERE key = :key", {{":key", key}});
    if (!q.exec() || !q.next()) return defaultValue;
    return q.value(0);
}

void MLCacheBackendSqlite::setInformation(const QString &key, const QVariant &value) {
    QSqlQuery q = query("INSERT OR REPLACE INTO Information (key, value) VALUES (:key, :value)");
    q.bindValue(":key", key);
    q.bindValue(":value", value.toString());
    q.exec();
}
