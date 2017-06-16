#include "neighborhoodsorter.h"
#include "p_multineighborhood_sort.h"
#include "get_sort_indices.h"

#include <QTime>
#include <diskreadmda32.h>
#include <mda.h>
#include <mda32.h>
#include "omp.h"
#include "compute_templates_0.h"
#include "merge_across_channels.h"
#include "fit_stage.h"
#include "reorder_labels.h"

QList<bigint> get_channels_from_geom(const Mda &geom,bigint m,double adjacency_radius);
void extract_timeseries_neighborhood(Mda32 &ret,const Mda32 &X,const QList<bigint> &channels);
void sort_events_by_time(QVector<double> &times,QVector<int> &labels,QVector<int> &central_channels);

bool p_multineighborhood_sort(QString timeseries,QString geom,QString firings_out,const P_multineighborhood_sort_opts &opts) {

    //important so we can parallelize in both time and space
    omp_set_nested(1);

    //Mda32 Y(timeseries);
    //DiskReadMda32 X(Y);
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

    int num_parallel_neighborhoods=qMin((int)M,omp_get_max_threads());
    int num_time_threads=qMin((int)(N/chunk_size),omp_get_max_threads());

    int CCheckval=0,CCheckval2=0;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Detect
        qDebug().noquote() << "******* Detecting events...";
        QTime timer; timer.start();
#pragma omp parallel for num_threads(num_time_threads) //parallel in time
        for (bigint t=0; t<N; t+=chunk_size) {
            Mda32 chunk;
#pragma omp critical(read1)
            {
                X.readChunk(chunk,0,t-chunk_overlap_size,M,chunk_size+2*chunk_overlap_size);
            }
            qDebug().noquote() << QString(" ----- Detect (chunk %1/%2)...").arg(t/chunk_size+1).arg((N+chunk_size-1)/chunk_size);
#pragma omp parallel for
            for (bigint m=1; m<=M; m++) {
                NeighborhoodSorter *S=sorters[m-1];
                NeighborhoodChunk neighborhood_chunk;
                extract_timeseries_neighborhood(neighborhood_chunk.data,chunk,S->channels());
                neighborhood_chunk.t1=t;
                neighborhood_chunk.t2=t+chunk_size-1;
                neighborhood_chunk.t_offset=chunk_overlap_size;

                QVector<double> timepoints0=S->chunkDetect(neighborhood_chunk);
                #pragma omp critical(add_timepoints1)
                {
                    for (bigint i=0; i<timepoints0.count(); i++) {
                        CCheckval+=((bigint)timepoints0[i])%10000;
                        CCheckval2+=((bigint)timepoints0[i]*m)%10001;
                        CCheckval=CCheckval%10000;
                        CCheckval2=CCheckval2%10001;
                    }
                    S->addTimepoints(timepoints0);
                }
            }
        }
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    int checkval=0,checkval2=0;
    for (int m=1; m<=M; m++) {
        NeighborhoodSorter *S=sorters[m-1];
        QVector<double> times0;
        QVector<int> labels0;
        S->getTimesLabels(times0,labels0);
        for (bigint i=0; i<times0.count(); i++) {
            checkval+=((bigint)times0[i])%10000;
            checkval2+=((bigint)times0[i]*m)%10001;
            checkval=checkval%10000;
            checkval2=checkval2%10001;
        }
    }
    qDebug() << "Check values:" << checkval << checkval2 << CCheckval << CCheckval2;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Extract clips
        qDebug().noquote() << "******* Extracting clips...";
        QTime timer; timer.start();
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->initializeExtractClips();
        }
#pragma omp parallel for num_threads(num_time_threads) //parallel in time
        for (bigint t=0; t<N; t+=chunk_size) {
            Mda32 chunk;
#pragma omp critical(read2)
            {
                X.readChunk(chunk,0,t-chunk_overlap_size,M,chunk_size+2*chunk_overlap_size);
            }
            qDebug().noquote() << QString(" ----- Extract clips (chunk %1/%2)...").arg(t/chunk_size+1).arg((N+chunk_size-1)/chunk_size);
#pragma omp parallel for
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
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Reduce clips
        qDebug().noquote() << "******* Dimension reducing clips...";
        QTime timer; timer.start();
#pragma omp parallel for num_threads(num_parallel_neighborhoods)
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->reduceClips();
        }
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Sort clips
        qDebug().noquote() << "******* Sorting clips...";
        QTime timer; timer.start();
#pragma omp parallel for num_threads(num_parallel_neighborhoods)
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->sortClips();
        }
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Compute templates
        qDebug().noquote() << "******* Computing templates...";
        QTime timer; timer.start();
