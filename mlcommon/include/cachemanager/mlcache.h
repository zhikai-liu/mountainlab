#ifndef MLCACHE_H
#define MLCACHE_H

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QSharedPointer>


#include "mlcache_backend.h"

class MLCacheFileInfo : public QFileInfo {

};

using MLCacheFileInfoList = QList<MLCacheFileInfo>;

class MLCacheFile;
class MLCache {
public:
    MLCache(const QDir& dir);

    MLCache(const QString& path);
    MLCache(const MLCache& other);

    MLCacheFile* file(const QString& filename, size_t duration = 0);

    QString absolutePath() const;
    QDir dir() const;

    QStringList entryList() const;
    MLCacheFileInfoList entryInfoList() const;
    void gc();
private:
    void init(const QDir& dir);
    QSharedPointer<MLCacheBackend> m_backend;
};

class MLCacheFile : public QFile {
public:
    void setDuration(size_t duration);
    size_t duration() const;
    friend class MLCache;
private:
    MLCacheFile(MLCache c, const MLCacheBackend::Id& id);
    MLCache m_cache;
    MLCacheBackend::Id m_id;
};

#endif // MLCACHE_H
