#ifndef NEIGHBORHOODSORTER_H
#define NEIGHBORHOODSORTER_H

#include "p_multineighborhood_sort.h"
#include "diskreadmda32.h"
#include <mda.h>
#include <mda32.h>

struct NeighborhoodChunk {
    Mda32 data;
    bigint t1,t2; //start and end timepoints for chunk
    bigint t_offset; //offset in data
};

class NeighborhoodSorterPrivate;
class NeighborhoodSorter {
public:
    friend class NeighborhoodSorterPrivate;
    NeighborhoodSorter();
    virtual ~NeighborhoodSorter();
    void setChannels(const QList<bigint> &channels);
    QList<bigint> channels() const;
    void setOptions(const P_multineighborhood_sort_opts &opts);

    void chunkDetect(const NeighborhoodChunk &chunk);

    void initializeExtractClips();
    void chunkExtractClips(const NeighborhoodChunk &chunk);
    void reduceClips();

    void sortClips();

    void computeTemplates();
    Mda templates() const;

    void consolidateClusters();
private:
    NeighborhoodSorterPrivate *d;
};

#endif // NEIGHBORHOODSORTER_H

