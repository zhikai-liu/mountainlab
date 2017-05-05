#include "mlcache.h"
#include "mlcache_sqlite.h"

MLCacheBackend::Id::Id() {}
MLCacheBackend::Id::~Id() {}

MLCacheBackend::Id::Id(const MLCacheBackend::Id& other)
    : m_ptr(other.m_ptr)
{
}

void* MLCacheBackend::Id::internalPointer() const
{
    return m_ptr;
}

quintptr MLCacheBackend::Id::internalId() const
{
    return (quintptr)m_ptr;
}

QString MLCacheBackend::Id::fileName() const
{
    return m_name;
}

bool MLCacheBackend::Id::isValid() const
{
    return !fileName().isEmpty();
}

MLCacheBackend::MLCacheBackend(const QDir& dir)
    : m_dir(dir)
{
}

QDir MLCacheBackend::dir() const
{
    return m_dir;
}

MLCacheBackend::~MLCacheBackend() {}

MLCacheBackend::Id MLCacheBackend::createId(const QString& fileName,
    void* ptr)
{
    Id id;
    id.m_name = fileName;
    id.m_ptr = ptr;
    return id;
}

MLCacheBackend::Id MLCacheBackend::createId(const QString& fileName,
    quintptr v)
{
    Id id;
    id.m_name = fileName;
    id.m_ptr = (void*)v;
    return id;
}

MLCache::MLCache(const QDir& dir)
{
    init(dir);
}

MLCache::MLCache(const QString& path)
{
    init(QDir(path));
}

MLCache::MLCache(const MLCache& other)
    : m_backend(other.m_backend)
{
}

QDir MLCache::dir() const
{
    return m_backend->dir();
}
void MLCache::gc()
{
    m_backend->expireFiles();
}

QString MLCache::absolutePath() const
{
    return dir().absolutePath();
}

QStringList MLCache::entryList() const
{
    return QStringList();
}

MLCacheFileInfoList MLCache::entryInfoList() const
{
    return MLCacheFileInfoList();
}

void MLCacheFile::setDuration(size_t duration)
{
    m_cache->setDuration(m_id, duration);
}

size_t MLCacheFile::duration() const
{
    return 0;
}

void MLCacheFile::setPid(uint32_t pid)
{
    if (pid == 0)
        m_cache->clearPid(m_id);
    else
        m_cache->setPid(m_id, pid);
}

uint32_t MLCacheFile::pid() const
{
    return m_cache->pid(m_id);
}

MLCacheFile::MLCacheFile(MLCache c, const MLCacheBackend::Id& id)
    : QFile(c.dir().absoluteFilePath(id.fileName()))
    , m_cache(c.m_backend)
    , m_id(id)
{
}

MLCacheFile* MLCache::file(const QString& filename, size_t duration)
{
    MLCacheBackend::Id id = m_backend->file(filename, duration);
    return new MLCacheFile(*this, id);
}

void MLCache::init(const QDir& dir)
{
    m_backend.reset(new MLCacheBackendSqlite(dir));
}
