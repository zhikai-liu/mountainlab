#ifndef DISKREADMDAOLD_H
#define DISKREADMDAOLD_H

#include "mda.h"
#include <QString>
#include <QObject>

class DiskReadMdaOldPrivate;

//changed ints to longs on 3/10/16

class DiskReadMdaOld : public QObject {
    Q_OBJECT
public:
    friend class DiskReadMdaOldPrivate;
    explicit DiskReadMdaOld(const QString& path = "");
    DiskReadMdaOld(const DiskReadMdaOld& other);
    DiskReadMdaOld(const Mda& X);
    void operator=(const DiskReadMdaOld& other);
    ~DiskReadMdaOld();

    void setPath(const QString& path);

    int N1() const;
    int N2() const;
    int N3() const;
    int N4() const;
    int N5() const;
    int N6() const;
    int totalSize() const;
    int size(int dim) const;
    double value(int i1, int i2);
    double value(int i1, int i2, int i3, int i4 = 0, int i5 = 0, int i6 = 0);
    double value1(int ii);
    void reshape(int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    void write(const QString& path);

private:
    DiskReadMdaOldPrivate* d;
};

#endif // DISKREADMDAOLD_H
