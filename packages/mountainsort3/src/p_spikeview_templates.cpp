#include "p_spikeview_templates.h"

#include <diskreadmda32.h>
#include <mda.h>

namespace P_spikeview_templates {
struct ClusterData {
    QVector<double> times;
    Mda32 template0;
    Mda32 template_stdev;
};

QVector<double> subsample(const QVector<double> &X,bigint max_elments);
void compute_template_stdev(Mda32 &template0,Mda32 &template_stdev,const DiskReadMda32 &X, const QVector<double> &times, int clip_size);

}

using namespace P_spikeview_templates;

bool p_spikeview_templates(QString timeseries, QString firings, QString templates_out, QString stdevs_out, P_spikeview_templates_opts opts)
{
    Mda FF(firings);
    bigint L=FF.N2();
    QVector<double> times(L);
    QVector<int> labels(L);

    qDebug().noquote() << "Reading firings file...";
    for (bigint i=0; i<L; i++) {
        times[i]=FF.value(1,i);
        labels[i]=FF.value(2,i);
    }

    QMap<int,ClusterData> CD;
    for (bigint i=0; i<L; i++) {
        int k=labels[i];
        CD[k].times << times[i];
    }

    qDebug().noquote() << "Subsampling as needed" << opts.max_events_per_template;
    QList<int> keys=CD.keys();
    foreach (int key,keys) {
        if ((opts.max_events_per_template>0)&&(CD[key].times.count()>opts.max_events_per_template)) {
            CD[key].times=subsample(CD[key].times,opts.max_events_per_template);
        }
    }

    DiskReadMda32 X(timeseries);
    int M=X.N1();
    //bigint N=X.N2();
    int T=opts.clip_size;
    foreach (int key,keys) {
        qDebug().noquote() << QString("Computing template for cluster %1/%2").arg(key).arg(MLCompute::max(keys.toVector()));
        compute_template_stdev(CD[key].template0,CD[key].template_stdev,X,CD[key].times,T);
    }

    qDebug().noquote() << "Assembling templates";
    int K=MLCompute::max(labels);
    Mda32 templates(M,T,K);
    Mda32 stdevs(M,T,K);
    for (int k=1; k<=K; k++) {
        if (CD.contains(k)) {
            templates.setChunk(CD[k].template0,0,0,k-1);
            stdevs.setChunk(CD[k].template_stdev,0,0,k-1);
        }
    }

    qDebug().noquote() << "Writing output";
    templates.write32(templates_out);

    if (!stdevs_out.isEmpty()) {
        stdevs.write32(stdevs_out);
    }

    return true;
}

namespace P_spikeview_templates {
QVector<double> subsample(const QVector<double> &X,bigint max_elements) {
    bigint increment=X.count()/max_elements;
    if (increment<1) increment=1;
    QVector<double> Y;
    for (bigint j=0; (j<max_elements)&&(j*increment<X.count()); j++) {
        Y << X[j*increment];
    }
    return Y;
}

void compute_template_stdev(Mda32 &template0,Mda32 &template_stdev,const DiskReadMda32 &X, const QVector<double> &times, int clip_size) {
    int M=X.N1();
    int T=clip_size;
    Mda sum(M,T);
    Mda sumsqr(M,T);

    int Tmid = (int)((T + 1) / 2) - 1;
    for (bigint i = 0; i < times.count(); i++) {
        bigint t1 = times[i] - Tmid;
        //bigint t2 = t1 + T - 1;
        Mda32 tmp;
        X.readChunk(tmp, 0, t1, M, T);
        for (bigint j=0; j<M*T; j++) {
            sum.set(sum.get(j)+tmp.get(j),j);
            sumsqr.set(sumsqr.get(j)+tmp.get(j)*tmp.get(j),j);
        }
    }
    template0.allocate(M,T);
    template_stdev.allocate(M,T);
    if (times.count()>0) {
        for (bigint j=0; j<M*T; j++) {
            double sum0 = sum.get(j);
            double sumsqr0 = sumsqr.get(j);
            template0.set(sum0 / times.count(), j);
            template_stdev.set(sqrt(sumsqr0 / times.count() - (sum0 * sum0) / (times.count() * times.count())), j);
        }
    }
}
}
