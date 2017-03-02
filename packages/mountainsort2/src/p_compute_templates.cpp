#include "p_compute_templates.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"

bool p_compute_templates(QStringList timeseries_list, QString firings_path, QString templates_out, int clip_size)
{
    DiskReadMda32 X(2,timeseries_list);
    DiskReadMda firings(firings_path);

    int M = X.N1();
    //long N = X.N2();
    int T = clip_size;
    long L = firings.N2();
    QVector<double> times;
    QVector<long> labels;

    if (!T) {
        qWarning() << "Unexpected: Clip size is zero.";
        return false;
    }

    for (long i=0; i<L; i++) {
        times << firings.value(1,i);
        labels << firings.value(2,i);
    }

    long K=MLCompute::max(labels);

    Mda templates(M,T,K);
    QVector<double> counts(K);
    counts.fill(0);

    double *templates_ptr=templates.dataPtr();

    int Tmid = (int)((T + 1) / 2) - 1;
    printf("computing templates (M=%d,T=%d,K=%ld,L=%ld)...\n", M, T, K, L);
    for (long i = 0; i < L; i++) {
        int t1 = times[i] - Tmid;
        //int t2 = t1 + T - 1;
        long k=labels[i];
        if (k>0) {
            Mda32 tmp;
            X.readChunk(tmp, 0, t1, M, T);
            float *tmp_ptr=tmp.dataPtr();
            long offset=M*T*(k-1);
            for (long aa=0; aa<M*T; aa++) {
                templates_ptr[offset+aa]+=tmp_ptr[aa];
            }
            counts[k-1]++;
        }
    }
    long bb=0;
    for (long k=1; k<=K; k++) {
        for (long aa=0; aa<M*T; aa++) {
            if (counts[k-1])
                templates_ptr[bb]/=counts[k-1];
            bb++;
        }
    }

    return templates.write32(templates_out);
}
