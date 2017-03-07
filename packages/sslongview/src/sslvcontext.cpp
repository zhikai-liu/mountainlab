#include "sslvcontext.h"

#include <QJsonArray>

class SSLVContextPrivate {
public:
    SSLVContext* q;

    QJsonObject m_original_object; //to preserve unused fields

    DiskReadMda32 m_timeseries;
    DiskReadMda m_firings;

    MVRange m_current_time_range = MVRange(0, 0); //(0,0) triggers automatic calculation
    double m_current_timepoint = 0;

    double m_sample_rate = 0;
    QJsonObject m_cluster_metrics;

    int m_current_cluster = -1;
    QList<int> m_selected_clusters;

    double m_max_timepoint = 0;

    QList<QColor> m_cluster_colors;
    QList<QColor> m_channel_colors;
    QMap<QString, QColor> m_colors;

    double compute_max_timepoint();
};

SSLVContext::SSLVContext()
{
    d = new SSLVContextPrivate;
    d->q = this;

    //this->setOption("spectrogram_time_resolution", 32);

    // default colors
    d->m_colors["background"] = QColor(240, 240, 240);
    d->m_colors["frame1"] = QColor(245, 245, 245);
    d->m_colors["info_text"] = QColor(80, 80, 80);
    d->m_colors["view_background"] = QColor(243, 243, 247);
    d->m_colors["view_background_highlighted"] = QColor(210, 230, 250);
    d->m_colors["view_background_selected"] = QColor(220, 240, 250);
    d->m_colors["view_background_hovered"] = QColor(243, 243, 237);
    d->m_colors["view_frame"] = QColor(200, 190, 190);
    d->m_colors["view_frame_selected"] = QColor(100, 80, 80);
    d->m_colors["divider_line"] = QColor(255, 100, 150);
    d->m_colors["calculation-in-progress"] = QColor(130, 130, 140, 50);
    d->m_colors["cluster_label"] = Qt::darkGray;
    d->m_colors["firing_rate_disk"] = Qt::lightGray;
}

SSLVContext::~SSLVContext()
{
    delete d;
}

void SSLVContext::clear()
{
    clearOptions();
}

void SSLVContext::setSampleRate(double rate)
{
    d->m_sample_rate = rate;
}

double SSLVContext::sampleRate() const
{
    return d->m_sample_rate;
}

void SSLVContext::setFromMV2FileObject(QJsonObject X)
{
    this->clear();
    d->m_original_object = X; // to preserve unused fields
    if (X.contains("timeseries"))
        d->m_timeseries = DiskReadMda32(X["timeseries"].toObject());
    if (X.contains("firings"))
        d->m_firings = DiskReadMda(X["firings"].toObject());

    d->m_sample_rate = X["samplerate"].toDouble();
    if (X.contains("options")) {
        this->setOptions(X["options"].toObject().toVariantMap());
    }

    d->m_cluster_metrics = X["cluster_metrics"].toObject();

    d->m_max_timepoint = d->compute_max_timepoint();

    this->setCurrentTimeRange(MVRange(0, d->m_max_timepoint));

    emit this->timeseriesChanged();
    emit this->currentTimepointChanged();
    emit this->currentTimeRangeChanged();
    emit this->optionChanged("");
}

QJsonObject SSLVContext::toMV2FileObject() const
{
    QJsonObject X = d->m_original_object;
    if (d->m_timeseries.N2() > 1)
        X["timeseries"] = d->m_timeseries.toPrvObject();
    if (d->m_firings.N1() > 1)
        X["firings"] = d->m_firings.toPrvObject();
    X["samplerate"] = d->m_sample_rate;
    X["options"] = QJsonObject::fromVariantMap(this->options());
    X["cluster_metrics"] = d->m_cluster_metrics;
    return X;
}

DiskReadMda32 SSLVContext::timeseries() const
{
    return d->m_timeseries;
}

QJsonObject SSLVContext::clusterMetrics() const
{
    return d->m_cluster_metrics;
}

void SSLVContext::setClusterMetrics(const QJsonObject& obj)
{
    d->m_cluster_metrics = obj;
    d->m_max_timepoint = d->compute_max_timepoint();
    emit this->clusterMetricsChanged();
}

void SSLVContext::setTimeseries(const DiskReadMda32& timeseries)
{
    d->m_timeseries = timeseries;
    d->m_max_timepoint = d->compute_max_timepoint();
    emit this->timeseriesChanged();
}

DiskReadMda SSLVContext::firings() const
{
    return d->m_firings;
}

void SSLVContext::setFirings(const DiskReadMda& X)
{
    d->m_firings = X;
    emit this->firingsChanged();
}

void SSLVContext::setCurrentTimepoint(double tp)
{
    if (tp < 0)
        tp = 0;
    if (tp > d->m_max_timepoint)
        tp = d->m_max_timepoint;
    if (d->m_current_timepoint == tp)
        return;
    d->m_current_timepoint = tp;
    emit currentTimepointChanged();
}

