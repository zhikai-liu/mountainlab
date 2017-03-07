#include "p_sort_clips.h"

#include <mda32.h>
#include "pca.h"
#include "isosplit5.h"
#include "mlcommon.h"

namespace P_sort_clips {
QVector<int> sort_clips_subset(const Mda32& clips, const QVector<int>& indices, Sort_clips_opts opts);
}

bool p_sort_clips(QString clips_path, QString labels_out, Sort_clips_opts opts)
{
    Mda32 clips(clips_path);
    //int M=clips.N1();
    //int T=clips.N2();
    int L = clips.N3();

    QVector<int> indices(L);
    for (int i = 0; i < L; i++) {
        indices[i] = i;
    }

    QVector<int> labels = P_sort_clips::sort_clips_subset(clips, indices, opts);

    Mda ret(1, L);
    for (int i = 0; i < L; i++) {
        ret.setValue(labels[i], i);
    }

    return ret.write64(labels_out);
}

namespace P_sort_clips {

QVector<int> sort_clips_subset(const Mda32& clips, const QVector<int>& indices, Sort_clips_opts opts)
{
    int M = clips.N1();
    int T = clips.N2();
    int L0 = indices.count();

    Mda32 FF;
    {
        // do this inside a code block so memory gets released
        Mda32 clips_reshaped(M * T, L0);
        int iii = 0;
        for (int ii = 0; ii < L0; ii++) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_reshaped.set(clips.value(m, t, indices[ii]), iii);
                    iii++;
                }
            }
        }

        Mda32 CC, sigma;
        pca(CC, FF, sigma, clips_reshaped, opts.num_features, false); //should we subtract the mean?
    }

    isosplit5_opts i5_opts;
    i5_opts.isocut_threshold = opts.isocut_threshold;
    i5_opts.K_init = opts.K_init;

    QVector<int> labels0(L0);
    isosplit5(labels0.data(), opts.num_features, L0, FF.dataPtr(), i5_opts);

    int K0 = MLCompute::max(labels0);
    if (K0 <= 1) {
        //only 1 cluster
        return labels0;
    }
    else {
        //branch method
        QVector<int> labels_new(L0);
        int k_offset = 0;
        for (int k = 1; k <= K0; k++) {
            QVector<int> indices2, indices2b;
            for (int a = 0; a < L0; a++) {
                if (labels0[a] == k) {
                    indices2 << indices[a];
                    indices2b << a;
                }
            }
            if (indices2.count() > 0) {
                QVector<int> labels2 = sort_clips_subset(clips, indices2, opts);
                int K2 = MLCompute::max(labels2);
                for (int bb = 0; bb < indices2.count(); bb++) {
                    labels_new[indices2b[bb]] = k_offset + labels2[bb];
                }
                k_offset += K2;
            }
        }

        return labels_new;
    }
}
}
