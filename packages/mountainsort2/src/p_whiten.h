#ifndef P_WHITEN_H
#define P_WHITEN_H

#include <QString>

struct Whiten_opts {

};

bool p_whiten(QString timeseries,QString timeseries_out,Whiten_opts opts);

#endif // P_WHITEN_H
