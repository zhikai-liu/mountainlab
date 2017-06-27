#include "p_mountainsort3.h"

#include <diskreadmda32.h>
#include <mda.h>
#include <mda32.h>
#include "omp.h"

namespace MountainSort3 {

struct TimeChunkInfo {
    bigint t_padding; //t1 corresponds to index t_padding (because we have padding/overlap)
    bigint t1; //starting timepoint (corresponding to index t_padding)
    bigint size; //number of timepoints (excluding padding on left and right)
};

struct NeighborhoodData {
    QList<int> channels;
    QVector<double> times;
    QVector<int> labels;
    Mda32 clips;
    Mda32 reduced_clips;
    Mda32 templates;
};
QList<int> get_channels_from_geom(const Mda &geom,bigint m,double adjacency_radius);
QVector<double> extract_channel(const Mda32 &X,int channel);
void extract_channels(Mda32 &ret,const Mda32 &X,const QList<int> &channels);
QVector<double> detect_events(const QVector<double>& X, double detect_threshold, double detect_interval, int sign);

QVector<double> parallel_detect_events(const QVector<double>& X, P_mountainsort3_opts opts, int num_threads);

QList<QList<int>> get_neighborhood_batches(int M,int batch_size) {
    QList<QList<int>> batches;
    for (int m=1; m<=M; m+=batch_size) {
        QList<int> batch;
        for (int m2=m; (m2<m+batch_size)&&(m2<=M); m2++)
            batch << m2;
        batches << batch;
    }
    return batches;
}

}

using namespace MountainSort3;

bool p_mountainsort3(QString timeseries, QString geom, QString firings_out, QString temp_path, const P_mountainsort3_opts &opts)
{
    int max_threads=omp_get_max_threads();

    DiskReadMda32 X(timeseries);
    bigint M=X.N1();
    //bigint N=X.N2();

    Mda Geom(geom);

    QMap<int,NeighborhoodData*> neighborhoods;
    for (bigint m=1; m<=M; m++) {
        neighborhoods[m]=new NeighborhoodData;
        neighborhoods[m]->channels=get_channels_from_geom(geom,m,opts.adjacency_radius);
    }

    int num_simultaneous_neighborhoods=qMin(max_threads,(int)M);
    QList<QList<int>> neighborhood_batches=get_neighborhood_batches(M,num_simultaneous_neighborhoods);


    /*
    //detect
    for (bigint m=1; m<=M; m++) {
        QList<int> single_channel;
        QVector<double> X0=extract_channel(X,neighborhoods[m]->channels.value(0));
        neighborhoods[m]->times=parallel_detect_events(X0,opts,max_threads);
    }
    */

    qDeleteAll(neighborhoods);
    return true;
}

