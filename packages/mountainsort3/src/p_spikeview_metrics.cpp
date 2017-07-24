#include "p_spikeview_metrics.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <mda.h>

struct ClusterInfo {
    bigint num_events=0;
};

bool p_spikeview_metrics1(QString firings, QString metrics_out)
{
    Mda FF(firings);
    bigint L=FF.N2();
    QVector<double> times(L);
    QVector<int> labels(L);

    for (bigint i=0; i<L; i++) {
        times[i]=FF.value(1,i);
        labels[i]=FF.value(2,i);
    }

    QMap<int,ClusterInfo> infos;
    for (bigint i=0; i<L; i++) {
        int k=labels[i];
        infos[k].num_events++;
    }

    QList<int> cluster_numbers=infos.keys();

    QJsonArray clusters;
    foreach (int k, cluster_numbers) {
        QJsonObject tmp;
        tmp["label"] = k;
        QJsonObject metrics;
        metrics["sv.num_events"] = (long long)infos[k].num_events;
        tmp["metrics"] = metrics;
        clusters.push_back(tmp);
    }

    {
        QJsonObject obj;
        obj["clusters"] = clusters;
        QString json = QJsonDocument(obj).toJson(QJsonDocument::Indented);
        if (!TextFile::write(metrics_out, json))
            return false;
    }

    return true;
}
