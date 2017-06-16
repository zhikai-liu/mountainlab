#include "neighborhoodsorter.h"
#include "p_multineighborhood_sort.h"

#include <diskreadmda32.h>
#include <mda.h>
#include <mda32.h>
#include "omp.h"

QList<bigint> get_channels_from_geom(const Mda &geom,bigint m,double adjacency_radius);
void extract_timeseries_neighborhood(Mda32 &ret,const Mda32 &X,const QList<bigint> &channels);

bool p_multineighborhood_sort(QString timeseries,QString geom,QString firings_out,const P_multineighborhood_sort_opts &opts) {
    DiskReadMda32 X(timeseries);
    Mda Geom(geom);
    bigint M=X.N1();
    bigint N=X.N2();

    QList<NeighborhoodSorter *> sorters;
    for (int m=1; m<=M; m++) {
        NeighborhoodSorter *S=new NeighborhoodSorter;
        S->setOptions(opts);
        QList<bigint> channels=get_channels_from_geom(Geom,m,opts.adjacency_radius);
        S->setChannels(channels);
        sorters << S;
    }

    bigint chunk_size=30000*20;
    if (chunk_size>N) chunk_size=N;
    bigint chunk_overlap_size=1000;

    {
        // Detect
#pragma omp parallel for
        for (bigint t=0; t<N; t+=chunk_size) {
            qDebug().noquote() << QString("Detect (chunk %1/%2)...").arg(t/chunk_size+1).arg((N+chunk_size-1)/chunk_size);
            Mda32 chunk;
            X.readChunk(chunk,0,t-chunk_overlap_size,M,chunk_size+2*chunk_overlap_size);
            for (bigint m=1; m<=M; m++) {
                NeighborhoodSorter *S=sorters[m-1];
                NeighborhoodChunk neighborhood_chunk;
                extract_timeseries_neighborhood(neighborhood_chunk.data,chunk,S->channels());
                neighborhood_chunk.t1=t;
                neighborhood_chunk.t2=t+chunk_size-1;
                neighborhood_chunk.t_offset=chunk_overlap_size;

                S->chunkDetect(neighborhood_chunk);
            }
        }
    }

    {
        // Extract clips
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->initializeExtractClips();
        }
#pragma omp parallel for
        for (bigint t=0; t<N; t+=chunk_size) {
            qDebug().noquote() << QString("Extract clips (chunk %1/%2)...").arg(t/chunk_size+1).arg((N+chunk_size-1)/chunk_size);
            Mda32 chunk;
            X.readChunk(chunk,0,t-chunk_overlap_size,M,chunk_size+2*chunk_overlap_size);
            for (bigint m=1; m<=M; m++) {
                NeighborhoodSorter *S=sorters[m-1];
                NeighborhoodChunk neighborhood_chunk;
                extract_timeseries_neighborhood(neighborhood_chunk.data,chunk,S->channels());
                neighborhood_chunk.t1=t;
                neighborhood_chunk.t2=t+chunk_size-1;
                neighborhood_chunk.t_offset=chunk_overlap_size;

                S->chunkExtractClips(neighborhood_chunk);
            }
        }
    }

    {
        // Reduce clips
        qDebug().noquote() << "Dimension reducing clips...";
#pragma omp parallel for
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->reduceClips();
        }
    }

    {
        // Sort clips
        qDebug().noquote() << "Sorting clips...";
#pragma omp parallel for
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->sortClips();
        }
    }

    {
        // Compute templates
        qDebug().noquote() << "Computing templates...";
#pragma omp parallel for
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->computeTemplates();
        }
    }

    {
        // Consolidate clusters
        qDebug().noquote() << "Consolidating clusters...";
#pragma omp parallel for
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->consolidateClusters();
            S->templates().write64(QString("/home/magland/tmp/test_templates_%1.mda").arg(m));
        }
    }

    qDeleteAll(sorters);

    return true;
}

QList<bigint> get_channels_from_geom(const Mda &geom,bigint m,double adjacency_radius) {
    bigint M=geom.N2();
    QList<bigint> ret;
    ret << m;
    for (int m2=1; m2<=M; m2++) {
        if (m2!=m) {
            double sumsqr=0;
            for (int i=0; i<geom.N1(); i++) {
                double tmp=geom.value(i,m-1)-geom.value(i,m2-1);
                sumsqr+=tmp*tmp;
            }
            double dist=sqrt(sumsqr);
            if (dist<=adjacency_radius)
                ret << m2;
        }
    }
    return ret;
}

void extract_timeseries_neighborhood(Mda32 &ret,const Mda32 &X,const QList<bigint> &channels) {
    bigint M2=channels.count();
    bigint N=X.N2();
    ret.allocate(M2,N);
    for (bigint t=0; t<N; t++) {
        for (bigint ii=0; ii<M2; ii++) {
            ret.set(X.value(channels[ii]-1,t),ii,t);
        }
    }
}