namespace MountainSort3 {
QList<int> get_channels_from_geom(const Mda &geom,bigint m,double adjacency_radius) {
    bigint M=geom.N2();
    QList<int> ret;
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

void extract_channels(Mda32 &ret,const Mda32 &X,const QList<int> &channels) {
    bigint M2=channels.count();
    bigint N=X.N2();
    ret.allocate(M2,N);
    for (bigint t=0; t<N; t++) {
        for (bigint ii=0; ii<M2; ii++) {
            ret.set(X.value(channels[ii]-1,t),ii,t);
        }
    }
}

QVector<double> extract_channel(const Mda32 &X,int channel) {
    QVector<double> ret(X.N2());
    for (bigint i=0; i<X.N2(); i++) {
        ret[i]=X.get(channel-1,i);
    }
    return ret;
}

QVector<double> detect_events(const QVector<double>& X, double detect_threshold, double detect_interval, int sign)
{
    double mean = MLCompute::mean(X);
    double stdev = MLCompute::stdev(X);
    double threshold2 = detect_threshold * stdev;
    //double threshold2=detect_threshold;

    QVector<bigint> timepoints_to_consider;
    QVector<double> vals_to_consider;
    for (bigint n=0; n<X.count(); n++) {
        //double val=X[n];
        double val=X[n]-mean;
        if (sign < 0)
            val = -val;
        else if (sign == 0)
            val = fabs(val);
        if (val>threshold2) {
            timepoints_to_consider << n;
            vals_to_consider << val;
        }
    }
    QVector<bool> to_use(timepoints_to_consider.count(),false);
    bigint last_best_ind=-1;
    for (bigint i=0; i<timepoints_to_consider.count(); i++) {
        if (last_best_ind>=0) {
            if (timepoints_to_consider[last_best_ind]<timepoints_to_consider[i]-detect_interval) {
                last_best_ind=-1;
                //last best ind is not within range. so update it
                if ((i>0)&&(timepoints_to_consider[i-1]>=timepoints_to_consider[i]-detect_interval)) {
                    last_best_ind=i-1;
                    bigint jj=last_best_ind;
                    while ((jj-1>=0)&&(timepoints_to_consider[jj-1]>=timepoints_to_consider[i]-detect_interval)) {
                        if (vals_to_consider[jj-1]>vals_to_consider[last_best_ind]) {
                            last_best_ind=jj-1;
                        }
                        jj--;
                    }
                }
                else {
                    last_best_ind=-1;
                }
            }
        }
        if (last_best_ind>=0) {
            if (vals_to_consider[i]>vals_to_consider[last_best_ind]) {
                to_use[i]=true;
                to_use[last_best_ind]=false;
                last_best_ind=i;
            }
            else {
                to_use[i]=false;
            }
        }
        else {
            to_use[i]=true;
            last_best_ind=i;
        }
    }

    QVector<double> times;
    for (bigint i=0; i<timepoints_to_consider.count(); i++) {
        if (to_use[i]) {
            times << timepoints_to_consider[i];
        }
    }
    return times;
}

QList<TimeChunkInfo> get_time_chunk_infos(bigint M,bigint N,int num_threads) {
    bigint RAM_available_for_chunks_bytes=1e9;
    int num_time_threads=omp_get_max_threads();
    bigint chunk_size=RAM_available_for_chunks_bytes/(M*num_time_threads*4);
    if (chunk_size<1000) chunk_size=1000;
    if (chunk_size>N) chunk_size=N;
    bigint chunk_overlap_size=1000;

    // Prepare the information on the time chunks
    QList<TimeChunkInfo> time_chunk_infos;
    for (bigint t=0; t<N; t+=chunk_size) {
        TimeChunkInfo info;
        info.t1=t;
        info.t_padding=chunk_overlap_size;
        info.size=chunk_size;
        if (t+info.size>N) info.size=N-t;
        time_chunk_infos << info;
    }

    return time_chunk_infos;
}

QVector<double> parallel_detect_events(const QVector<double>& X, P_mountainsort3_opts opts, int num_threads) {
    QVector<double> times;

    bigint N=X.count();
    QList<TimeChunkInfo> time_chunk_infos=get_time_chunk_infos(1,N,num_threads);

    for (int i=0; i<time_chunk_infos.count(); i++) {
        TimeChunkInfo TCI=time_chunk_infos[i];
        QVector<double> X0=X.mid(TCI.t1-TCI.t_padding,TCI.size+2*TCI.t_padding);
        QVector<double> times0=detect_events(X0,opts.detect_threshold,opts.detect_interval,opts.detect_sign);
        QVector<double> times1;
        for (bigint a=0; a<times0.count(); a++) {
            if ((TCI.t_padding<=times0[a])&&(times0[a]-TCI.t_padding<TCI.size)) {
                times1 << times0[a]-TCI.t_padding+TCI.t1;
            }
        }
#pragma omp critical(parallel_detect_events_append_times)
        {
            times.append(times1);
        }
    }
    qSort(times);
    return times;
}

}
