#ifndef P_SORT_CLIPS_H
#define P_SORT_CLIPS_H

#include <QString>
#include "mlcommon.h"

struct Sort_clips_opts {
    int num_features = 10;
    double isocut_threshold = 1;
    double K_init = 200;
    bigint max_samples = 10000; //for subsampled pca
};

bool p_sort_clips(QString clips, QString labels_out, Sort_clips_opts opts);

#endif // P_SORT_CLIPS_H
