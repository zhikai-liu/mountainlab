#include "p_sort_clips.h"

#include <QTime>
#include <mda32.h>
#include "pca.h"
#include "isosplit5.h"
#include "mlcommon.h"
#include <QCoreApplication>

namespace P_sort_clips {
QVector<int> sort_clips_subset(const Mda32& clips, const QVector<bigint>& indices, Sort_clips_opts opts);
Mda32 dimension_reduce_clips(Mda32& clips, bigint num_features_per_channel, bigint max_samples);
}

bool p_sort_clips(QString clips_path, QString labels_out, Sort_clips_opts opts)
{
    qDebug().noquote() << "p_sort_clips";
    Mda32 clips(clips_path);
    {
        QTime timer;
        timer.start();
        clips = P_sort_clips::dimension_reduce_clips(clips, opts.num_features, opts.max_samples);
        qDebug().noquote() << QString("Time elapsed for dimension reduction per channel (%1x%2x%3): %4 sec").arg(clips.N1()).arg(clips.N2()).arg(clips.N3()).arg(timer.elapsed() * 1.0 / 1000);
    }
    //bigint M=clips.N1();
    //bigint T=clips.N2();
    bigint L = clips.N3();

    QVector<bigint> indices(L);
    for (bigint i = 0; i < L; i++) {
        indices[i] = i;
    }

    qDebug().noquote() << "Sorting clips...";
    QVector<int> labels = P_sort_clips::sort_clips_subset(clips, indices, opts);

    Mda ret(1, L);
    for (bigint i = 0; i < L; i++) {
        ret.setValue(labels[i], i);
    }

    return ret.write64(labels_out);
}

namespace P_sort_clips {

Mda32 dimension_reduce_clips(Mda32& clips, bigint num_features_per_channel, bigint max_samples)
{
    bigint M = clips.N1();
    bigint T = clips.N2();
    bigint L = clips.N3();
    float* clips_ptr = clips.dataPtr();

    qDebug().noquote() << QString("Dimension reduce clips %1x%2x%3").arg(M).arg(T).arg(L);

    Mda32 ret(M, num_features_per_channel, L);
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
    return ret;
}

QVector<int> sort_clips_subset(const Mda32& clips, const QVector<bigint>& indices, Sort_clips_opts opts)
{
    bigint M = clips.N1();
    bigint T = clips.N2();
    bigint L0 = indices.count();

    qDebug().noquote() << QString("Sorting clips %1x%2x%3").arg(M).arg(T).arg(L0);

    Mda32 FF;
    {
        // do this inside a code block so memory gets released
        Mda32 clips_reshaped(M * T, L0);
        bigint iii = 0;
        for (bigint ii = 0; ii < L0; ii++) {
            for (bigint t = 0; t < T; t++) {
                for (bigint m = 0; m < M; m++) {
                    clips_reshaped.set(clips.value(m, t, indices[ii]), iii);
                    iii++;
                }
            }
        }

        Mda32 CC, sigma;
        QTime timer;
        timer.start();
        pca_subsampled(CC, FF, sigma, clips_reshaped, opts.num_features, false, opts.max_samples); //should we subtract the mean?
        qDebug().noquote() << QString("Time elapsed for pca (%1x%2x%3): %4 sec").arg(M).arg(T).arg(L0).arg(timer.elapsed() * 1.0 / 1000);
    }

    isosplit5_opts i5_opts;
    i5_opts.isocut_threshold = opts.isocut_threshold;
    i5_opts.K_init = opts.K_init;

    QVector<int> labels0(L0);
    {
        QTime timer;
        timer.start();
        isosplit5(labels0.data(), opts.num_features, L0, FF.dataPtr(), i5_opts);
        qDebug().noquote() << QString("Time elapsed for isosplit (%1x%2) - K=%3: %4 sec").arg(FF.N1()).arg(FF.N2()).arg(MLCompute::max(labels0)).arg(timer.elapsed() * 1.0 / 1000);
    }

    bigint K0 = MLCompute::max(labels0);
    if (K0 <= 1) {
        //only 1 cluster
        return labels0;
    }
    else {
        //branch method
        QVector<int> labels_new(L0);
        bigint k_offset = 0;
        for (bigint k = 1; k <= K0; k++) {
            QVector<bigint> indices2, indices2b;
            for (bigint a = 0; a < L0; a++) {
                if (labels0[a] == k) {
                    indices2 << indices[a];
                    indices2b << a;
                }
            }
            if (indices2.count() > 0) {
                QVector<int> labels2 = sort_clips_subset(clips, indices2, opts);
                bigint K2 = MLCompute::max(labels2);
                for (bigint bb = 0; bb < indices2.count(); bb++) {
                    labels_new[indices2b[bb]] = k_offset + labels2[bb];
                }
                k_offset += K2;
            }
        }

        return labels_new;
    }
}
}

bool p_reorder_labels(QString templates_path, QString firings_path, QString firings_out_path)
{
    Mda32 templates(templates_path);
    Mda firings(firings_path);
}
