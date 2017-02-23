#ifndef P_FIT_STAGE_H
#define P_FIT_STAGE_H

#include <QString>

struct Fit_stage_opts {
    int clip_size = 50;
};

bool p_fit_stage(QString timeseries, QString firings, QString firings_out, Fit_stage_opts opts);

#endif // P_FIT_STAGE_H
