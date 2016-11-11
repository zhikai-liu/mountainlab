#include "mvspikespraypanel.h"

#include <QList>
#include <QMutex>
#include <QPainter>
#include <QPen>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "mda.h"
#include "mlcommon.h"

namespace {
/*!
 * \brief A RAII object executes a given function upon destruction
 *
 *        Optionally it also executes another function upon construction.
 */
class RAII {
public:
    using Callback = std::function<void()>;
    RAII(const Callback& func, const Callback& cfunc = Callback())
    {
        m_func = func;
        if (cfunc)
            cfunc();
    }
    ~RAII()
    {
        if (m_func)
            m_func();
    }

private:
    Callback m_func;
};
}

class MVSpikeSprayPanelControlPrivate {
public:
    MVSpikeSprayPanelControlPrivate(MVSpikeSprayPanelControl* qq)
        : q(qq)
    {
    }
    MVSpikeSprayPanelControl* q;
    double amplitude = 1;
    double brightness = 2;
    double weight = 1;
    bool legendVisible = true;
    QSet<int> labels;
    QList<QColor> labelColors;
    Mda* clipsToRender = nullptr;
    QVector<int> renderLabels;
    bool needsRerender = true;
    QThread *renderThread;
    MVSSRenderer* renderer = nullptr;
    QImage render;
    int progress = 0;
    int allocProgress = 0;
};

class MVSpikeSprayPanelPrivate {
public:
    MVSpikeSprayPanel* q;
    MVContext* m_context;
    MVSpikeSprayPanelControl m_control;
};

MVSpikeSprayPanel::MVSpikeSprayPanel(MVContext* context)
{
    d = new MVSpikeSprayPanelPrivate;
    d->q = this;
    d->m_context = context;
    d->m_control.setLabelColors(context->clusterColors());
    QObject::connect(context, SIGNAL(clusterColorsChanged(QList<QColor>)), &d->m_control, SLOT(setLabelColors(QList<QColor>)));
    connect(&d->m_control, SIGNAL(renderAvailable()), this, SLOT(update()));
    connect(&d->m_control, SIGNAL(progressChanged(int)), this, SLOT(update()));
    connect(&d->m_control, SIGNAL(allocationProgressChanged(int)), this, SLOT(update()));
}

MVSpikeSprayPanel::~MVSpikeSprayPanel()
{
    delete d;
}

void MVSpikeSprayPanel::setLabelsToUse(const QSet<int>& labels)
{
    d->m_control.setLabels(labels);
}

void MVSpikeSprayPanel::setClipsToRender(Mda* X)
{
    d->m_control.setClips(X);
}

void MVSpikeSprayPanel::setLabelsToRender(const QVector<int>& X)
{
    d->m_control.setRenderLabels(X);
}

void MVSpikeSprayPanel::setAmplitudeFactor(double X)
{
    d->m_control.setAmplitude(X);
}

void MVSpikeSprayPanel::setBrightnessFactor(double factor)
{
    d->m_control.setBrightness(factor);
}

void MVSpikeSprayPanel::setWeightFactor(double factor)
{
    d->m_control.setWeight(factor);
}

void MVSpikeSprayPanel::setLegendVisible(bool val)
{
    d->m_control.setLegendVisible(val);
}

void MVSpikeSprayPanel::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /// TODO (LOW) this should be a configured color to match the cluster view
    painter.fillRect(0, 0, width(), height(), QBrush(QColor(0, 0, 0)));

    d->m_control.paint(&painter, rect());

    // render progress
    int progress = d->m_control.progress();
    int allocprogress = d->m_control.allocationProgress();
    if (progress > 0 && progress < 100) {
        QRect r(0, height() - 2, width() * progress * 1.0 / 100, 2);
        painter.fillRect(r, Qt::white);
    }
    else if (!progress && allocprogress > 0 && allocprogress < 100) {
        QRect r(0, height() - 2, width() * allocprogress * 1.0 / 100, 2);
        painter.fillRect(r, Qt::red);
    }
}

