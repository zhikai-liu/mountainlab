#ifndef DISKARRAYMODEL_H
#define DISKARRAYMODEL_H

#include <QObject>
#include "mda.h"

//changed all ints to longs!!! 3/10/2016 (hope it didn't cause problems!)

class DiskArrayModelPrivate;
class DiskArrayModel : public QObject {
    Q_OBJECT
public:
    friend class DiskArrayModelPrivate;
    explicit DiskArrayModel(QObject* parent = 0);
    ~DiskArrayModel();
    void setPath(QString path);
    void setFromMda(const Mda& X);
    QString path();
    bool fileHierarchyExists();
    void createFileHierarchyIfNeeded();
    double value(int ch, int t);

    virtual Mda loadData(int scale, int t1, int t2);
    virtual int size(int dim);
    virtual int dim3();

private:
    DiskArrayModelPrivate* d;

signals:

public slots:
};

#endif // DISKARRAYMODEL_H
