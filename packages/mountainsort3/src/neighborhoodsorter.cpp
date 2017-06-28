#include "neighborhoodsorter.h"
#include "detect_events.h"
#include "pca.h"
#include "sort_clips.h"
#include "consolidate_clusters.h"

class NeighborhoodSorterPrivate {
public:
    NeighborhoodSorter* q;
    P_mountainsort3_opts m_opts;

    bigint m_M = 0;
    bigint m_N = 0;
    int m_num_threads = 1;
    QVector<double> m_times;
    QVector<Mda32> m_clips_to_append;
    double m_num_bytes = 0;

    Mda32 m_clips;
    Mda32 m_reduced_clips;
    QVector<int> m_labels;
    Mda32 m_templates;

    static void dimension_reduce_clips(Mda32& ret, const Mda32& clips, bigint num_features_per_channel, bigint max_samples);
    static Mda32 compute_templates_from_clips(const Mda32& clips, const QVector<int>& labels, int num_threads);
    static QVector<double> get_subarray(const QVector<double>& X, const QVector<bigint>& inds);
    static QVector<int> get_subarray(const QVector<int>& X, const QVector<bigint>& inds);
    static Mda32 get_subclips(const Mda32& clips, const QVector<bigint>& inds);
};

NeighborhoodSorter::NeighborhoodSorter()
{
    d = new NeighborhoodSorterPrivate;
    d->q = this;
    d->m_clips.allocate(0, 0, 0); //important!
}

NeighborhoodSorter::~NeighborhoodSorter()
{
    delete d;
}

void NeighborhoodSorter::setNumThreads(int num_threads)
{
    d->m_num_threads = num_threads;
}

void NeighborhoodSorter::setOptions(P_mountainsort3_opts opts)
{
    d->m_opts = opts;
}

void NeighborhoodSorter::addTimeChunk(const Mda32& X, bigint padding_left, bigint padding_right)
{
    d->m_M = X.N1();
    int T = d->m_opts.clip_size;
    int Tmid = (int)((T + 1) / 2) - 1;
    QVector<double> X0;
    for (bigint i = 0; i < X.N2(); i++) {
        X0 << X.value(0, i);
    }

    QVector<double> times0 = detect_events(X0, d->m_opts.detect_threshold, d->m_opts.detect_interval, d->m_opts.detect_sign);
    QVector<double> times1; // only the times that fit within the proper time chunk (excluding padding)
    for (bigint i = 0; i < times0.count(); i++) {
        double t0 = times0[i];
        if ((0 <= t0 - padding_left) && (t0 - padding_left < X.N2() - padding_left - padding_right)) {
            times1 << t0;
        }
    }
    for (bigint i = 0; i < times1.count(); i++) {
        double t0 = times1[i];
        Mda32 clip0;
        X.getChunk(clip0, 0, t0 - Tmid, X.N1(), T);
        d->m_clips_to_append << clip0;
        d->m_times << t0 - padding_left + d->m_N;
    }
    d->m_N += X.N2() - padding_left - padding_right;
}

