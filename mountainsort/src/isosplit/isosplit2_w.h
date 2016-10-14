#ifndef ISOSPLIT2_W_H
#define ISOSPLIT2_W_H

#include "mda32.h"
#include <QVector>

QVector<int> isosplit2_w(Mda32& X, const QVector<float>& weights, float isocut_threshold = 1.5, int K_init = 30, bool verbose = false);

struct AttemptedComparisons_w {
    QVector<double> centers1, centers2;
    QVector<double> counts1, counts2;
};

#endif // ISOSPLIT2_W_H
