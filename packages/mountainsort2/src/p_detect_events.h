#ifndef P_DETECT_EVENTS_H
#define P_DETECT_EVENTS_H

#include <QString>

bool p_detect_events(QString timeseries,QString event_times_out,int central_channel,double detect_threshold,double detect_interval,int sign);

#endif // P_DETECT_EVENTS_H
