#ifndef MLCACHE_BACKEND_H
#define MLCACHE_BACKEND_H

#include <QDir>
#include <QDateTime>

class MLCacheBackend {
public:
    class Id {
    public:
        Id();
        ~Id();
        Id(const Id& other);
        void* internalPointer() const;
        quintptr internalId() const;
        QString fileName() const;
        bool isValid() const;
        friend class MLCacheBackend;

    private:
        void* m_ptr = nullptr;
        QString m_name;
    };

    MLCacheBackend(const QDir& dir);
    QDir dir() const;
    virtual ~MLCacheBackend();
    virtual void expireFiles() = 0;
    virtual Id file(const QString& fileName, size_t duration = 0) = 0;
    virtual QDateTime validTo(const Id& id) const = 0;
    virtual uint32_t pid(const Id& id) const = 0;
    virtual uint64_t duration(const Id& id) const = 0;
    virtual void setDuration(const Id& id, size_t duration) = 0;
    virtual void setPid(const Id& id, uint32_t pid) = 0;
    virtual void clearPid(const Id& id) = 0;

protected:
    Id createId(const QString& fileName, void* ptr);
    Id createId(const QString& fileName, quintptr v);

private:
    QDir m_dir;
};

#endif // MLCACHE_BACKEND_H
