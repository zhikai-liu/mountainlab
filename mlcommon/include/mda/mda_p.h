#ifndef MDA_P_H
#define MDA_P_H

#include <QSharedData>
#include "icounter.h"
#include <objectregistry.h>
#include <cstring>
#include "mlcommon.h"

#define MDA_MAX_DIMS 6

template <typename T>
class MdaData : public QSharedData {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;

    MdaData()
        : QSharedData()
        , m_data(0)
        , m_dims(1, 1)
        , total_size(0)
    {
        ICounterManager* manager = ObjectRegistry::getObject<ICounterManager>();
        if (manager) {
            allocatedCounter = static_cast<IIntCounter*>(manager->counter("allocated_bytes"));
            freedCounter = static_cast<IIntCounter*>(manager->counter("freed_bytes"));
            bytesReadCounter = static_cast<IIntCounter*>(manager->counter("bytes_read"));
            bytesWrittenCounter = static_cast<IIntCounter*>(manager->counter("bytes_written"));
        }
    }
    MdaData(const MdaData& other)
        : QSharedData(other)
        , m_data(0)
        , m_dims(other.m_dims)
        , total_size(other.total_size)
        , allocatedCounter(other.allocatedCounter)
        , freedCounter(other.freedCounter)
        , bytesReadCounter(other.bytesReadCounter)
        , bytesWrittenCounter(other.bytesWrittenCounter)
    {
        allocate(total_size);
        std::copy(other.m_data, other.m_data + other.totalSize(), m_data);
    }
    ~MdaData()
    {
        deallocate();
    }
    bool allocate(T value, int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1)
    {
        deallocate();
        setDims(N1, N2, N3, N4, N5, N6);
        if (N1 > 0 && N2 > 0 && N3 > 0 && N4 > 0 && N5 > 0 && N6 > 0)
            setTotalSize(N1 * N2 * N3 * N4 * N5 * N6);
        else
            setTotalSize(0);

        if (totalSize() > 0) {
            allocate(totalSize());
            if (!constData()) {
                qCritical() << QString("Unable to allocate Mda of size %1x%2x%3x%4x%5x%6 (total=%7)").arg(N1).arg(N2).arg(N3).arg(N4).arg(N5).arg(N6).arg(totalSize());
                exit(-1);
            }
            if (value == 0.0) {
                std::memset(data(), 0, totalSize() * sizeof(value_type));
            }
            else
                std::fill(data(), data() + totalSize(), value);
        }
        return true;
    }

    inline int dim(size_t idx) const { return m_dims.at(idx); }
    inline int N1() const { return dim(0); }
    inline int N2() const { return dim(1); }

    void allocate(size_t size)
    {
        m_data = (value_type*)::allocate(size * sizeof(value_type));
        if (!m_data)
            return;
        incrementBytesAllocatedCounter(totalSize() * sizeof(value_type));
    }
    void deallocate()
    {
        if (!m_data)
            return;
        free(m_data);
        incrementBytesFreedCounter(totalSize() * sizeof(value_type));
        m_data = 0;
    }
    inline size_t totalSize() const { return total_size; }
    inline void setTotalSize(size_t ts) { total_size = ts; }
    inline T* data() { return m_data; }
    inline const T* constData() const { return m_data; }
    inline T at(size_t idx) const { return *(constData() + idx); }
    inline T at(size_t i1, size_t i2) const { return at(i1 + dim(0) * i2); }
    inline void set(T val, size_t idx) { m_data[idx] = val; }
    inline void set(T val, size_t i1, size_t i2) { set(val, i1 + dim(0) * i2); }

    inline int dims(size_t idx) const
    {
        if (idx < 0 || idx >= m_dims.size())
            return 0;
        return *(m_dims.data() + idx);
    }
    void setDims(int n1, int n2, int n3, int n4, int n5, int n6)
    {
        m_dims.resize(MDA_MAX_DIMS);
        m_dims[0] = n1;
        m_dims[1] = n2;
        m_dims[2] = n3;
        m_dims[3] = n4;
        m_dims[4] = n5;
        m_dims[5] = n6;
    }

