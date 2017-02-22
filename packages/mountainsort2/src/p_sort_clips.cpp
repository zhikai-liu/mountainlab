#include "p_sort_clips.h"

#include <mda32.h>
#include "pca.h"
#include "isosplit5.h"

namespace P_sort_clips {
}

bool p_sort_clips(QString clips_path, QString labels_out, Sort_clips_opts opts)
{
    Mda32 clips(clips_path);

    int M=clips.N1();
    int T=clips.N2();
    long L=clips.N3();

    Mda32 FF;
    {
        // do this inside a code block so memory gets released
        Mda32 clips_reshaped(M * T, L);
        long iii = 0;
        for (long ii = 0; ii < L; ii++) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_reshaped.set(clips.value(m, t, ii), iii);
                    iii++;
                }
            }
        }

        Mda32 CC,sigma;
        pca(CC, FF, sigma, clips_reshaped, opts.num_features, false); //should we subtract the mean?
    }

    isosplit5_opts i5_opts;
    i5_opts.isocut_threshold=opts.isocut_threshold;
    i5_opts.K_init=opts.K_init;

    QVector<int> labels(L);
    isosplit5(labels.data(),opts.num_features,L,FF.dataPtr(),i5_opts);

    Mda ret(1,L);
    for (long i=0; i<L; i++) {
        ret.setValue(labels[i],i);
    }

    return ret.write64(labels_out);
}

namespace P_sort_clips {

}
