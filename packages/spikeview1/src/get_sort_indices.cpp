#include "get_sort_indices.h"

#include <QVector>

MLVector<bigint> get_sort_indices(const MLVector<double>& X)
{
    MLVector<bigint> result;
    result.reserve(X.size());
    for (bigint i = 0; i < X.size(); ++i)
        result << i;
    std::stable_sort(result.begin(), result.end(),
        [&X](bigint i1, bigint i2) { return X[i1] < X[i2]; });
    return result;
}
