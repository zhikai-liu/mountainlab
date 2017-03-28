#include "mccontext.h"

#include <QMutex>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "mlcommon.h"

class MergeFiringsCalculator {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings1;
    DiskReadMda firings2;

    //output
    DiskReadMda firings_merged;
    DiskReadMda confusion_matrix;
    DiskReadMda optimal_label_map;

    virtual void compute();
};

class MCContextPrivate {
public:
    MCContext* q;
    MVContext *m_mv_context1;
    MVContext *m_mv_context2;


    /////////////////////////////////////
    DiskReadMda m_firings1;
    DiskReadMda m_firings2;
    int m_current_cluster2 = -1;
    QSet<int> m_selected_clusters2;

    QMutex m_mutex;
    bool m_merged_firings_computed = false;
    DiskReadMda m_confusion_matrix;
    DiskReadMda m_optimal_label_map;
    DiskReadMda m_firings_merged;

    MergeFiringsCalculator m_calculator;

    void setup_mv_contexts();
};

MCContext::MCContext()
{
    d = new MCContextPrivate;
    d->q = this;
    d->m_mv_context1=new MVContext;
    d->m_mv_context2=new MVContext;

    d->setup_mv_contexts();
}

MCContext::~MCContext()
{
    delete d->m_mv_context1;
    delete d->m_mv_context2;
    delete d;
}

MVContext *MCContext::mvContext1()
{
    return d->m_mv_context1;
}

MVContext *MCContext::mvContext2()
{
    return d->m_mv_context2;
}

void MCContext::setFromMV2FileObject(QJsonObject obj)
{
    //finish
}

QJsonObject MCContext::toMV2FileObject() const
{
    //finish
}

MCEvent MCContext::currentEvent() const
{
    MCEvent ret;
    if (d->m_mv_context1->currentEvent().label>=0) {
        ret.firings_num=1;
        ret.time=d->m_mv_context1->currentEvent().time;
        ret.label=d->m_mv_context1->currentEvent().label;
    }
    else {
        ret.firings_num=2;
        ret.time=d->m_mv_context2->currentEvent().time;
        ret.label=d->m_mv_context2->currentEvent().label;
    }
    return ret;
}

MCCluster MCContext::currentCluster() const
{
    MCCluster ret;
    if (d->m_mv_context2->currentEvent().label>=0) {
        ret.firings_num=1;
        ret.num=d->m_mv_context1->currentCluster();
    }
    else {
        ret.firings_num=2;
        ret.num=d->m_mv_context2->currentCluster();
    }
    return ret;
}

double MCContext::currentTimepoint() const
{
    return d->m_mv_context1->currentTimepoint();
}

MVRange MCContext::currentTimeRange() const
{
    return d->m_mv_context1->currentTimeRange();
}

void MCContext::setCurrentEvent(const MCEvent &evt)
{
    MVEvent evt2;
    evt2.label=evt.label;
    evt2.time=evt.time;
    if (evt.firings_num==1) {
        d->m_mv_context1->setCurrentEvent(evt2);
        d->m_mv_context2->setCurrentEvent(MVEvent());
    }
    else if (evt.firings_num==2) {
        d->m_mv_context2->setCurrentEvent(evt2);
        d->m_mv_context1->setCurrentEvent(MVEvent());
    }
    else {
        d->m_mv_context1->setCurrentEvent(MVEvent());
        d->m_mv_context2->setCurrentEvent(MVEvent());
    }
}

void MCContext::setCurrentCluster(const MCCluster &C)
{
    if (C.firings_num==1) {
        d->m_mv_context1->setCurrentCluster(C.num);
        d->m_mv_context2->setCurrentCluster(-1);
    }
    else if (evt.firings_num==2) {
        d->m_mv_context2->setCurrentCluster(C.num);
        d->m_mv_context1->setCurrentCluster(-1);
    }
    else {
        d->m_mv_context1->setCurrentCluster(-1);
        d->m_mv_context2->setCurrentCluster(-1);
    }
}

bool matches(const QSet<MCCluster> &clusters1,const QSet<MCCluster> &clusters2) {
    if (clusters1.count()!=clusters2.count())
        return false;
    foreach (MCCluster C,clusters1) {
        if (!clusters2.contains(C))
            return false;
    }
    return true;
}

