#ifndef P_EXTRACT_TIME_INTERVAL_H
#define P_EXTRACT_TIME_INTERVAL_H

#include <QString>
#include "mlcommon.h"

bool p_extract_time_interval(QStringList timeseries_list, QString firings, QString timeseries_out, QString firings_out, bigint t1, bigint t2);

#endif // P_EXTRACT_TIME_INTERVAL_H
