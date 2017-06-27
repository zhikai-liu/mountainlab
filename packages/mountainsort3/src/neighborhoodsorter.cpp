#include "neighborhoodsorter.h"
#include "detect_events.h"
#include "pca.h"
#include "sort_clips.h"

class NeighborhoodSorterPrivate {
public:
    NeighborhoodSorter *q;
    P_mountainsort3_opts m_opts;

    bigint m_M=0;
    bigint m_N=0;
    QVector<double> m_times;
    QVector<Mda32> m_clips_to_append;

    Mda32 m_clips;
    Mda32 m_reduced_clips;
    QVector<int> m_labels;

    static void dimension_reduce_clips(Mda32& ret, const Mda32& clips, bigint num_features_per_channel, bigint max_samples);
};

NeighborhoodSorter::NeighborhoodSorter()
{
    d=new NeighborhoodSorterPrivate;
    d->q=this;
}

NeighborhoodSorter::~NeighborhoodSorter()
{
    delete d;
}

void NeighborhoodSorter::setOptions(P_mountainsort3_opts opts)
{
    d->m_opts=opts;
}

void NeighborhoodSorter::addTimeChunk(const Mda32 &X, bigint padding_left, bigint padding_right)
{
    d->m_M=X.N1();
    int T=d->m_opts.clip_size;
    int Tmid=(int)((T+1)/2)-1;
    QVector<double> X0;
    for (bigint i=0; i<X.N2(); i++) {
        X0 << X.value(0,i);
    }
    QVector<double> times0=detect_events(X0,d->m_opts.detect_threshold,d->m_opts.detect_interval,d->m_opts.detect_sign);
    QVector<double> times0b,times1;
    for (bigint i=0; i<times0.count(); i++) {
        double t0=times0[i];
        if ((0<=t0-padding_left)&&(t0-padding_left<X.N2()-padding_left-padding_right)) {
            times1 << t0;
        }
    }
    for (bigint i=0; i<times1.count(); i++) {
        double t0=times1[i];
        Mda32 clip0;
        X.getChunk(clip0,0,t0-Tmid,X.N1(),T);
        d->m_clips_to_append << clip0;
        d->m_times << t0-padding_left+d->m_N;
    }
    d->m_N=d->m_N+X.N2()-padding_left-padding_right;
}

void NeighborhoodSorter::sort()
{
    int T=d->m_opts.clip_size;

    // append the buffered clips
    if (d->m_clips_to_append.count()>0) {
        bigint L=d->m_clips.N3()+d->m_clips_to_append.count();
        Mda32 clips_new(d->m_M,T,L);
        clips_new.setChunk(d->m_clips,0,0,0);
        for (bigint i=0; i<d->m_clips_to_append.count(); i++) {
            clips_new.setChunk(d->m_clips_to_append[i],0,0,i+d->m_clips.N3());
        }
        d->m_clips=clips_new;
        d->m_clips_to_append.clear();
    }

    // dimension reduce clips
    d->dimension_reduce_clips(d->m_reduced_clips,d->m_clips,d->m_opts.num_features_per_channel,d->m_opts.max_pca_samples);

    Sort_clips_opts ooo;
    ooo.max_samples=d->m_opts.max_pca_samples;
    ooo.num_features=d->m_opts.num_features;
    d->m_labels=sort_clips(d->m_reduced_clips,ooo);
}

QVector<double> NeighborhoodSorter::times() const
{
    return d->m_times;
}

QVector<int> NeighborhoodSorter::labels() const
{
    return d->m_labels;
}


void NeighborhoodSorterPrivate::dimension_reduce_clips(Mda32 &ret, const Mda32 &clips, bigint num_features_per_channel, bigint max_samples)
{
    bigint M = clips.N1();
    bigint T = clips.N2();
    bigint L = clips.N3();
    const float* clips_ptr = clips.constDataPtr();

    qDebug().noquote() << QString("Dimension reduce clips %1x%2x%3").arg(M).arg(T).arg(L);

    ret.allocate(M, num_features_per_channel, L);
    float* retptr = ret.dataPtr();
    for (bigint m = 0; m < M; m++) {
        Mda32 reshaped(T, L);
        float* reshaped_ptr = reshaped.dataPtr();
        bigint aa = 0;
        bigint bb = m;
        for (bigint i = 0; i < L; i++) {
            for (bigint t = 0; t < T; t++) {
                //reshaped.set(clips.get(bb),aa);
                reshaped_ptr[aa] = clips_ptr[bb];
                aa++;
                bb += M;
                //reshaped.setValue(clips.value(m, t, i), t, i);
            }
        }
        Mda32 CC, FF, sigma;
        pca_subsampled(CC, FF, sigma, reshaped, num_features_per_channel, false, max_samples);
        float* FF_ptr = FF.dataPtr();
        aa = 0;
        bb = m;
        for (bigint i = 0; i < L; i++) {
            for (bigint a = 0; a < num_features_per_channel; a++) {
                //ret.setValue(FF.value(a, i), m, a, i);
                //ret.set(FF.get(aa),bb);
                retptr[bb] = FF_ptr[aa];
                aa++;
                bb += M;
            }
        }
    }
}
