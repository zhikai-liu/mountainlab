#include "diskreadmdaold.h"

#include <QFile>
#include <QFileInfo>
#include <QVector>
#include "mdaio.h"
#include "usagetracking.h"
#include <math.h>
#include <QDebug>

#define CHUNKSIZE 10000

struct DataChunk {
    double* data;
};

class DiskReadMdaOldPrivate {
public:
    DiskReadMdaOld* q;
    QVector<DataChunk> m_chunks;
    FILE* m_file;
    int m_header_size;
    int m_num_bytes_per_entry;
    int m_total_size;
    QString m_path;
    int m_data_type;
    int m_size[MDAIO_MAX_DIMS];
    Mda m_memory_mda;
    bool m_using_memory_mda;

    int get_index(int i1, int i2, int i3, int i4 = 0, int i5 = 0, int i6 = 0);
    int get_index(int i1, int i2);
    double* load_chunk(int i);
    void clear_chunks();
    void load_header();
    void initialize_contructor();
};

DiskReadMdaOld::DiskReadMdaOld(const QString& path)
    : QObject()
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    if (!path.isEmpty())
        setPath(path);
}

DiskReadMdaOld::DiskReadMdaOld(const DiskReadMdaOld& other)
    : QObject()
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    if (other.d->m_using_memory_mda) {
        d->m_using_memory_mda = true;
        d->m_memory_mda = other.d->m_memory_mda;
    }
    else {
        setPath(other.d->m_path);
    }
}

DiskReadMdaOld::DiskReadMdaOld(const Mda& X)
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    d->m_memory_mda = X;
    d->m_using_memory_mda = true;
}

void DiskReadMdaOld::operator=(const DiskReadMdaOld& other)
{
    if (other.d->m_using_memory_mda) {
        d->m_using_memory_mda = true;
        d->m_memory_mda = other.d->m_memory_mda;
        return;
    }
    setPath(other.d->m_path);
}

DiskReadMdaOld::~DiskReadMdaOld()
{
    d->clear_chunks();
    if (d->m_file)
        jfclose(d->m_file);
    delete d;
}

void DiskReadMdaOld::setPath(const QString& path)
{
    if (path == d->m_path)
        return; //added 3/3/2016
    d->clear_chunks();
    if (d->m_file)
        jfclose(d->m_file);
    d->m_file = 0;
    d->m_path = path;
    d->load_header();
    d->m_chunks.resize(ceil(d->m_total_size * 1.0 / CHUNKSIZE));
    for (int i = 0; i < d->m_chunks.count(); i++) {
        d->m_chunks[i].data = 0;
    }
}

int DiskReadMdaOld::N1() const { return size(0); }
int DiskReadMdaOld::N2() const { return size(1); }
int DiskReadMdaOld::N3() const { return size(2); }
int DiskReadMdaOld::N4() const { return size(3); }
int DiskReadMdaOld::N5() const { return size(4); }
int DiskReadMdaOld::N6() const { return size(5); }

int DiskReadMdaOld::totalSize() const
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.totalSize();
    }
    return d->m_total_size;
}

int DiskReadMdaOld::size(int dim) const
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.size(dim);
    }
    if (dim >= MDAIO_MAX_DIMS)
        return 1;

    return d->m_size[dim];
}

double DiskReadMdaOld::value(int i1, int i2)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(i1, i2);
    }
    int ind = d->get_index(i1, i2);
    if (ind < 0)
        return 0;
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X) {
        //qWarning() << "chunk not loaded:" << ind << ind/CHUNKSIZE;
        return 0;
    }
    return X[ind % CHUNKSIZE];
}

double DiskReadMdaOld::value(int i1, int i2, int i3, int i4, int i5, int i6)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(i1, i2, i3, i4);
    }
    int ind = d->get_index(i1, i2, i3, i4, i5, i6);
    if (ind < 0)
        return 0;
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X) {
        //qWarning() << "chunk not loaded:" << ind << ind/CHUNKSIZE;
        return 0;
    }
    return X[ind % CHUNKSIZE];
}

double DiskReadMdaOld::value1(int ind)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(ind);
    }
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X)
        return 0;
    return X[ind % CHUNKSIZE];
}

void DiskReadMdaOld::reshape(int N1, int N2, int N3, int N4, int N5, int N6)
{
    if (d->m_using_memory_mda) {
        printf("Warning: unable to reshape because we are using a memory mda.\n");
    }
    int tot = N1 * N2 * N3 * N4 * N5 * N6;
    if (tot != d->m_total_size) {
        printf("Warning: unable to reshape %d <> %d: %s\n", tot, d->m_total_size, d->m_path.toLatin1().data());
        return;
    }
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        d->m_size[i] = 1;
    d->m_size[0] = N1;
    d->m_size[1] = N2;
    d->m_size[2] = N3;
    d->m_size[3] = N4;
    d->m_size[4] = N5;
    d->m_size[5] = N6;
}