QColor brighten(QColor col, double factor)
{
    // TODO: replace with QColor::lighter
    double r = col.red() * factor;
    double g = col.green() * factor;
    double b = col.blue() * factor;
    r = qMin(255.0, r);
    g = qMin(255.0, g);
    b = qMin(255.0, b);
    return QColor(r, g, b);
}

void MVSSRenderer::render()
{
    RAII releaseRAII([this]() { release(); }); // make sure we call release while returning
    if (isInterruptionRequested()) {
        return;
    }
    m_processing = true;
    clearInterrupt();
    QImage image = QImage(W, H, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    replaceImage(image);
    emit imageUpdated(0);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    clips = allocator.allocate([this]() { return isInterruptionRequested(); });

    const long L = clips.N3();
    if (L != colors.count()) {
        qWarning() << "Unexpected sizes: " << colors.count() << L;
        return;
    }
    QTime timer;
    timer.start();
    for (long i = 0; i < L; i++) {
        if (timer.elapsed() > 300) { // each 300ms report an intermediate result
            replaceImage(image);
            emit imageUpdated(100 * i / L);
            timer.restart();
        }
        if (isInterruptionRequested()) {
            return;
        }
        render_clip(&painter, i);
    }
    replaceImage(image);
    emit imageUpdated(100);
}

void MVSSRenderer::render_clip(QPainter* painter, long i) const
{
    QColor col = colors[i];
    const long M = clips.N1();
    const long T = clips.N2();
    const double* ptr = clips.constDataPtr() + (M * T * i);
    QPen pen = painter->pen();
    pen.setColor(col);
    painter->setPen(pen);
    for (long m = 0; m < M; m++) {
        QPainterPath path;
        for (long t = 0; t < T; t++) {
            double val = ptr[m + M * t];
            QPointF pt = coord2pix(m, t, val);
            if (t == 0) {
                path.moveTo(pt);
            }
            else {
                path.lineTo(pt);
            }
        }
        painter->drawPath(path);
    }
}

QPointF MVSSRenderer::coord2pix(int m, double t, double val) const
{
    long M = clips.N1();
    int clip_size = clips.N2();
    double margin_left = 20, margin_right = 20;
    double margin_top = 20, margin_bottom = 20;
    /*
    double max_width = 300;
    if (q->width() - margin_left - margin_right > max_width) {
        double diff = (q->width() - margin_left - margin_right) - max_width;
        margin_left += diff / 2;
        margin_right += diff / 2;
    }
    */
    QRectF rect(margin_left, margin_top, W - margin_left - margin_right, H - margin_top - margin_bottom);
    double pctx = (t + 0.5) / clip_size;
    double pcty = (m + 0.5) / M - val / M * amplitude_factor;
    return QPointF(rect.left() + pctx * rect.width(), rect.top() + pcty * rect.height());
}

MVSpikeSprayPanelControl::MVSpikeSprayPanelControl(QObject* parent)
    : QObject(parent)
    , d(new MVSpikeSprayPanelControlPrivate(this))
{
    d->renderThread=new QThread;
}

MVSpikeSprayPanelControl::~MVSpikeSprayPanelControl()
{
    d->renderThread->terminate();
    if (d->renderer)
        d->renderer->deleteLater();
    d->renderThread->deleteLater();
}

double MVSpikeSprayPanelControl::amplitude() const
{
    return d->amplitude;
}

void MVSpikeSprayPanelControl::setAmplitude(double amplitude)
{
    if (d->amplitude == amplitude)
        return;
    d->amplitude = amplitude;
    emit amplitudeChanged(amplitude);
    updateRender();
}

double MVSpikeSprayPanelControl::brightness() const
{
    return d->brightness;
}

void MVSpikeSprayPanelControl::setBrightness(double brightness)
{
    if (d->brightness == brightness)
        return;
    d->brightness = brightness;
    emit brightnessChanged(brightness);
    updateRender();
}

double MVSpikeSprayPanelControl::weight() const
{
    return d->weight;
}

void MVSpikeSprayPanelControl::setWeight(double weight)
{
    if (d->weight == weight)
        return;
    d->weight = weight;
    emit weightChanged(weight);
    updateRender();
}

bool MVSpikeSprayPanelControl::legendVisible() const
{
    return d->legendVisible;
}

void MVSpikeSprayPanelControl::setLegendVisible(bool legendVisible)
{
    if (d->legendVisible == legendVisible)
        return;
    d->legendVisible = legendVisible;
    emit legendVisibleChanged(legendVisible);
    update();
}

QSet<int> MVSpikeSprayPanelControl::labels() const
{
    return d->labels;
}

void MVSpikeSprayPanelControl::paint(QPainter* painter, const QRectF& rect)
{
    if (d->needsRerender)
        rerender();
    if (!d->render.isNull())
        painter->drawImage(rect, d->render);

    painter->save();
    paintLegend(painter, rect);
    painter->restore();
}

void MVSpikeSprayPanelControl::paintLegend(QPainter* painter, const QRectF& rect)
{
    if (!legendVisible())
        return;

    double W = rect.width();
    double spacing = 6;
    double margin = 10;
    double text_height = qBound(12.0, W * 1.0 / 10, 25.0);
    double y0 = rect.top() + margin;
    QFont font = painter->font();
    font.setPixelSize(text_height - 1);
    painter->setFont(font);

    QSet<int> labels_used = labels();
    QList<int> list = labels_used.toList();
    qSort(list);
    QPen pen = painter->pen();
    for (int i = 0; i < list.count(); i++) {
        QRectF r(rect.left(), y0, W - margin, text_height);
        QString str = QString("%1").arg(list[i]);
        pen.setColor(labelColors().at(list[i]));
        painter->setPen(pen);
        painter->drawText(r, Qt::AlignRight, str);
        y0 += text_height + spacing;
    }
}

void MVSpikeSprayPanelControl::setLabels(const QSet<int>& labels)
{
    if (d->labels == labels)
        return;
    d->labels = labels;
    emit labelsChanged(labels);
    update();
}

const QList<QColor>& MVSpikeSprayPanelControl::labelColors() const
{
    return d->labelColors;
}

void MVSpikeSprayPanelControl::setClips(Mda* X)
{
    if (X == d->clipsToRender)
        return;
    d->clipsToRender = X;
    updateRender();
}

void MVSpikeSprayPanelControl::setRenderLabels(const QVector<int>& X)
{
    if (X == d->renderLabels)
        return;
    d->renderLabels = X;
    updateRender();
}

int MVSpikeSprayPanelControl::progress() const
{
    return d->progress;
}

int MVSpikeSprayPanelControl::allocationProgress() const
{
    return d->allocProgress;
}

void MVSpikeSprayPanelControl::setLabelColors(const QList<QColor>& labelColors)
{
    if (labelColors == d->labelColors)
        return;
    d->labelColors = labelColors;
    emit labelColorsChanged(labelColors);
    update();
}

void MVSpikeSprayPanelControl::update(int progress)
{
    if (d->renderer) {
        d->render = d->renderer->resultImage();
        emit renderAvailable();
    }
    if (progress != -1) {
        d->progress = progress;
        emit progressChanged(progress);
    }
}

void MVSpikeSprayPanelControl::updateRender()
{
    d->needsRerender = true;
    update();
}

void MVSpikeSprayPanelControl::rerender()
{
    d->needsRerender = false;
    if (!d->clipsToRender) {
        return;
    }
    if (d->clipsToRender->N3() != d->renderLabels.count()) {
        qWarning() << "Number of clips to render does not match the number of labels to render" << d->clipsToRender->N3() << d->renderLabels.count();
        return;
    }


    if (!amplitude()) {
        double maxval = qMax(qAbs(d->clipsToRender->minimum()), qAbs(d->clipsToRender->maximum()));
        if (maxval)
            d->amplitude = 1.5 / maxval;
    }

    int K = MLCompute::max(d->renderLabels);
    if (!K) {
        return;
    }
    QVector<int> counts(K + 1, 0);
    QList<long> inds;
    for (long i = 0; i < d->renderLabels.count(); i++) {
        int label0 = d->renderLabels[i];
        if (label0 >= 0) {
            if (this->d->labels.contains(label0)) {
                counts[label0]++;
                inds << i;
            }
        }
    }
    QVector<int> alphas(K + 1);
    for (int k = 0; k <= K; k++) {
        if ((counts[k]) && (weight())) {
            alphas[k] = (int)(255.0 / counts[k] * 2 * weight());
            alphas[k] = qBound(10, alphas[k], 255);
        }
        else
            alphas[k] = 255;
    }

    if (d->renderer) {
        d->renderer->requestInterruption();
        d->renderer->wait();
        d->renderer->deleteLater();
        d->renderer = nullptr;
    }
    d->renderer = new MVSSRenderer;
    if (!d->renderThread->isRunning())
        d->renderThread->start();

    long M = d->clipsToRender->N1();
    long T = d->clipsToRender->N2();
    // defer copying data until we're in a worker thread to keep GUI responsive
    d->renderer->allocator = { d->clipsToRender, M, T, inds };
    d->renderer->allocator.progressFunction = [this](int progress) {
        emit d->renderer->allocateProgress(progress);
    };

    d->renderer->colors.clear();
    for (long j = 0; j < inds.count(); j++) {
        int label0 = d->renderLabels[inds[j]];
        QColor col = d->labelColors[label0];
        col = brighten(col, brightness());
        if (label0 >= 0) {
            col.setAlpha(alphas[label0]);
        }
        d->renderer->colors << col;
    }

    d->renderer->amplitude_factor = amplitude();
    d->renderer->W = T * 4;
    d->renderer->H = 1500;
    connect(d->renderer, SIGNAL(imageUpdated(int)), this, SLOT(update(int)));
    connect(d->renderer, SIGNAL(allocateProgress(int)), this, SLOT(updateAllocProgress(int)));
    d->renderer->moveToThread(d->renderThread);
    QMetaObject::invokeMethod(d->renderer, "render", Qt::QueuedConnection);
}

void MVSpikeSprayPanelControl::updateAllocProgress(int progress)
{
    if (d->allocProgress == progress)
        return;
    d->allocProgress = progress;
    emit allocationProgressChanged(d->allocProgress);
}

void MVSSRenderer::replaceImage(const QImage& img)
{
    QMutexLocker locker(&image_in_progress_mutex);
    image_in_progress = img;
}

QImage MVSSRenderer::resultImage() const
{
    QMutexLocker locker(&image_in_progress_mutex);
    return image_in_progress;
}

void MVSSRenderer::wait()
{
    m_condMutex.lock();
    while (m_processing)
        m_cond.wait(&m_condMutex);
    m_condMutex.unlock();
}

void MVSSRenderer::release()
{
    m_condMutex.lock();
    m_processing = false;
    m_cond.wakeAll();
    m_condMutex.unlock();
}

Mda MVSSRenderer::ClipsAllocator::allocate(const std::function<bool()>& breakFunc)
{
    Mda result(M, T, inds.count());
    for (long j = 0; j < inds.count(); j++) {
        for (long t = 0; t < T; t++) {
            for (long m = 0; m < M; m++) {
                result.setValue(source->value(m, t, inds[j]), m, t, j);
            }
        }
        if (breakFunc && breakFunc())
            return result;
        if (progressFunction)
            progressFunction(j * 100 / inds.count());
    }
    if (progressFunction)
        progressFunction(100);
    return result;
}
