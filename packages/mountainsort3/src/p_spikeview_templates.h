#ifndef P_SPIKEVIEW_TEMPLATES_H
#define P_SPIKEVIEW_TEMPLATES_H

#include <QString>
#include "mlcommon.h"

struct P_spikeview_templates_opts {
    int clip_size=100;
    bigint max_events_per_template=1000;
};

bool p_spikeview_templates(QString timeseries,QString firings,QString templates_out,QString stdevs_out,P_spikeview_templates_opts opts);

#endif // P_SPIKEVIEW_TEMPLATES_H

