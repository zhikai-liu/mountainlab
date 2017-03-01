/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "sslvtimespanview.h"
#include <math.h>

#include "mvutils.h"
#include <QImageWriter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <taskprogress.h>
//#include "mvclusterlegend.h"
#include "paintlayerstack.h"
#include "sslvcontext.h"

/// TODO: (MEDIUM) control brightness in firing event view

/*
class MVTimeSpanViewCalculator {
public:
    //input
    QString timeseries;
    QString firings;
    QString mlproxy_url;
    QSet<int> labels_to_use;

    //output
    QVector<double> times;
    QVector<int> labels;
    QVector<double> amplitudes;

    void compute();
};
*/

class TimeSpanAxisLayer : public PaintLayer {
public:
    void paint(QPainter* painter) Q_DECL_OVERRIDE;

    QRectF content_geometry;
    //MVRange amplitude_range;
};

class TimeSpanContentLayer;
class TimeSpanImageCalculator : public QThread {
public:
    void run();

    /// TODO only calculate the image in the content geometry rect so that we don't need things like window_size and the image may be safely resized on resizing the window

    //input
    QList<QColor> cluster_colors;
    QRectF content_geometry;
    QSize window_size;
    MVRange time_range;
    QList<int> clusters_to_show;
    QJsonObject cluster_metrics;
    double samplerate;

    //output
    QImage image;

    QColor cluster_color(int k);
    double time2xpix(double t);
    double val2ypix(double val);
};

class SSLVTimeSpanViewPrivate {
public:
    SSLVTimeSpanView* q;
    //DiskReadMda m_timeseries;

    QList<int> m_clusters_to_show;

    TimeSpanContentLayer* m_content_layer;
    //MVClusterLegend* m_legend;

    //MVTimeSpanViewCalculator m_calculator;

    PaintLayerStack m_paint_layer_stack;
    TimeSpanAxisLayer* m_axis_layer;

    double val2ypix(double val);
    double ypix2val(double ypix);
};

SSLVTimeSpanView::SSLVTimeSpanView(MVAbstractContext* context)
    : SSLVTimeSeriesViewBase(context)
{
    d = new SSLVTimeSpanViewPrivate;
    d->q = this;

    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    this->setMouseTracking(true);

    //d->m_legend = new MVClusterLegend;
    //d->m_legend->setClusterColors(c->clusterColors());

    d->m_content_layer = new TimeSpanContentLayer;

    d->m_axis_layer = new TimeSpanAxisLayer;
    d->m_paint_layer_stack.addLayer(d->m_content_layer);
    d->m_paint_layer_stack.addLayer(d->m_axis_layer);
    //d->m_paint_layer_stack.addLayer(d->m_legend);
    connect(&d->m_paint_layer_stack, SIGNAL(repaintNeeded()), this, SLOT(update()));

    onOptionChanged("cluster_color_index_shift", this, SLOT(onCalculationFinished()));

    this->setMargins(60, 60, 40, 40);

    mvtsv_prefs p = this->prefs();
    p.colors.axis_color = Qt::white;
    p.colors.text_color = Qt::white;
    p.colors.background_color = Qt::black;
    this->setPrefs(p);

    connect(c, SIGNAL(currentTimeRangeChanged()), SLOT(slot_update_image()));

    this->setNumTimepoints(c->maxTimepoint() + 1);

    //this->recalculateOn(c, SIGNAL(clusterMergeChanged()));
    //this->recalculateOn(c, SIGNAL(firingsChanged()), false);
    this->recalculate();
}

SSLVTimeSpanView::~SSLVTimeSpanView()
{
    this->stopCalculation();
    delete d;
}

void SSLVTimeSpanView::prepareCalculation()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    /*
    d->m_calculator.labels_to_use = d->m_labels_to_use;
    d->m_calculator.timeseries = c->currentTimeseries().makePath();
    d->m_calculator.firings = c->firings().makePath();
    */
}

void SSLVTimeSpanView::runCalculation()
{
    //d->m_calculator.compute();
}

void SSLVTimeSpanView::onCalculationFinished()
{
    /*
    d->m_labels0 = d->m_calculator.labels;
    d->m_times0 = d->m_calculator.times;
    d->m_amplitudes0 = d->m_calculator.amplitudes;

    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    if (c->viewMerged()) {
        d->m_labels0 = c->clusterMerge().mapLabels(d->m_labels0);
    }

    {
        QSet<int> X;
        foreach (int k, d->m_labels_to_use) {
            X.insert(c->clusterMerge().representativeLabel(k));
        }
        QList<int> list = X.toList();
        qSort(list);
        //d->m_legend->setClusterNumbers(list);
    }

    slot_update_image();
    /// TODO: (MEDIUM) only do this if user has specified that it should be auto calculated (should be default)
    this->autoSetAmplitudeRange();
    */
}

