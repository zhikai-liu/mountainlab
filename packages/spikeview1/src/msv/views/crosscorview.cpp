/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "crosscorview.h"
//#include "crosscorviewpanel.h"
#include "histogramview.h"
#include "get_sort_indices.h"
#include "mvthreadmanager.h"

#include <QLabel>
#include <QMutex>
#include <QSpinBox>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <diskreadmda.h>
#include <mountainprocessrunner.h>
#include <svcontext.h>
#include <taskprogress.h>
#include "actionfactory.h"

struct CrosscorHistData {
    QVector<double> data;
};

class CrosscorViewCalculator {
public:
    //input
    DiskReadMda firings;
    QList<int> k1s, k2s;

    //output
    QVector<double> times;
    QVector<int> labels;

    virtual void compute();
};

class CrosscorViewPrivate {
public:
    CrosscorView* q;

    CrosscorViewCalculator m_calculator;

    QList<int> m_ks;
    QList<int> m_k1s, m_k2s;
    QList<HistogramView*> m_panels;

    CrosscorWorker m_worker;

    void update_panels();
};

CrosscorView::CrosscorView(MVAbstractContext* context)
    : MVGridView(context)
{
    d = new CrosscorViewPrivate;
    d->q = this;

    SVContext* c = qobject_cast<SVContext*>(context);
    Q_ASSERT(c);

    this->recalculateOn(c, SIGNAL(firingsChanged()), false);
    //this->recalculateOnOptionChanged("templates_clip_size");

    QObject::connect(c, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(c, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));

    connect(this, SIGNAL(signalViewClicked(int, Qt::KeyboardModifiers)), this, SLOT(slot_panel_clicked(int, Qt::KeyboardModifiers)));

    connect(&d->m_worker, SIGNAL(data_update()), this, SLOT(slot_worker_data_update()));

    this->recalculateOnOptionChanged("cc_max_dt_msec");
    this->recalculateOnOptionChanged("cc_bin_size_msec");

    this->recalculate();
}

CrosscorView::~CrosscorView()
{
    d->m_worker.requestInterruption();
    d->m_worker.wait();

    this->stopCalculation();
    delete d;
}

void CrosscorView::setKs(const QList<int>& k1s, const QList<int>& k2s, const QList<int>& ks)
{
    d->m_k1s = k1s;
    d->m_k2s = k2s;
    d->m_ks = ks;
    this->recalculate();
}

void CrosscorView::prepareCalculation()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_worker.requestInterruption();
    d->m_worker.wait();

    d->m_calculator.firings = c->firings();
    d->m_calculator.k1s = d->m_k1s;
    d->m_calculator.k2s = d->m_k2s;

    update();
}

void CrosscorView::runCalculation()
{
    d->m_calculator.compute();
}

void CrosscorView::onCalculationFinished()
{
    d->update_panels();
}

void CrosscorView::keyPressEvent(QKeyEvent* evt)
{
    (void)evt;
    /*
    if (evt->key() == Qt::Key_Up) {
        slot_vertical_zoom_in();
    }
    if (evt->key() == Qt::Key_Down) {
        slot_vertical_zoom_out();
    }
    */
}

void CrosscorView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    /*
    int view_index = d->find_view_index_at(pos);
    if (view_index >= 0) {
        int k = d->m_views[view_index]->k();
        if (!c->selectedClusters().contains(k)) {
            c->clickCluster(k, Qt::NoModifier);
        }
    }
    */

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << c->selectedClusters();
    mimeData.setData("application/x-msv-clusters", ba); // selected cluster data

    MVAbstractView::prepareMimeData(mimeData, pos); // call base class implementation
}

void CrosscorView::slot_update_panels()
{
    d->update_panels();
}

void CrosscorView::slot_update_highlighting()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    int current_k = c->currentCluster();
    QSet<int> selected_ks = c->selectedClusters().toSet();
    for (int i = 0; i < d->m_panels.count(); i++) {
        int k0 = d->m_panels[i]->property("k").toInt();
        d->m_panels[i]->setCurrent(current_k == k0);
        d->m_panels[i]->setSelected(selected_ks.contains(k0));
    }
    updateViews();
}

void CrosscorView::slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers)
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    if (modifiers & Qt::ShiftModifier) {
        int i1 = MVGridView::currentViewIndex();
        int i2 = index;
        int j1 = qMin(i1, i2);
        int j2 = qMax(i1, i2);
        if ((j1 >= 0) && (j2 >= 0)) {
            QSet<int> set = c->selectedClusters().toSet();
            for (int j = j1; j <= j2; j++) {
                HistogramView* panel = d->m_panels.value(j);
                if (panel) {
                    int k = panel->property("k").toInt();
                    if (k > 0)
                        set.insert(k);
                }
            }
            c->setSelectedClusters(set.toList());
        }
    }
    else {
        HistogramView* panel = d->m_panels.value(index);
        if (panel) {
            int k = panel->property("k").toInt();
            if (k > 0) {
                c->clickCluster(k, modifiers);
            }
        }
    }
}

