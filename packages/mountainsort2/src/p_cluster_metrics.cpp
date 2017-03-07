#include "p_cluster_metrics.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <diskreadmda32.h>
#include <mda.h>
#include "mlcommon.h"

namespace P_cluster_metrics {
QJsonObject get_cluster_metrics(const DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int k, Cluster_metrics_opts opts);
}

bool p_cluster_metrics(QString timeseries_path, QString firings_path, QString metrics_out_path, Cluster_metrics_opts opts)
{
    QJsonArray clusters;

    DiskReadMda32 X(timeseries_path);
    Mda firings(firings_path);

    QVector<double> times(firings.N2());
    QVector<int> labels(firings.N2());
    for (int i = 0; i < firings.N2(); i++) {
        times[i] = firings.value(1, i);
        labels[i] = firings.value(2, i);
    }

    QSet<int> used_labels_set;
    for (int i = 0; i < labels.count(); i++) {
        used_labels_set.insert(labels[i]);
    }
    QList<int> used_labels = used_labels_set.toList();
    qSort(used_labels);

    foreach (int k, used_labels) {
        QJsonObject tmp = P_cluster_metrics::get_cluster_metrics(X, times, labels, k, opts);
        clusters.push_back(tmp);
    }

    QJsonObject obj;
    obj["clusters"] = clusters;
    QString json = QJsonDocument(obj).toJson(QJsonDocument::Indented);

    return TextFile::write(metrics_out_path, json);
}

namespace P_cluster_metrics {
QJsonObject get_cluster_metrics(const DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int k, Cluster_metrics_opts opts)
{
    (void)X;
    QVector<int> times_k;
    for (int i = 0; i < labels.count(); i++) {
        if (labels[i] == k)
            times_k << times[i];
    }

    qSort(times_k);

    QJsonObject metrics;
    metrics["num_events"] = times_k.count();
    if (!opts.samplerate)
        opts.samplerate = 1;
    double t1_sec = times_k.value(0) / opts.samplerate;
    double t2_sec = times_k.value(times_k.count() - 1) / opts.samplerate;
    double dur_sec = t2_sec - t1_sec;
    double firing_rate = 0;
    if (dur_sec > 0) {
        firing_rate = times_k.count() / dur_sec;
    }
    metrics["t1_sec"] = t1_sec;
    metrics["t2_sec"] = t2_sec;
    metrics["dur_sec"] = dur_sec;
    metrics["firing_rate"] = firing_rate;

    QJsonObject obj;
    obj["label"] = (double)k;
    obj["metrics"] = metrics;

    return obj;
}
}
