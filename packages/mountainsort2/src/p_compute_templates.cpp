#include "p_compute_templates.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"

bool p_compute_templates(QStringList timeseries_list, QString firings_path, QString templates_out, int clip_size, const QList<int> &clusters)
{
    DiskReadMda32 X(2, timeseries_list);
    DiskReadMda firings(firings_path);

    int M = X.N1();
    //int N = X.N2();
    int T = clip_size;
    QVector<double> times;
    QVector<int> labels;

    if (!T) {
        qWarning() << "Unexpected: Clip size is zero.";
        return false;
    }

    if (clusters.count()==0) {
        qWarning() << "Unexpected: Clusters is empty.";
        return false;
    }

    int Kmax=MLCompute::max(clusters.toVector());
    QVector<int> label_map(Kmax+1);
    label_map.fill(-1);
    for (int c=0; c<clusters.count(); c++) {
        label_map[clusters[c]]=c;
    }

    QSet<int> clusters_set=clusters.toSet();

    for (int i = 0; i < firings.N2(); i++) {
        int label0=firings.value(2, i);
        if (clusters_set.contains(label0)) {
            times << firings.value(1, i);
            labels << label0;
        }
    }

    int K0 = clusters.count();

    Mda templates(M, T, K0);
    QVector<double> counts(K0);
    counts.fill(0);

    double* templates_ptr = templates.dataPtr();

    int Tmid = (int)((T + 1) / 2) - 1;
    printf("computing templates (M=%d,T=%d,K=%d,L=%d)...\n", M, T, K0, times.count());
    for (int i = 0; i < times.count(); i++) {
        int t1 = times[i] - Tmid;
        //int t2 = t1 + T - 1;
        int k = label_map[labels[i]];
        {
            Mda32 tmp;
            X.readChunk(tmp, 0, t1, M, T);
            float* tmp_ptr = tmp.dataPtr();
            int offset = M * T * k;
            for (int aa = 0; aa < M * T; aa++) {
                templates_ptr[offset + aa] += tmp_ptr[aa];
            }
            counts[k]++;
        }
    }
    int bb = 0;
    for (int k = 0; k < K0; k++) {
        for (int aa = 0; aa < M * T; aa++) {
            if (counts[k])
                templates_ptr[bb] /= counts[k];
            bb++;
        }
    }

    return templates.write32(templates_out);
}