void SSLVTimeSpanView::setClustersToShow(const QList<int>& clusters_to_show)
{
    d->m_clusters_to_show = clusters_to_show;
    this->recalculate();
}

void SSLVTimeSpanView::mouseMoveEvent(QMouseEvent* evt)
{
    d->m_paint_layer_stack.mouseMoveEvent(evt);
    if (evt->isAccepted())
        return;
    SSLVTimeSeriesViewBase::mouseMoveEvent(evt);
    /*
    int k=d->m_legend.clusterNumberAt(evt->pos());
    if (d->m_legend.hoveredClusterNumber()!=k) {
        d->m_legend.setHoveredClusterNumber(k);
        update();
    }
    */
}

void SSLVTimeSpanView::mouseReleaseEvent(QMouseEvent* evt)
{
    d->m_paint_layer_stack.mouseReleaseEvent(evt);
    if (evt->isAccepted())
        return;
    /*
    int k=d->m_legend.clusterNumberAt(evt->pos());
    if (k>0) {
        /// TODO (LOW) make the legend more like a widget, responding to mouse clicks and movements on its own, and emitting signals
        d->m_legend.toggleActiveClusterNumber(k);
        evt->ignore();
        update();
    }
    */
    SSLVTimeSeriesViewBase::mouseReleaseEvent(evt);
}

void SSLVTimeSpanView::resizeEvent(QResizeEvent* evt)
{
    d->m_axis_layer->content_geometry = this->contentGeometry();
    d->m_paint_layer_stack.setWindowSize(this->size());
    slot_update_image();
    SSLVTimeSeriesViewBase::resizeEvent(evt);
}

void SSLVTimeSpanView::slot_update_image()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_content_layer->calculator->requestInterruption();
    d->m_content_layer->calculator->wait();
    d->m_content_layer->calculator->cluster_colors = c->clusterColors();
    d->m_content_layer->calculator->content_geometry = this->contentGeometry();
    d->m_content_layer->calculator->window_size = this->size();
    d->m_content_layer->calculator->time_range = c->currentTimeRange();
    d->m_content_layer->calculator->clusters_to_show = d->m_clusters_to_show;
    d->m_content_layer->calculator->cluster_metrics = c->clusterMetrics();
    d->m_content_layer->calculator->samplerate = c->sampleRate();
    d->m_content_layer->calculator->start();
}

void SSLVTimeSpanView::paintContent(QPainter* painter)
{
    d->m_paint_layer_stack.paint(painter);

    //legend
    //d->m_legend.setParentWindowSize(this->size());
    //d->m_legend.draw(painter);
    /*
    {
        double spacing = 6;
        double margin = 10;
        // it would still be better if m_labels.was presorted right from the start
        QList<int> list = d->m_labels_to_use.toList();
        qSort(list);
        double text_height = qBound(12.0, width() * 1.0 / 10, 25.0);
        double y0 = margin;
        QFont font = painter->font();
        font.setPixelSize(text_height - 1);
        painter->setFont(font);
        for (int i = 0; i < list.count(); i++) {
            QRectF rect(0, y0, width() - margin, text_height);
            QString str = QString("%1").arg(list[i]);
            QPen pen = painter->pen();
            pen.setColor(mvContext()->clusterColor(list[i]));
            painter->setPen(pen);
            painter->drawText(rect, Qt::AlignRight, str);
            y0 += text_height + spacing;
        }
    }
    */
}

double SSLVTimeSpanViewPrivate::val2ypix(double val)
{
    double pcty = (val + 0.5) / m_clusters_to_show.count();

    return q->contentGeometry().top() + pcty * q->contentGeometry().height();
}

double SSLVTimeSpanViewPrivate::ypix2val(double ypix)
{
    double pcty = 1 - (ypix - q->contentGeometry().top()) / q->contentGeometry().height();
    double val = pcty * m_clusters_to_show.count() - 0.5;
    return val;
}

/*
void MVTimeSpanViewCalculator::compute()
{
    TaskProgress task("Computing firing events");

    DiskReadMda firings2 = compute_amplitudes(timeseries, firings, mlproxy_url);

    long L = firings2.N2();
    times.clear();
    labels.clear();
    amplitudes.clear();
    for (long i = 0; i < L; i++) {
        if (i % 100 == 0) {
            if (MLUtil::threadInterruptRequested()) {
                task.error("Halted");
                return;
            }
        }
        int label0 = (int)firings2.value(2, i);
        if (labels_to_use.contains(label0)) {
            task.setProgress(i * 1.0 / L);
            times << firings2.value(1, i);
            labels << label0;
            amplitudes << firings2.value(3, i);
        }
    }
    task.log(QString("Found %1 events, using %2 clusters").arg(times.count()).arg(labels_to_use.count()));
}
*/

