#include "p_consolidate_clusters.h"
#include <QFile>
#include <diskreadmda32.h>
#include <mda.h>
#include <mda32.h>
#include "mlcommon.h"

namespace P_consolidate_clusters {
    QVector<int> read_labels(QString path);
    bool write_labels(QString path,const QVector<int> &labels);
    Mda32 compute_template(QString clips_path,const QVector<int> &labels,int k);
    bool should_use_template(const Mda32 &template0,Consolidate_clusters_opts opts);
}

bool p_consolidate_clusters(QString clips, QString labels_path, QString labels_out, Consolidate_clusters_opts opts)
{
    QVector<int> labels=P_consolidate_clusters::read_labels(labels_path);
    long L=labels.count();

    int K=MLCompute::max(labels);

    QVector<int> to_use(K+1);
    to_use.fill(0);

    for (int k=1; k<=K; k++) {
        Mda32 template0=P_consolidate_clusters::compute_template(clips,labels,k);
        if (P_consolidate_clusters::should_use_template(template0,opts)) {
            to_use[k]=1;
        }
    }

    QMap<int,int> label_map;
    label_map[0]=0;
    int knext=1;
    for (int k=1; k<=K; k++) {
        if (to_use[k]) {
            label_map[k]=knext;
            knext++;
        }
        else {
            label_map[k]=0;
        }
    };

    QVector<int> new_labels(L);
    for (long i=0; i<L; i++) {
        new_labels[i]=label_map.value(labels[i]);
    }

    return P_consolidate_clusters::write_labels(labels_out,new_labels);
}

namespace P_consolidate_clusters {
    QVector<int> read_labels(QString path) {
        Mda X(path);
        long L=X.totalSize();
        QVector<int> ret(L);
        for (long i=0; i<L; i++)
            ret[i]=X.value(i);
        return ret;
    }
    bool write_labels(QString path,const QVector<int> &labels) {
        Mda X(1,labels.count());
        for (long i=0; i<labels.count(); i++) {
            X.setValue(labels[i],i);
        }
        return X.write64(path);
    }
    Mda32 compute_template(QString clips_path,const QVector<int> &labels,int k) {
        DiskReadMda32 clips(clips_path);
        int M=clips.N1();
        int T=clips.N2();
        Mda sum(M,T);
        double ct=0;
        for (long i=0; i<labels.count(); i++) {
            if (labels[i]==k) {
                Mda32 tmp;
                clips.readChunk(tmp,0,0,i,M,T,1);
                for (long j=0; j<M*T; j++) {
                    sum.set(sum.get(j)+tmp.get(j),j);
                }
                ct++;
            }
        }
        Mda32 ret(M,T);
        if (ct) {
            for (long j=0; j<M*T; j++) {
                ret.set(sum.get(j)/ct,j);
            }
        }
        return ret;
    }
    bool should_use_template(const Mda32 &template0,Consolidate_clusters_opts opts) {
        int peak_location_tolerance=10;

        int M=template0.N1();
        int T=template0.N2();
        long Tmid = (int)((T + 1) / 2) - 1;

        QVector<double> energies(M);
        energies.fill(0);
        double max_energy = 0;
        for (long t = 0; t < T; t++) {
            for (long m = 0; m < M; m++) {
                double val = template0.value(m, t);
                energies[m] += val * val;
                if ((m != opts.central_channel-1) && (energies[m] > max_energy))
                    max_energy = energies[m];
            }
        }
        if (energies[opts.central_channel-1] < max_energy * opts.consolidation_factor)
            return false;

        double abs_peak_val = 0;
        long abs_peak_ind = 0;
        for (long t = 0; t < T; t++) {
            double value = template0.value(opts.central_channel-1, t);
            if (fabs(value) > abs_peak_val) {
                abs_peak_val = fabs(value);
                abs_peak_ind = t;
            }
        }

        if (fabs(abs_peak_ind - Tmid) > peak_location_tolerance) {
            return false;
        }
        return true;
    }
}
