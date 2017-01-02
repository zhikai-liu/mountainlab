/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "spectrogramview.h"
#include <math.h>

#include <QAction>
#include <QIcon>
#include <QImageWriter>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <mountainprocessrunner.h>
#include <mveegcontext.h>
#include <qlabel.h>
#include <taskprogress.h>

class SpectrogramViewCalculator {
public:
    //input
    DiskReadMda timeseries;
    //QString mlproxy_url;

    //output
    DiskReadMda spectrogram;

    void compute();
};

class SpectrogramViewPrivate {
public:
    SpectrogramView* q;
    DiskReadMda m_spectrogram;
    double m_window_min = 0, m_window_max = 2;
    double m_brightness_level = 0;

    SpectrogramViewCalculator m_calculator;

    QColor get_pixel_color(double val);
};

SpectrogramView::SpectrogramView(MVAbstractContext* context)
    : MVTimeSeriesViewBase(context)
{
    d = new SpectrogramViewPrivate;
    d->q = this;

    MVEEGContext* c = qobject_cast<MVEEGContext*>(context);
    Q_ASSERT(c);

    {
        QSlider* S = new QSlider;
        S->setOrientation(Qt::Horizontal);
        S->setRange(-100, 100);
        S->setValue(0);
        S->setMaximumWidth(150);
        this->addToolbarControl(new QLabel("brightness:"));
        this->addToolbarControl(S);
        QObject::connect(S, SIGNAL(valueChanged(int)), this, SLOT(slot_brightness_slider_changed(int)));
    }

    /*
    {
        QAction* a = new QAction(QIcon(":/images/vertical-zoom-in.png"), "Vertical Zoom In", this);
        a->setProperty("action_type", "toolbar");
        a->setToolTip("Vertical zoom in. Alternatively, use the UP arrow.");
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(slot_vertical_zoom_in()));
    }
    {
        QAction* a = new QAction(QIcon(":/images/vertical-zoom-out.png"), "Vertical Zoom Out", this);
        a->setProperty("action_type", "toolbar");
        a->setToolTip("Vertical zoom out. Alternatively, use the DOWN arrow.");
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(slot_vertical_zoom_out()));
    }
    */

    this->recalculateOn(context, SIGNAL(currentTimeseriesChanged()));

    this->recalculate();
}

SpectrogramView::~SpectrogramView()
{
    this->stopCalculation();
    delete d;
}

void SpectrogramView::prepareCalculation()
{
    MVEEGContext* c = qobject_cast<MVEEGContext*>(mvContext());
    Q_ASSERT(c);

    //d->m_layout_needed = true;
    d->m_calculator.timeseries = c->currentTimeseries();

    MVTimeSeriesViewBase::prepareCalculation();
}

void SpectrogramView::runCalculation()
{
    d->m_calculator.compute();

    MVTimeSeriesViewBase::runCalculation();
}

void SpectrogramView::onCalculationFinished()
{
    MVEEGContext* c = qobject_cast<MVEEGContext*>(mvContext());
    Q_ASSERT(c);

    d->m_spectrogram = d->m_calculator.spectrogram;

    MVTimeSeriesViewBase::onCalculationFinished();
}

void SpectrogramView::resizeEvent(QResizeEvent* evt)
{
    //d->m_layout_needed = true;
    MVTimeSeriesViewBase::resizeEvent(evt);
}

void SpectrogramView::paintContent(QPainter* painter)
{
    MVEEGContext* c = qobject_cast<MVEEGContext*>(mvContext());
    Q_ASSERT(c);

    int time_resolution = 32;

    MVRange timerange = c->currentTimeRange();
    long t1 = ceil(timerange.min / time_resolution);
    long t2 = floor(timerange.max / time_resolution);

    if (t2 <= t1 + 2)
        return;

    int M = d->m_spectrogram.N1();
    int N2 = t2 - t1 + 1;
    int K = d->m_spectrogram.N3();

    QImage img = QImage(N2, M, QImage::Format_RGB32);
    for (int n = 0; n < N2; n++) {
        for (int m = 0; m < M; m++) {
            double val = d->m_spectrogram.value(m, t1 + n, 10);
            QColor col = d->get_pixel_color(val);
            img.setPixel(n, m, col.rgb());
        }
    }

    QRectF geom = this->contentGeometry();
    // TODO: fine adjustment on this based on t1,t2,timerange
    QImage img2 = img.scaled(geom.width(), geom.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->drawImage(geom.topLeft(), img2);

    /*
    // Geometry of channels
    if (d->m_layout_needed) {
        int M = d->m_num_channels;
        d->m_layout_needed = false;
        d->m_channels = d->make_channel_layout(M);
        for (long m = 0; m < M; m++) {
            mvtsv_channel* CH = &d->m_channels[m];
            CH->channel = m;
            CH->label = QString("%1").arg(m + 1);
        }
    }

    double WW = this->contentGeometry().width();
    double HH = this->contentGeometry().height();
    QImage img;
    img = d->m_render_manager.getImage(c->currentTimeRange().min, c->currentTimeRange().max, d->m_amplitude_factor, WW, HH);
    painter->drawImage(this->contentGeometry().left(), this->contentGeometry().top(), img);

    // Channel labels
    d->paint_channel_labels(painter, this->width(), this->height());
    */
}

void SpectrogramView::keyPressEvent(QKeyEvent* evt)
{
    /*
    if (evt->key() == Qt::Key_Up) {
        d->m_amplitude_factor *= 1.2;
        update();
    }
    else if (evt->key() == Qt::Key_Down) {
        d->m_amplitude_factor /= 1.2;
        update();
    }
    else {
        MVTimeSeriesViewBase::keyPressEvent(evt);
    }
    */
}

void SpectrogramView::slot_brightness_slider_changed(int val)
{
    d->m_brightness_level = val;
    this->update();
}

void SpectrogramViewCalculator::compute()
{

    TaskProgress task(TaskProgress::Calculate, "eeg-spectrogram");
    MountainProcessRunner X;
    QString processor_name = "eeg-spectrogram";
    X.setProcessorName(processor_name);

    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries.makePath();
    X.setInputParameters(params);

    QString spectrogram_fname = X.makeOutputFilePath("spectrogram_out");

    X.runProcess();
    spectrogram.setPath(spectrogram_fname);
}

SpectrogramDataFactory::SpectrogramDataFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SpectrogramDataFactory::id() const
{
    return QStringLiteral("open-Spectrogram-data");
}

QString SpectrogramDataFactory::name() const
{
    return tr("Spectrogram Data");
}

QString SpectrogramDataFactory::title() const
{
    return tr("Spectrogram");
}

MVAbstractView* SpectrogramDataFactory::createView(MVAbstractContext* context)
{
    MVEEGContext* c = qobject_cast<MVEEGContext*>(context);
    Q_ASSERT(c);

    SpectrogramView* X = new SpectrogramView(context);
    //QList<int> ks = c->selectedClusters();
    //X->setLabelsToView(ks.toSet());
    return X;
}

QColor SpectrogramViewPrivate::get_pixel_color(double val)
{
    double pct = (val - m_window_min) / (m_window_max - m_window_min);
    double brightness_factor = exp(m_brightness_level / 100 * 5);
    pct *= brightness_factor;
    pct = qMin(1.0, qMax(0.0, pct));
    QColor gray = QColor(pct * 255, pct * 255, pct * 255);
    return gray;
}