void MCContext::setSelectedClusters(const QList<MCCluster> &Cs)
{
    if (matches(Cs.toSet(),d->m_selected_clusters.toSet()))
        return;
    QList<int> C1s,C2s;
    foreach (MCCluster C,Cs) {
        if (C.firings_num==1) {
            C1s << C.num;
        }
        else if (C.firings_num==2) {
            C2s << C.num;
        }
    }
    d->m_mv_context1->setSelectedClusters(C1s);
    d->m_mv_context2->setSelectedClusters(C2s);
}

void MCContext::setCurrentTimepoint(double tp)
{
    d->m_mv_context1->setCurrentTimepoint(tp);
}

void MCContext::setCurrentTimeRange(const MVRange &range)
{
    d->m_mv_context1->setCurrentTimeRange(range);
}

void MCContext::clickCluster(MCCluster C, Qt::KeyboardModifiers modifiers)
{
    if (C.firings_num==1) {
        d->m_mv_context1->clickCluster(C,modifiers);
        if (!(modifiers & Qt::ControlModifier)) {
            d->m_mv_context2->setSelectedClusters(QList<int>());
        }
    }
    else if (C.firings_num==2) {
        d->m_mv_context2->clickCluster(C,modifiers);
        if (!(modifiers & Qt::ControlModifier)) {
            d->m_mv_context1->setSelectedClusters(QList<int>());
        }
    }
}

QColor MCContext::clusterColor(int k) const
{
    return d->m_mv_context1->clusterColor(k);
}

QColor MCContext::channelColor(int m) const
{
    return d->m_mv_context1->channelColor(m);
}

QColor MCContext::color(QString name, QColor default_color) const
{
    return d->m_mv_context1->color(name,default_color);
}

QMap<QString, QColor> MCContext::colors() const
{
    return d->m_mv_context1->colors();
}

QList<QColor> MCContext::channelColors() const
{
    return d->m_mv_context1->channelColors();
}

QList<QColor> MCContext::clusterColors() const
{
    return d->m_mv_context1->clusterColors();
}

void MCContext::setClusterColors(const QList<QColor> &colors)
{
    d->m_mv_context1->setClusterColors(colors);
    d->m_mv_context2->setClusterColors(colors);
}

void MCContext::setChannelColors(const QList<QColor> &colors)
{
    d->m_mv_context1->setChannelColors(colors);
    d->m_mv_context2->setChannelColors(colors);
}

void MCContext::setColors(const QMap<QString, QColor> &colors)
{
    d->m_mv_context1->setColors(colors);
    d->m_mv_context2->setColors(colors);
}

DiskReadMda MCContext::currentTimeseries() const
{
    return d->m_mv_context1->currentTimeseries();
}

DiskReadMda MCContext::timeseries(QString name) const
{
    return d->m_mv_context1->timeseries(name);
}

QString MCContext::currentTimeseriesName() const
{
    return d->m_mv_context1->currentTimeseriesName();
}

QStringList MCContext::timeseriesNames() const
{
    return d->m_mv_context1->timeseriesNames();
}

void MCContext::addTimeseries(QString name, DiskReadMda timeseries)
{
    d->m_mv_context1->addTimeseries(timeseries);
    d->m_mv_context2->addTimeseries(timeseries);
}

void MCContext::setCurrentTimeseriesName(QString name)
{
    d->m_mv_context1->setCurrentTimeseriesName(name);
    d->m_mv_context2->setCurrentTimeseriesName(name);
}

DiskReadMda MCContext::firings1()
{
    return d->m_mv_context1->firings();
}

DiskReadMda MCContext::firings2()
{
    return d->m_mv_context2->firings();
}

void MCContext::setFirings1(const DiskReadMda &F)
{
    d->m_mv_context1->setFirings(F);
}

void MCContext::setFirings2(const DiskReadMda &F)
{
    d->m_mv_context2->setFirings(F);
}

double MCContext::sampleRate() const
{
    return d->m_mv_context1->sampleRate();
}

void MCContext::setSampleRate(double sample_rate)
{
    d->m_mv_context1->setSampleRate(sample_rate);
    d->m_mv_context2->setSampleRate(sample_rate);
}

Mda MCContext::confusionMatrix() const
{
    return d->m_confusion_matrix;
}

QList<int> MCContext::optimalLabelMap() const
{
    return d->m_optimal_label_map;
}

