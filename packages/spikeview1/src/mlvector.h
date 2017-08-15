#ifndef MLVECTOR_H
#define MLVECTOR_H

#include "mlcommon.h"

class MLVectorPrivate;
class MLVector {
public:
    friend class MLVectorPrivate;
    MLVector(bigint N=0);
    MLVector(const MLVector &other);
    virtual ~MLVector();
    void operator=(const MLVector &other);
    void resize(bigint N);
    bigint count() const;
    double &operator[](bigint index);
    const double &operator[](bigint index) const;
    double value(bigint index);
    void clear();
    QVector<double> toQVector() const;
private:
    MLVectorPrivate *d;

};

#endif // MLVECTOR_H