    int determine_num_dims(int N1, int N2, int N3, int N4, int N5, int N6) const
    {
        Q_UNUSED(N1);
        Q_UNUSED(N2);
        //changed this on 2/21/17 by jfm
        //if (!(N6 > 0 && N5 > 0 && N4 > 0 && N3 > 0 && N2 > 0 && N1 > 0))
        //    return 0;
        if (N6 != 1)
            return 6;
        if (N5 != 1)
            return 5;
        if (N4 != 1)
            return 4;
        if (N3 != 1)
            return 3;
        return 2;
    }
    bool safe_index(size_t i) const
    {
        return (i < totalSize());
    }
    bool safe_index(size_t i1, size_t i2) const
    {
        return (((int)i1 < dims(0)) && ((int)i2 < dims(1)));
    }
    bool safe_index(size_t i1, size_t i2, size_t i3) const
    {
        return (((int)i1 < dims(0)) && ((int)i2 < dims(1)) && ((int)i3 < dims(2)));
    }
    bool safe_index(int i1, int i2, int i3, int i4, int i5, int i6) const
    {
        return (
            (0 <= i1) && (i1 < dims(0))
            && (0 <= i2) && (i2 < dims(1))
            && (0 <= i3) && (i3 < dims(2))
            && (0 <= i4) && (i4 < dims(3))
            && (0 <= i5) && (i5 < dims(4))
            && (0 <= i6) && (i6 < dims(5)));
    }

    bool read_from_text_file(const QString& path)
    {
        QString txt = TextFile::read(path);
        if (txt.isEmpty()) {
            return false;
        }
        QStringList lines = txt.split("\n", QString::SkipEmptyParts);
        QStringList lines2;
        for (int i = 0; i < lines.count(); i++) {
            QString line = lines[i].trimmed();
            if (!line.isEmpty()) {
                if (i == 0) {
                    //check whether this is a header line, if so, don't include it
                    line = line.split(",", QString::SkipEmptyParts).join(" ");
                    QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    bool ok;
                    vals.value(0).toDouble(&ok);
                    if (ok) {
                        lines2 << line;
                    }
                }
                else {
                    lines2 << line;
                }
            }
        }
        for (int i = 0; i < lines2.count(); i++) {
            QString line = lines2[i].trimmed();
            line = line.split(",", QString::SkipEmptyParts).join(" ");
            QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (i == 0) {
                allocate(0, vals.count(), lines2.count());
            }
            for (int j = 0; j < vals.count(); j++) {
                set(vals[j].toDouble(), j, i);
            }
        }
        return true;
    }
    bool write_to_text_file(const QString& path) const
    {
        char sep = ' ';
        if (path.endsWith(".csv"))
            sep = ',';
        int max_num_entries = 1e6;
        if (N1() * N2() == max_num_entries) {
            qWarning() << "mda is too large to write text file";
            return false;
        }
        QList<QString> lines;
        for (int i = 0; i < N2(); i++) {
            QStringList vals;
            for (int j = 0; j < N1(); j++) {
                vals << QString("%1").arg(at(j, i));
            }
            QString line = vals.join(sep);
            lines << line;
        }
        return TextFile::write(path, lines.join("\n"));
    }

    void incrementBytesAllocatedCounter(int64_t size) const
    {
        if (allocatedCounter)
            allocatedCounter->add(size);
    }
    void incrementBytesFreedCounter(int64_t size) const
    {
        if (freedCounter)
            freedCounter->add(size);
    }
    void incrementBytesReadCounter(int64_t size) const
    {
        if (bytesReadCounter)
            bytesReadCounter->add(size);
    }
    void incrementBytesWrittenCounter(int64_t size) const
    {
        if (bytesWrittenCounter)
            bytesWrittenCounter->add(size);
    }

private:
    pointer m_data;
    std::vector<int> m_dims;
    size_t total_size;
    mutable IIntCounter* allocatedCounter = nullptr;
    mutable IIntCounter* freedCounter = nullptr;
    mutable IIntCounter* bytesReadCounter = nullptr;
    mutable IIntCounter* bytesWrittenCounter = nullptr;
};

#endif // MDA_P_H