/*
void MCContext::computeMergedFirings()
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (d->m_merged_firings_computed)
            return;
    }

    d->m_calculator.mlproxy_url = this->mlProxyUrl();
    d->m_calculator.firings1 = this->firings1();
    d->m_calculator.firings2 = this->firings2();
    d->m_calculator.compute();
    d->m_confusion_matrix = d->m_calculator.confusion_matrix;
    d->m_optimal_label_map = d->m_calculator.optimal_label_map;
    d->m_firings_merged = d->m_calculator.firings_merged;

    {
        QMutexLocker locker(&d->m_mutex);
        d->m_merged_firings_computed = true;
    }
}

Mda MCContext::confusionMatrix() const
{
    Mda ret;
    d->m_confusion_matrix.readChunk(ret, 0, 0, d->m_confusion_matrix.N1(), d->m_confusion_matrix.N2());
    return ret;
}

QList<int> MCContext::optimalLabelMap() const
{
    QList<int> ret;
    for (int i = 0; i < d->m_optimal_label_map.totalSize(); i++) {
        ret << d->m_optimal_label_map.value(i);
    }
    return ret;
}

DiskReadMda MCContext::firingsMerged() const
{
    return d->m_firings_merged;
}

DiskReadMda MCContext::firings1()
{
    return d->m_firings1;
}

DiskReadMda MCContext::firings2()
{
    return d->m_firings2;
}

int MCContext::currentCluster2() const
{
    return d->m_current_cluster2;
}

QList<int> MCContext::selectedClusters2() const
{
    QList<int> ret = d->m_selected_clusters2.toList();
    qSort(ret);
    return ret;
}

void MCContext::setFirings1(const DiskReadMda& F)
{
    d->m_firings1 = F;
    emit firings1Changed();
}

void MCContext::setFirings2(const DiskReadMda& F)
{
    d->m_firings2 = F;
    emit firings2Changed();
}

void MCContext::setCurrentCluster2(int k)
{
    if (d->m_current_cluster2 == k)
        return;
    d->m_current_cluster2 = k;
    emit currentCluster2Changed();
}

void MCContext::setSelectedClusters2(const QList<int>& list)
{
    if (list.toSet() == d->m_selected_clusters2)
        return;
    d->m_selected_clusters2 = list.toSet();
    emit selectedClusters2Changed();
}

void MCContext::clickCluster2(int k, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier) {
        if (k < 0)
            return;
        if (d->m_selected_clusters2.contains(k)) {
            QList<int> tmp = d->m_selected_clusters2.toList();
            tmp.removeAll(k);
            this->setSelectedClusters2(tmp);
        }
        else {
            if (k >= 0) {
                QList<int> tmp = d->m_selected_clusters2.toList();
                tmp << k;
                this->setSelectedClusters2(tmp);
            }
        }
    }
    else {
        //this->setSelectedClusterPairs(QSet<ClusterPair>());
        this->setSelectedClusters2(QList<int>());
        this->setCurrentCluster2(k);
    }
}

void MergeFiringsCalculator::compute()
{
    TaskProgress task(TaskProgress::Calculate, "Compute confusion matrix");
    task.setProgress(0.1);

    MountainProcessRunner MPR;
    MPR.setProcessorName("merge_firings");

    QMap<QString, QVariant> params;
    params["firings1"] = firings1.makePath();
    params["firings2"] = firings2.makePath();
    params["max_matching_offset"] = 30;
    MPR.setInputParameters(params);
    MPR.setMLProxyUrl(mlproxy_url);

    task.log() << "Firings 1/2 dimensions" << firings1.N1() << firings1.N2() << firings2.N1() << firings2.N2();

    QString firings_merged_path = MPR.makeOutputFilePath("firings_merged");
    QString optimal_label_map_path = MPR.makeOutputFilePath("optimal_label_map");
    QString confusion_matrix_path = MPR.makeOutputFilePath("confusion_matrix");

    MPR.runProcess();

    if (MLUtil::threadInterruptRequested()) {
        task.error(QString("Halted while running process."));
        return;
    }

    firings_merged.setPath(firings_merged_path);
    confusion_matrix.setPath(confusion_matrix_path);
    optimal_label_map.setPath(optimal_label_map_path);
}

void MCContextPrivate::setup_mv_contexts()
{
    //finish
}
*/