void NeighborhoodSorter::sort()
{
    int T = d->m_opts.clip_size;

    // append the buffered clips
    if (d->m_clips_to_append.count() > 0) {
        bigint L = d->m_clips.N3() + d->m_clips_to_append.count();
        Mda32 clips_new(d->m_M, T, L);
        clips_new.setChunk(d->m_clips, 0, 0, 0);
        for (bigint i = 0; i < d->m_clips_to_append.count(); i++) {
            clips_new.setChunk(d->m_clips_to_append[i], 0, 0, i + d->m_clips.N3());
        }
        d->m_clips = clips_new;
        d->m_clips_to_append.clear();
    }

    // dimension reduce clips
    d->dimension_reduce_clips(d->m_reduced_clips, d->m_clips, d->m_opts.num_features_per_channel, d->m_opts.max_pca_samples);

    // Sort
    Sort_clips_opts ooo;
    ooo.max_samples = d->m_opts.max_pca_samples;
    ooo.num_features = d->m_opts.num_features;
    d->m_labels = sort_clips(d->m_reduced_clips, ooo);

    // Compute templates
    d->m_templates = d->compute_templates_from_clips(d->m_clips, d->m_labels, d->m_num_threads);

    // Consolidate clusters
    Consolidate_clusters_opts oo;
    QMap<int, int> label_map = consolidate_clusters(d->m_templates, oo);
    QVector<bigint> inds_to_keep;
    for (bigint i = 0; i < d->m_labels.count(); i++) {
        int k0 = d->m_labels[i];
        if ((k0 > 0) && (label_map[k0] > 0))
            inds_to_keep << i;
    }
    qDebug() << QString("Consolidate. Keeping %1 of %2 events").arg(inds_to_keep.count()).arg(d->m_labels.count());
    d->m_times = d->get_subarray(d->m_times, inds_to_keep);
    d->m_labels = d->get_subarray(d->m_labels, inds_to_keep);
    d->m_clips = d->get_subclips(d->m_clips, inds_to_keep);
    d->m_reduced_clips = d->get_subclips(d->m_reduced_clips, inds_to_keep);
    for (bigint i = 0; i < d->m_labels.count(); i++) {
        int k0 = label_map[d->m_labels[i]];
        d->m_labels[i] = k0;
    }

    // Compute templates
    d->m_templates = d->compute_templates_from_clips(d->m_clips, d->m_labels, d->m_num_threads);

    d->m_templates.write32("/home/magland/tmp/templates.mda");
}

QVector<double> NeighborhoodSorter::times() const
{
    return d->m_times;
}

QVector<int> NeighborhoodSorter::labels() const
{
    return d->m_labels;
}

Mda32 NeighborhoodSorter::templates() const
{
    return d->m_templates;
}

bigint NeighborhoodSorter::numTimepoints()
{
    return d->m_N;
}

void NeighborhoodSorterPrivate::dimension_reduce_clips(Mda32& ret, const Mda32& clips, bigint num_features_per_channel, bigint max_samples)
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

Mda32 NeighborhoodSorterPrivate::compute_templates_from_clips(const Mda32& clips, const QVector<int>& labels, int num_threads)
{
    int M = clips.N1();
    int T = clips.N2();
    bigint L = clips.N3();
    int K = MLCompute::max(labels);
    Mda sums(M, T, K);
    QVector<bigint> counts(K, 0);
#pragma omp parallel num_threads(num_threads)
    {
        Mda local_sums(M, T, K);
        QVector<bigint> local_counts(K, 0);
#pragma omp for
        for (bigint i = 0; i < L; i++) {
            int k0 = labels[i];
            if (k0 > 0) {
                Mda32 tmp;
                clips.getChunk(tmp, 0, 0, i, M, T, 1);
                for (int t = 0; t < T; t++) {
                    for (int m = 0; m < M; m++) {
                        local_sums.set(local_sums.get(m, t, k0 - 1) + tmp.get(m, t), m, t, k0 - 1);
                    }
                }
                local_counts[k0 - 1]++;
            }
        }
#pragma omp critical(set_sums_and_counts)
        for (int kk = 1; kk <= K; kk++) {
            counts[kk - 1] += local_counts[kk - 1];
            Mda tmp;
            local_sums.getChunk(tmp, 0, 0, kk - 1, M, T, 1);
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    sums.set(sums.get(m, t, kk - 1) + tmp.get(m, t), m, t, kk - 1);
                }
            }
        }
    }
    Mda32 templates(M, T, K);
    for (int kk = 1; kk <= K; kk++) {
        if (counts[kk - 1]) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    templates.set(sums.get(m, t, kk - 1) / counts[kk - 1], m, t, kk - 1);
                }
            }
        }
    }
    return templates;
}

QVector<double> NeighborhoodSorterPrivate::get_subarray(const QVector<double>& X, const QVector<bigint>& inds)
{
    QVector<double> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

QVector<int> NeighborhoodSorterPrivate::get_subarray(const QVector<int>& X, const QVector<bigint>& inds)
{
    QVector<int> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

Mda32 NeighborhoodSorterPrivate::get_subclips(const Mda32& clips, const QVector<bigint>& inds)
{
    Mda32 ret(clips.N1(), clips.N2(), inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        Mda32 tmp;
        clips.getChunk(tmp, 0, 0, inds[i], clips.N1(), clips.N2(), 1);
        ret.setChunk(tmp, 0, 0, i);
    }
    return ret;
}
