#include "merge_across_channels.h"
#include "get_sort_indices.h"

QList<QVector<bigint> > find_label_inds(const QVector<int>& labels, int K);
bool peaks_are_within_range_to_consider(double p1, double p2, Merge_across_channels_opts opts);
bool peaks_are_within_range_to_consider(double p11, double p12, double p21, double p22, Merge_across_channels_opts opts);
QList<int> reverse_order(const QList<int>& inds);
bool cluster_is_already_being_used(const QVector<double>& times_in, const QVector<double>& other_times_in, Merge_across_channels_opts opts);

void merge_across_channels(QVector<double>& times, QVector<int>& labels, QVector<int>& central_channels, const Mda32& templates, Merge_across_channels_opts opts)
{
    //Important: We assume the times are already sorted!!

    int M = templates.N1();
    int T = templates.N2();
    int K = templates.N3();
    int L = times.count();

    Mda32 channel_peaks(M, K);
    for (int k = 0; k < K; k++) {
        for (int m = 0; m < M; m++) {
            double peak_value = 0;
            for (int t = 0; t < T; t++) {
                double val = templates.value(m, t, k);
                if (qAbs(val) > qAbs(peak_value)) {
                    peak_value = val;
                }
            }
            channel_peaks.setValue(peak_value, m, k);
        }
    }

    //printf("Find candidate pairs to consider...\n");
    //find the candidate pairs for merging
    Mda32 candidate_pairs(K, K);
    QList<QVector<bigint> > all_label_inds = find_label_inds(labels, K);

    QVector<int> central_channels_for_clusters;
    for (int k = 1; k <= K; k++) {
        QVector<bigint>* inds1 = &all_label_inds[k - 1];
        if (!inds1->isEmpty()) {
            int chan = central_channels[(*inds1)[0]]; //the peak channel should be the same for all events with this labels, so we just need to look at the first one
            central_channels_for_clusters << chan;
        }
        else {
            central_channels_for_clusters << 0;
        }
    }

    for (int k1 = 0; k1 < K; k1++) {
        //QVector<int> inds1 = find_label_inds(labels, k1 + 1);
        QVector<bigint>* inds1 = &all_label_inds[k1];
        if (!inds1->isEmpty()) {
            for (int k2 = 0; k2 < K; k2++) {
                QVector<bigint>* inds2 = &all_label_inds[k2];
                if (!inds2->isEmpty()) {
                    int central_chan1 = central_channels_for_clusters[k1];
                    int central_chan2 = central_channels_for_clusters[k2];
                    if (central_chan1 != central_chan2) { //only attempt to merge if the peak channels are different -- that's why it's called "merge_across_channels"
                        double val11 = channel_peaks.value(central_chan1 - 1, k1);
                        double val12 = channel_peaks.value(central_chan2 - 1, k1);
                        double val21 = channel_peaks.value(central_chan1 - 1, k2);
                        double val22 = channel_peaks.value(central_chan2 - 1, k2);
                        if (peaks_are_within_range_to_consider(val11, val12, val21, val22, opts)) {
                            //printf("Within range to consider: m=(%d,%d) k=(%d,%d) %g,%g,%g,%g\n", central_chan1, central_chan2, k1, k2, val11, val12, val21, val22);
                            candidate_pairs.setValue(1, k1, k2);
                        }
                    }
                }
            }
        }
    }

    //sort by largest peak so we can go through in order
    QVector<double> abs_peaks_on_own_channels;
    for (int k = 0; k < K; k++) {
        abs_peaks_on_own_channels << channel_peaks.value(central_channels_for_clusters[k] - 1, k);
    }
    QList<int> inds1 = get_sort_indices(abs_peaks_on_own_channels);
    inds1 = reverse_order(inds1);

    //printf("Testing timings of merge candidates...\n");
    int num_removed = 0;
    QList<bool> clusters_to_use;
    for (int k = 0; k < K; k++)
        clusters_to_use << false;
#pragma omp parallel for
    for (int ii = 0; ii < inds1.count(); ii++) {
        QVector<double> times_k;
        QVector<double> other_times;
        bool to_use;
        int ik;
#pragma omp critical(merge_across_channels1)
        {
            ik = inds1[ii];
            QVector<bigint>* inds_k = &all_label_inds[ik];

            for (int a = 0; a < inds_k->count(); a++) {
                times_k << times[(*inds_k)[a]];
            }

            for (int ik2 = 0; ik2 < K; ik2++) {
                if (candidate_pairs.value(ik, ik2)) {
                    //printf("Merge candidate pair: %d,%d\n", ik + 1, ik2 + 1);
                    if (clusters_to_use[ik2]) { //we are already using the other one
                        QVector<bigint>* inds_k2 = &all_label_inds[ik2];
                        for (int a = 0; a < inds_k2->count(); a++) {
                            other_times << times[(*inds_k2)[a]];
                        }
                    }
                }
            }
        }
        if (cluster_is_already_being_used(times_k, other_times, opts)) {
            to_use = false;
        }
        else {
            to_use = true;
        }
#pragma omp critical(merge_across_channels1)
        {
            if (to_use)
                clusters_to_use[ik] = true;
            else {
                clusters_to_use[ik] = false;
                num_removed++;
            }
        }
    }

    //printf("Eliminating clusters not to use...\n");

    //now we eliminate the clusters not to use
    QVector<int> inds_to_use;
    for (int ii = 0; ii < L; ii++) {
        int ik = labels[ii] - 1;
        if (clusters_to_use[ik]) {
            inds_to_use << ii;
        }
    }

    //printf("Finalizing...\n");

    QVector<double> times2(inds_to_use.count());
    QVector<int> labels2(inds_to_use.count());
    QVector<int> central_channels2(inds_to_use.count());
    for (bigint i = 0; i < inds_to_use.count(); i++) {
        times2[i] = times[inds_to_use[i]];
        labels2[i] = labels[inds_to_use[i]];
        central_channels2[i] = central_channels[inds_to_use[i]];
    }

    qDebug().noquote() << QString("Using %1 of %2 events after %3 redundant clusters removed").arg(times2.count()).arg(times.count()).arg(num_removed);
    times = times2;
    labels = labels2;
    central_channels = central_channels2;
}