void TimeSpanAxisLayer::paint(QPainter* painter)
{
    draw_axis_opts opts;
    opts.minval = opts.maxval = 0;
    //opts.minval = amplitude_range.min;
    //opts.maxval = amplitude_range.max;
    opts.orientation = Qt::Vertical;
    opts.pt1 = content_geometry.bottomLeft() + QPointF(-3, 0);
    opts.pt2 = content_geometry.topLeft() + QPointF(-3, 0);
    opts.tick_length = 5;
    opts.color = Qt::white;
    draw_axis(painter, opts);
}

TimeSpanContentLayer::TimeSpanContentLayer()
{
    calculator = new TimeSpanImageCalculator;
    //Direct connection appears to be important here
    connect(calculator, SIGNAL(finished()), this, SLOT(slot_calculator_finished()), Qt::DirectConnection);
}

TimeSpanContentLayer::~TimeSpanContentLayer()
{
    calculator->requestInterruption();
    calculator->wait();
    delete calculator;
}

void TimeSpanContentLayer::paint(QPainter* painter)
{
    painter->drawImage(0, 0, output_image);
}

void TimeSpanContentLayer::setWindowSize(QSize size)
{
    PaintLayer::setWindowSize(size);
}

/*
void TimeSpanContentLayer::updateImage()
{
    calculator->requestInterruption();
    calculator->wait();
    calculator->start();
}
*/

void TimeSpanContentLayer::slot_calculator_finished()
{
    if (this->calculator->isRunning()) {
        qWarning() << "Calculator still running even though we are in slot_caculator_finished";
        return;
    }
    if (this->calculator->isInterruptionRequested())
        return;
    this->output_image = this->calculator->image;
    emit repaintNeeded();
}

void TimeSpanImageCalculator::run()
{
    image = QImage(window_size, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    image.fill(transparent);
    QPainter painter(&image);

    QJsonArray clusters = cluster_metrics["clusters"].toArray();

    QMap<int,int> cluster_index_lookup;
    for (int i=0; i<clusters.count(); i++) {
        QJsonObject cluster = clusters.at(i).toObject();
        int label=cluster["label"].toInt();
        cluster_index_lookup[label]=i;
    }

    for (int i = 0; i < clusters_to_show.count(); i++) {
        int k = clusters_to_show[i];
        if (cluster_index_lookup.contains(k)) {
            int jj=cluster_index_lookup.value(k);
            QJsonObject cluster = clusters.at(jj).toObject();
            QJsonObject metrics = cluster["metrics"].toObject();
            double t1_sec = metrics["t1_sec"].toDouble();
            double t2_sec = metrics["t2_sec"].toDouble();
            double t1 = t1_sec * samplerate;
            double t2 = t2_sec * samplerate;
            double xpix1 = time2xpix(t1);
            double xpix2 = time2xpix(t2);
            double ypix0=val2ypix(i);
            double ypix1 = val2ypix(i - 0.4);
            double ypix2 = val2ypix(i + 0.4);
            qDebug() << t1_sec << t2_sec << t1 << t2 << xpix1 << xpix2;
            if (ypix2-ypix1<=2) {
                ypix1=ypix0-1;
                ypix2=ypix0+1;
            }
            QRectF R(xpix1, ypix1, xpix2 - xpix1, ypix2 - ypix1);
            QColor col = cluster_colors.value(k % cluster_colors.count());
            painter.fillRect(R, col);
        }
    }

    /*
    double alpha_pct = 0.7;
    for (long i = 0; i < times.count(); i++) {
        if (this->isInterruptionRequested())
            return;
        double t0 = times.value(i);
        int k0 = labels.value(i);
        QColor col = cluster_color(k0);
        col.setAlpha((int)(alpha_pct * 255));
        QPen pen = painter.pen();
        pen.setColor(col);
        painter.setPen(pen);
        double amp0 = amplitudes.value(i);
        double xpix = time2xpix(t0);
        double ypix = val2ypix(amp0);
        if ((xpix >= 0) && (ypix >= 0)) {
            painter.drawEllipse(xpix, ypix, 3, 3);
        }
    }
    */
}

QColor TimeSpanImageCalculator::cluster_color(int k)
{
    if (k <= 0)
        return Qt::white;
    if (cluster_colors.isEmpty())
        return Qt::white;
    return cluster_colors[(k - 1) % cluster_colors.count()];
}

/// TODO do not duplicate code in time2xpix and val2ypix, but beware of calculator thread
double TimeSpanImageCalculator::time2xpix(double t)
{
    double view_t1 = time_range.min;
    double view_t2 = time_range.max;

    if (view_t2 <= view_t1)
        return 0;

    double xpct = (t - view_t1) / (view_t2 - view_t1);
    //if ((xpct < 0) || (xpct > 1))
    //    return -1;
    double px = content_geometry.x() + xpct * content_geometry.width();
    return px;
}

double TimeSpanImageCalculator::val2ypix(double val)
{
    double pcty = (val + 0.5) / clusters_to_show.count();

    return content_geometry.top() + pcty * content_geometry.height();
}
