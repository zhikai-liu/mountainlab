#include "mlvector.h"

#include <mda.h>

class MLVectorPrivate {
public:
    MLVector *q;
    Mda m_data;
};

MLVector::MLVector(bigint size)
{
    d=new MLVectorPrivate;
    d->q=this;
    this->resize(size);
}

MLVector::MLVector(const MLVector &other)
{
    d->m_data=other.d->m_data;
}

MLVector::~MLVector()
{
    delete d;
}

void MLVector::operator=(const MLVector &other)
{
    d->m_data=other.d->m_data;
}

void MLVector::resize(bigint N)
{
    d->m_data.allocate(N,1);
}

bigint MLVector::count() const
{
    return d->m_data.totalSize();
}

double &MLVector::operator[](bigint index)
{
    return d->m_data.dataPtr()[index];
}

const double &MLVector::operator[](bigint index) const
{
    return d->m_data.constDataPtr()[index];
}

double MLVector::value(bigint index)
{
    return d->m_data.value(index);
}

void MLVector::clear()
{
    this->resize(0);
}

QVector<double> MLVector::toQVector() const
{
    QVector<double> ret(d->m_data.totalSize());
    for (bigint i=0; i<d->m_data.totalSize(); i++) {
        ret[i]=d->m_data.get(i);
    }
    return ret;
}
