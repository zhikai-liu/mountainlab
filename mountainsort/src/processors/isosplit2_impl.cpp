#include "isosplit2_impl.h"
#include "isosplit2.h"
#include "isosplit2_w.h"

#include "mda32.h"

#include <mda.h>

bool isosplit2_impl::isosplit2(QString data_in, QString labels_out, isosplit2_impl_opts opts)
{
    Q_UNUSED(opts)
    Mda32 X(data_in);
    long N = X.N2();
    qDebug() << X.N1() << X.N2();
    QVector<int> labels = isosplit2(X);
    Mda labels0(1, N);
    for (long i = 0; i < N; i++) {
        labels0.set(labels[i], i);
    }
    return labels0.write64(labels_out);
}

bool isosplit2_w_impl::isosplit2_w(QString data_in, QString weights_in, QString labels_out, isosplit2_w_impl_opts opts)
{
    Q_UNUSED(opts)
    Mda32 X(data_in);
    Mda32 W(weights_in);
    long N = X.N2();
    QVector<float> weights;
    for (int long i = 0; i < N; i++) {
        weights << W.value(i);
    }
    QVector<int> labels = isosplit2_w(X, weights);
    Mda labels0(1, N);
    for (long i = 0; i < N; i++) {
        labels0.set(labels[i], i);
    }
    return labels0.write64(labels_out);
}