void DiskReadMdaOld::write(const QString& path)
{
    if (d->m_using_memory_mda) {
        d->m_memory_mda.write32(path);
        return;
    }
    if (path == d->m_path)
        return;
    if (path.isEmpty())
        return;
    if (QFileInfo(path).exists()) {
        QFile::remove(path);
    }
    QFile::copy(d->m_path, path);
}

int DiskReadMdaOldPrivate::get_index(int i1, int i2, int i3, int i4, int i5, int i6)
{
    int inds[6];
    inds[0] = i1;
    inds[1] = i2;
    inds[2] = i3;
    inds[3] = i4;
    inds[4] = i5;
    inds[5] = i6;
    int factor = 1;
    int ret = 0;
    for (int j = 0; j < 6; j++) {
        if (inds[j] >= m_size[j])
            return -1;
        ret += factor * inds[j];
        factor *= m_size[j];
    }
    return ret;
}

int DiskReadMdaOldPrivate::get_index(int i1, int i2)
{
    return i1 + m_size[0] * i2;
}

double* DiskReadMdaOldPrivate::load_chunk(int i)
{
    if (i >= m_chunks.count()) {
        //qWarning() << "i>=m_chunks.count()" << this->m_size[0] << this->m_size[1] << this->m_size[2] << this->m_size[3];
        return 0;
    }
    if (!m_chunks[i].data) {
        m_chunks[i].data = (double*)jmalloc(sizeof(double) * CHUNKSIZE);
        for (int j = 0; j < CHUNKSIZE; j++)
            m_chunks[i].data[j] = 0;
        if (m_file) {
            fseek(m_file, m_header_size + m_num_bytes_per_entry * i * CHUNKSIZE, SEEK_SET);
            size_t num;
            if (i * CHUNKSIZE + CHUNKSIZE <= m_total_size)
                num = CHUNKSIZE;
            else {
                num = m_total_size - i * CHUNKSIZE;
            }
            void* data = jmalloc(m_num_bytes_per_entry * num);
            size_t num_read = jfread(data, m_num_bytes_per_entry, num, m_file);
            if (num_read == num) {
                if (m_data_type == MDAIO_TYPE_BYTE) {
                    unsigned char* tmp = (unsigned char*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_FLOAT32) {
                    float* tmp = (float*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_INT16) {
                    int16_t* tmp = (int16_t*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_INT32) {
                    int32_t* tmp = (int32_t*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_UINT16) {
                    quint32* tmp = (quint32*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_FLOAT64) {
                    double* tmp = (double*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else {
                    printf("Warning: unexpected data type: %d\n", m_data_type);
                }
                jfree(data);
            }
            else {
                qWarning() << "Problem reading from mda 212" << num_read << num << m_path << i << m_num_bytes_per_entry << m_size[0] << m_size[1] << m_size[2];
            }
        }
        else
            qWarning() << "File is not open!";
    }
    return m_chunks[i].data;
}

void DiskReadMdaOldPrivate::clear_chunks()
{
    for (int i = 0; i < m_chunks.count(); i++) {
        if (m_chunks[i].data)
            jfree(m_chunks[i].data);
    }
    m_chunks.clear();
}

void DiskReadMdaOldPrivate::load_header()
{
    if (m_file) {
        printf("Unexpected problem in load_header\n");
        return;
    }
    if (m_path.isEmpty()) {
        return;
    }

    if (m_path.contains("/spikespy-server?")) {
    }

    m_file = jfopen(m_path.toLatin1().data(), "rb");
    if (!m_file) {
        printf("Unable to open file %s\n", m_path.toLatin1().data());
        return;
    }
    MDAIO_HEADER HH;
    if (!mda_read_header(&HH, m_file)) {
        qWarning() << "Problem in mda_read_header" << m_path;
    }
    m_data_type = HH.data_type;
    m_num_bytes_per_entry = HH.num_bytes_per_entry;
    m_total_size = 1;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++) {
        m_size[i] = HH.dims[i];
        m_total_size *= m_size[i];
    }
    m_header_size = HH.header_size;
}

void DiskReadMdaOldPrivate::initialize_contructor()
{
    m_file = 0;
    m_header_size = 0;
    m_num_bytes_per_entry = 0;
    m_total_size = 0;
    m_data_type = MDAIO_TYPE_FLOAT32;
    m_using_memory_mda = false;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        m_size[i] = 1;
}
