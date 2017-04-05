#ifndef P_GENERATE_BACKGROUND_DATASET_H
#define P_GENERATE_BACKGROUND_DATASET_H

#include "mlcommon.h"

struct P_generate_background_dataset_opts {
    bigint min_segment_size = 600;
    bigint max_segment_size = 6000;
    bigint segment_buffer = 100;
    bigint blend_overlap_size = 100;
    double signal_scale_factor = 100;
};

bool p_generate_background_dataset(QString timeseries, QString event_times, QString timeseries_out, P_generate_background_dataset_opts opts);

#endif // P_GENERATE_BACKGROUND_DATASET_H
