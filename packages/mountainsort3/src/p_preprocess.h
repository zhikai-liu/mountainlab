#ifndef P_PREPROCESS_H
#define P_PREPROCESS_H

#include "mlcommon.h"

struct P_preprocess_opts {
    bool bandpass_filter=true;
    double freq_min=300;
    double freq_max=6000;
    bool mask_out_artifacts=false;
    double mask_out_artifacts_threshold=6;
    double mask_out_artifacts_interval=2000;
    bool whiten=true;
};

bool p_preprocess(QString timeseries,QString timeseries_out,QString temp_path,P_preprocess_opts opts);

#endif // P_PREPROCESS_H

