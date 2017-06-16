#ifndef P_MULTINEIGHBORHOOD_SORT_H
#define P_MULTINEIGHBORHOOD_SORT_H

#include <QString>
#include "mlcommon.h"

struct P_multineighborhood_sort_opts {
    double adjacency_radius=0;
    double detect_threshold=5;
    int detect_interval=10;
    int detect_sign=0;
    int num_features=10;
    int num_features_per_channel=10;
    int clip_size=50;
    bigint max_pca_samples=10000;
};

bool p_multineighborhood_sort(QString timeseries,QString geom,QString firings_out,const P_multineighborhood_sort_opts &opts);

#endif // P_MULTINEIGHBORHOOD_SORT_H