QList<QVector<bigint> > find_label_inds(const QVector<int>& labels, int K)
{
    QList<QVector<bigint> > ret;
    for (int k = 0; k < K; k++) {
        ret << QVector<bigint>();
    }
    for (bigint i = 0; i < labels.count(); i++) {
        int k0 = labels[i];
        if (k0 > 0) {
            ret[k0 - 1] << i;
        }
    }
    return ret;
}

bool peaks_are_within_range_to_consider(double p1, double p2, Merge_across_channels_opts opts)
{
    if ((!p1) || (!p2))
        return false;
    double ratio = p1 / p2;
    if (ratio > 1)
        ratio = 1 / ratio;
    if (ratio < 0)
        return false;
    return (ratio > opts.min_peak_ratio_to_consider);
}

bool peaks_are_within_range_to_consider(double p11, double p12, double p21, double p22, Merge_across_channels_opts opts)
{
    return (
        (peaks_are_within_range_to_consider(p11, p12, opts)) && (peaks_are_within_range_to_consider(p21, p22, opts)) && (peaks_are_within_range_to_consider(p11, p21, opts)) && (peaks_are_within_range_to_consider(p12, p22, opts)));
}

QList<int> reverse_order(const QList<int>& inds)
{
    QList<int> ret;
    for (int i = 0; i < inds.count(); i++) {
        ret << inds[inds.count() - 1 - i];
    }
    return ret;
}

bool cluster_is_already_being_used(const QVector<double>& times_in, const QVector<double>& other_times_in, Merge_across_channels_opts opts)
{
    if (times_in.isEmpty())
        return false;
    if (other_times_in.isEmpty())
        return false;
    int T = opts.clip_size;
    QVector<double> times = times_in;
    QVector<double> other_times = other_times_in;
    qSort(times);
    qSort(other_times);
    QList<int> counts; //size = 2*T+1
    for (int a = 0; a < 2 * T + 1; a++) {
        counts << 0;
    }
    int ii_other = 0;
    for (int ii = 0; ii < times.count(); ii++) {
        double t0 = times[ii];
        while ((ii_other + 1 < other_times.count()) && (other_times[ii_other] < t0 - T))
            ii_other++;
        while ((ii_other < other_times.count()) && (other_times[ii_other] <= t0 + T)) {
            int diff = (int)(other_times[ii_other] - t0);
            if ((-T <= diff) && (diff <= T)) {
                counts[diff + T]++;
            }
            ii_other++;
        }
    }
    //look at +/- 3 timepoints
    int max_dt = 3;
    double best_frac = 0;
    int best_t = 0;
    for (int t = max_dt; t + max_dt < 2 * T + 1; t++) {
        int count0 = 0;
        for (int t2 = t - max_dt; t2 <= t + max_dt; t2++) {
            count0 += counts[t2];
        }
        double frac = count0 * 1.0 / times.count();
        if (frac > best_frac) {
            best_frac = frac;
            best_t = t;
        }
    }
    if (best_frac >= opts.event_fraction_threshold) {
        qDebug().noquote() << QString("Cluster is already being used: frac=%1, dt=%2").arg(best_frac).arg(best_t - T);
        //qDebug().noquote() << counts.mid(best_t - max_dt, max_dt * 2 + 1);
        return true;
    }
    return false;
}
