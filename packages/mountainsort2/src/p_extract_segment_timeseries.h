/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/23/2017
*******************************************************/
#ifndef P_EXTRACT_SEGMENT_TIMESERIES_H
#define P_EXTRACT_SEGMENT_TIMESERIES_H

#include <QString>
#include "mlcommon.h"

bool p_extract_segment_timeseries(QString timeseries, QString timeseries_out, bigint t1, bigint t2,const QList<int> &channels);
bool p_extract_segment_timeseries_from_concat_list(QStringList timeseries_list, QString timeseries_out, bigint t1, bigint t2,const QList<int> &channels);
bool p_extract_segment_firings(QString firings, QString firings_out, bigint t1, bigint t2);

#endif // P_EXTRACT_SEGMENT_TIMESERIES_H