void SSLVContext::setCurrentTimeRange(const MVRange& range_in)
{
    MVRange range = range_in;
    if (range.max > d->m_max_timepoint) {
        range = range + (d->m_max_timepoint - range.max);
    }
    if (range.min < 0) {
        range = range + (0 - range.min);
    }
    if (range.max - range.min < 150) { //don't allow range to be too small
        range.max = range.min + 150;
    }
    if ((range.max > d->m_max_timepoint) && (range.min == 0)) { //second condition important
        //don't allow it to extend too far
        range.max = d->m_max_timepoint;
    }
    if (d->m_current_time_range == range)
        return;
    d->m_current_time_range = range;
    emit currentTimeRangeChanged();
}

int SSLVContext::currentCluster() const
{
    return d->m_current_cluster;
}

QList<int> SSLVContext::selectedClusters() const
{
    return d->m_selected_clusters;
}

void SSLVContext::setCurrentCluster(int k)
{
    if (k == d->m_current_cluster)
        return;
    d->m_current_cluster = k;
    emit this->currentClusterChanged();
}

void SSLVContext::setSelectedClusters(const QList<int>& ks)
{
    QList<int> ks2 = QList<int>::fromSet(ks.toSet()); //remove duplicates and -1
    ks2.removeAll(-1);
    qSort(ks2);
    if (d->m_selected_clusters == ks2)
        return;
    d->m_selected_clusters = ks2;
    if (!d->m_selected_clusters.contains(d->m_current_cluster)) {
        this->setCurrentCluster(-1);
    }
    emit selectedClustersChanged();
}

void SSLVContext::clickCluster(int k, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier) {
        if (k < 0)
            return;
        if (d->m_selected_clusters.contains(k)) {
            QList<int> tmp = d->m_selected_clusters;
            tmp.removeAll(k);
            this->setSelectedClusters(tmp);
        }
        else {
            if (k >= 0) {
                QList<int> tmp = d->m_selected_clusters;
                tmp << k;
                this->setSelectedClusters(tmp);
            }
        }
    }
    else {
        this->setSelectedClusters(QList<int>());
        this->setCurrentCluster(k);
    }
}

bool SSLVContext::clusterIsVisible(int k)
{
    (void)k;
    return true;
}

QList<int> SSLVContext::getAllClusterNumbers() const
{
    QList<int> ret;
    QJsonArray clusters = d->m_cluster_metrics["clusters"].toArray();
    for (int i = 0; i < clusters.count(); i++) {
        QJsonObject cluster = clusters[i].toObject();
        int k = cluster["label"].toInt();
        ret << k;
    }
    qSort(ret);
    return ret;
}

double SSLVContext::currentTimepoint() const
{
    return d->m_current_timepoint;
}

MVRange SSLVContext::currentTimeRange() const
{
    if ((d->m_current_time_range.min <= 0) && (d->m_current_time_range.max <= 0)) {
        return MVRange(0, qMax(0.0, d->m_max_timepoint));
    }
    return d->m_current_time_range;
}

void SSLVContext::setClusterColors(const QList<QColor>& colors)
{
    d->m_cluster_colors = colors;
    emit clusterColorsChanged(colors);
}

void SSLVContext::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

void SSLVContext::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
}

double SSLVContext::maxTimepoint() const
{
    return d->m_max_timepoint;
}

QList<QColor> SSLVContext::channelColors() const
{
    return d->m_channel_colors;
}

QList<QColor> SSLVContext::clusterColors() const
{
    QList<QColor> ret;
    for (int i = 0; i < d->m_cluster_colors.count(); i++) {
        ret << this->clusterColor(i + 1);
    }
    return ret;
}

QColor SSLVContext::clusterColor(int k) const
{
    if (k < 0)
        return Qt::black;
    if (k == 0)
        return Qt::gray;
    if (d->m_cluster_colors.isEmpty())
        return Qt::black;
    int cluster_color_index_shift = this->option("cluster_color_index_shift", 0).toInt();
    k += cluster_color_index_shift;
    while (k < 1)
        k += d->m_cluster_colors.count();
    return d->m_cluster_colors[(k - 1) % d->m_cluster_colors.count()];
}

QColor SSLVContext::channelColor(int m) const
{
    if (m < 0)
        return Qt::black;
    if (d->m_channel_colors.isEmpty())
        return Qt::black;
    return d->m_channel_colors[m % d->m_channel_colors.count()];
}

QColor SSLVContext::color(QString name, QColor default_color) const
{
    return d->m_colors.value(name, default_color);
}

QMap<QString, QColor> SSLVContext::colors() const
{
    return d->m_colors;
}

double SSLVContextPrivate::compute_max_timepoint()
{
    double ret = m_timeseries.N2() - 1;
    QJsonArray clusters = m_cluster_metrics["clusters"].toArray();
    for (int i = 0; i < clusters.count(); i++) {
        QJsonObject cluster = clusters[i].toObject();
        QJsonObject metrics = cluster["metrics"].toObject();
        double t2 = metrics["t2_sec"].toDouble() * m_sample_rate;
        if (t2 > ret)
            ret = t2;
    }
    return ret;
}
