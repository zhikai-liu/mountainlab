#ifndef P_MULTINEIGHBORHOOD_SORT_H
#define P_MULTINEIGHBORHOOD_SORT_H

#include <QString>
#include "mlcommon.h"

struct P_multineighborhood_sort_opts {
    double adjacency_radius=0;
    int clip_size=50;

    double detect_threshold=5;
    int detect_interval=10;
    int detect_sign=0;

    int num_features=10;
    int num_features_per_channel=10;
    bigint max_pca_samples=10000;

    bool consolidate_clusters=true;
    double consolidation_factor=0.9;

    bool merge_across_channels=true;

    bool fit_stage=true;
};

bool p_multineighborhood_sort(QString timeseries,QString geom,QString firings_out,QString temp_path,const P_multineighborhood_sort_opts &opts);

#endif // P_MULTINEIGHBORHOOD_SORT_H

