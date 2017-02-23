#ifndef P_CLUSTER_METRICS_H
#define P_CLUSTER_METRICS_H

#include <QString>


struct Cluster_metrics_opts {
    double samplerate=0;
};

bool p_cluster_metrics(QString timeseries,QString firings,QString metrics_out,Cluster_metrics_opts opts);

#endif // P_CLUSTER_METRICS_H
