#ifndef CONSOLIDATE_CLUSTERS_H
#define CONSOLIDATE_CLUSTERS_H

#include <QVector>
#include "mda.h"

struct Consolidate_clusters_opts {
    double consolidation_factor = 0.9;
};

void consolidate_clusters(QVector<bigint> &event_inds,QVector<double> &timepoints,QVector<int> &labels,const Mda &templates,Consolidate_clusters_opts opts);

#endif // CONSOLIDATE_CLUSTERS_H