#pragma omp parallel for num_threads(num_parallel_neighborhoods)
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->computeTemplates();
        }
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.consolidate_clusters) {
        // Consolidate clusters
        qDebug().noquote() << "******* Consolidating clusters...";
        QTime timer; timer.start();
#pragma omp parallel for num_threads(num_parallel_neighborhoods)
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            S->consolidateClusters();
        }
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QVector<double> times;
    QVector<int> labels;
    QVector<int> central_channels;
    {
        // Collect events
        qDebug().noquote() << "******* Collecting events...";
        QTime timer; timer.start();
        int label_offset=0;
        for (bigint m=1; m<=M; m++) {
            NeighborhoodSorter *S=sorters[m-1];
            QVector<double> times0;
            QVector<int> labels0;
            S->getTimesLabels(times0,labels0);
            for (bigint i=0; i<times0.count(); i++) {
                if (labels0[i]>0) {
                    times << times0[i];
                    labels << labels0[i]+label_offset;
                    central_channels << m;
                }
            }
            label_offset+=MLCompute::max(labels0);
        }
        sort_events_by_time(times,labels,central_channels);
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mda32 templates;
    {
        // Compute templates
        qDebug().noquote() << "******* Computing templates...";
        QTime timer; timer.start();
        templates=compute_templates_0(X,times,labels,opts.clip_size);
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.merge_across_channels) {
        // Merge across channels
        qDebug().noquote() << "******* Merging across channels...";
        QTime timer; timer.start();
        Merge_across_channels_opts oo;
        oo.clip_size=opts.clip_size;
        merge_across_channels(times,labels,central_channels,templates,oo);
        templates=compute_templates_0(X,times,labels,opts.clip_size);
        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.fit_stage) {
        // Fit stage
        qDebug().noquote() << "******* Fit stage...";
        QTime timer; timer.start();
        QVector<bigint> inds_to_use;
#pragma omp parallel for num_threads(num_time_threads) //parallel in time
        for (bigint t=0; t<N; t+=chunk_size) {
            Mda32 chunk;
#pragma omp critical(read_for_fit_stage)
            {
                X.readChunk(chunk,0,t-chunk_overlap_size,M,chunk_size+2*chunk_overlap_size);
            }
            qDebug().noquote() << QString("Fit stage (chunk %1/%2)...").arg(t/chunk_size+1).arg((N+chunk_size-1)/chunk_size);

            QVector<bigint> local_inds;
            QVector<double> local_times;
            QVector<int> local_labels;

            bigint ii=0;
            while ((ii<times.count())&&(times[ii]<t)) ii++;
            while ((ii<times.count())&&(times[ii]<t+chunk_size)) {
                local_inds << ii;
                local_times << times[ii]-t+chunk_overlap_size;
                local_labels << labels[ii];
                ii++;
            }
            Fit_stage_opts oo;
            oo.clip_size=opts.clip_size;
            QVector<bigint> local_inds_to_use=fit_stage(chunk,local_times,local_labels,templates,oo);
#pragma omp critical(set_inds_to_use1)
            {
                for (bigint a=0; a<local_inds_to_use.count(); a++) {
                    inds_to_use << local_inds[local_inds_to_use[a]];
                }
            }
        }
        qSort(inds_to_use);
        qDebug().noquote() << QString("Fit stage: Using %1 of %2 events").arg(inds_to_use.count()).arg(times.count());

        QVector<double> times2(inds_to_use.count());
        QVector<int> labels2(inds_to_use.count());
        QVector<int> central_channels2(inds_to_use.count());
        for (bigint a=0; a<inds_to_use.count(); a++) {
            times2[a]=times[inds_to_use[a]];
            labels2[a]=labels[inds_to_use[a]];
            central_channels2[a]=central_channels[inds_to_use[a]];
        }
        times=times2;
        labels=labels2;
        central_channels=central_channels2;

        templates=compute_templates_0(X,times,labels,opts.clip_size);

        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Reorder labels
        qDebug().noquote() << "******* Reordering labels...";
        QTime timer; timer.start();
        QMap<int,int> label_map=reorder_labels(templates);

        QVector<double> times2;
        QVector<int> labels2;
        QVector<int> central_channels2;
        for (bigint i=0; i<times.count(); i++) {
            int k0=labels[i];
            int k1=label_map.value(k0,0);
            if (k1>0) {
                times2 << times[i];
                labels2 << k1;
                central_channels2 << central_channels[i];
            }
        }
        times=times2;
        labels=labels2;
        central_channels=central_channels2;

        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        // Creating firings
        qDebug().noquote() << "******* Creating firings...";
        QTime timer; timer.start();

        bigint L=times.count();
        Mda firings(3,L);
        for (bigint i=0; i<L; i++) {
            firings.set(central_channels[i],0,i);
            firings.set(times[i],1,i);
            firings.set(labels[i],2,i);
        }

        firings.write64(firings_out);

        qDebug().noquote() << "Elapsed: " << timer.elapsed()*1.0/1000;
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

void sort_events_by_time(QVector<double> &times,QVector<int> &labels,QVector<int> &central_channels) {
    QList<bigint> inds = get_sort_indices_bigint(times);
    QVector<double> times2(times.count());
    QVector<int> labels2(times.count());
    QVector<int> central_channels2(times.count());
    for (bigint i = 0; i < inds.count(); i++) {
        times2[i]=times[inds[i]];
        labels2[i]=labels[inds[i]];
        central_channels2[i]=central_channels[inds[i]];
    }
    times=times2;
    labels=labels2;
    central_channels=central_channels2;
}
