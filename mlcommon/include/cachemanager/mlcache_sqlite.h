#ifndef MLCACHE_SQLITE_H
#define MLCACHE_SQLITE_H

#include "mlcache_backend.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>

// TODO: Extract as plugin

class MLCacheBackendSqlite : public MLCacheBackend {
public:
    const int current_schema_version = 1;

    MLCacheBackendSqlite(const QDir& dir);
    ~MLCacheBackendSqlite();

    virtual void expireFiles();

    virtual Id file(const QString& fileName, size_t duration);
    virtual QDateTime validTo(const Id& id) const;
    uint32_t pid(const Id& id) const;
    uint64_t duration(const Id& id) const;
    void setDuration(const Id& id, size_t duration);
    void setPid(const Id& id, uint32_t pid);
    void clearPid(const Id& id);

private:
    bool createTables();

    QSqlQuery query(const QString& statement,
        const QVariantMap& binds = QVariantMap()) const;
    QVariant getInformation(const QString& key,
        const QVariant& defaultValue = QVariant()) const;
    void setInformation(const QString& key, const QVariant& value);

    QSqlDatabase m_db;
    static QAtomicInt m_dbId;
};

#endif // MLCACHE_SQLITE_H