void CrosscorView::slot_worker_data_update()
{
    QMutexLocker locker(&d->m_worker.mutex);
    for (int i = 0; i < d->m_panels.count(); i++) {
        HistogramView* P = d->m_panels[i];
        int k1 = P->property("k1").toInt();
        int k2 = P->property("k2").toInt();
        if (d->m_worker.hist_data.contains(k1)) {
            if (d->m_worker.hist_data[k1].contains(k2)) {
                QVector<double> tmp = d->m_worker.hist_data[k1][k2];
                d->m_worker.hist_data[k1][k2].clear();
                P->appendData(tmp);
            }
        }
    }
}

void CrosscorViewCalculator::compute()
{
    QVector<double> times0(firings.N2());
    QVector<int> labels0(firings.N2());
    for (bigint i = 0; i < firings.N2(); i++) {
        times0[i] = firings.value(1, i);
        labels0[i] = firings.value(2, i);
    }
    QList<bigint> inds0 = get_sort_indices_bigint(times0);

    times.resize(firings.N2());
    labels.resize(firings.N2());
    for (bigint j = 0; j < inds0.count(); j++) {
        times[j] = times0[inds0[j]];
        labels[j] = labels0[inds0[j]];
    }
}

void CrosscorViewPrivate::update_panels()
{
    SVContext* c = qobject_cast<SVContext*>(q->mvContext());
    Q_ASSERT(c);

    m_panels.clear();
    q->clearViews();

    double max_dt_msec = c->option("cc_max_dt_msec", 100).toDouble();
    double bin_size_msec = c->option("cc_bin_size_msec", 1).toDouble();
    double dt_max = (c->sampleRate() / 1000) * max_dt_msec;
    double bin_size = (c->sampleRate() / 1000) * bin_size_msec;
    int num_bins = (int)(2 * dt_max / bin_size);
    if (num_bins > 10000)
        num_bins = 10000;
    if (num_bins < 1)
        num_bins = 1;

    bool auto_only = true;
    for (int i = 0; i < m_k1s.count(); i++) {
        if (m_k1s[i] != m_k2s[i])
            auto_only = false;
    }

    for (int i = 0; i < m_k1s.count(); i++) {
        HistogramView* panel = new HistogramView;
        QString title0;
        if (auto_only)
            title0 = QString("%1").arg(m_k1s.value(i));
        else
            title0 = QString("%1/%2").arg(m_k1s.value(i)).arg(m_k2s.value(i));
        panel->setTitle(title0);
        panel->setProperty("k1", m_k1s.value(i));
        panel->setProperty("k2", m_k2s.value(i));
        panel->setProperty("k", m_ks.value(i));
        panel->setColors(c->colors());
        //panel->setData(m_hist_data.value(i).data);
        panel->setBinInfo(-dt_max, dt_max, num_bins);
        m_panels << panel;
    }

    for (int i = 0; i < m_panels.count(); i++) {
        q->addView(m_panels[i]);
    }
    q->slot_update_highlighting();

    q->updateLayout();

    m_worker.requestInterruption();
    m_worker.wait();
    m_worker.hist_data.clear();
    m_worker.times = m_calculator.times;
    m_worker.labels = m_calculator.labels;
    m_worker.k1s = m_k1s;
    m_worker.k2s = m_k2s;
    m_worker.dt_max = dt_max;

    //MVThreadManager::globalInstance()->addThread(&m_worker);
    m_worker.start();
}

void CrosscorWorker::run()
{
    TaskProgress TP("Computing cross-correlograms");
    bigint L = times.count();

    {
        QMutexLocker locker(&mutex);
        hist_data.clear();
        for (int i = 0; i < k1s.count(); i++) {
            int k1 = k1s[i];
            int k2 = k2s[i];
            if (!hist_data.contains(k1)) {
                QMap<int, QVector<double> > empty;
                hist_data[k1] = empty;
            }
            hist_data[k1][k2] = QVector<double>();
        }
    }

    bigint last_j_left = 0;
    QTime timer;
    timer.start();
    for (bigint i = 0; i < L; i++) {
        TP.setProgress(i * 1.0 / L);
        if (MLUtil::threadInterruptRequested())
            return;
        {
            QMutexLocker locker(&mutex);
            int k1 = labels[i];
            if (hist_data.contains(k1)) {
                QMap<int, QVector<double> >* hist_data1 = &hist_data[k1];
                double t1 = times[i];
                bigint j = last_j_left;
                while ((j < L) && (times[j] < t1 - dt_max))
                    j++;
                last_j_left = j;
                while ((j < L) && (times[j] <= t1 + dt_max)) {
                    if (j != i) {
                        int k2 = labels[j];
                        if (hist_data1->contains(k2)) {
                            double dt = times[j] - t1;
                            (*hist_data1)[k2] << dt;
                        }
                    }
                    j++;
                }
            }
        }
        if (timer.elapsed() > 2000) {
            emit this->data_update();
            timer.restart();
        }
    }

    emit this->data_update();
}
