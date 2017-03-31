#ifndef P_DETECT_EVENTS_H
#define P_DETECT_EVENTS_H

#include <QString>
#include "mlcommon.h"

struct P_detect_events_opts {
    int central_channel = 0;
    double detect_threshold = 3.5;
    double detect_interval = 10;
    int detect_rms_window = 0;
    int sign = 0;
    double subsample_factor = 1;
};

bool p_detect_events(QString timeseries, QString event_times_out, P_detect_events_opts opts);

#endif // P_DETECT_EVENTS_H
