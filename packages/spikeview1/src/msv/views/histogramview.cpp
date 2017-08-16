#include "histogramview.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QThread>
#include "mvutils.h"
#include "mlcommon.h"
#include <QMenu>
#include "mlvector.h"
#include "svcontext.h" //temporary dependency

struct BinInfo {
    double bin_min = -1;
    double bin_max = 1;
    int num_bins = 200;
};

struct HistogramViewCoord2PixOpts {
    int margin_bottom, margin_top, margin_left, margin_right;
    bool draw_caption;
    int num_bins;
    MVRange xrange;
    double max_bin_density;
    int W, H;
};

class HistogramViewDataRenderer : public QThread {
public:
    void run();

    //input
    HistogramViewCoord2PixOpts coord2pix_opts;
    QSize window_size;
    MLVector<double> mm_bin_lefts;
    MLVector<double> mm_bin_rights;
    MLVector<double> mm_bin_densities;
    MLVector<double> mm_second_bin_densities;
    QColor mm_fill_color;
    QColor mm_line_color;

    //output
    QImage image;
};

class BinCounter : public QThread {
public:
    void run();

    //input
    MLVector<double> data;
    MLVector<double> second_data;
    MLVector<double> bin_lefts;
    MLVector<double> bin_rights;

    //output
    MLVector<int> bin_counts;
    MLVector<double> bin_densities;
    MLVector<int> second_bin_counts;
    MLVector<double> second_bin_densities;
    double max_bin_density;
};

class HistogramViewPrivate {
public:
    HistogramView* q;
    MLVector<double> m_data;
    MLVector<double> m_bin_lefts;
    MLVector<double> m_bin_rights;
    MLVector<int> m_bin_counts;
    MLVector<double> m_bin_densities;
    MLVector<double> m_second_data;
    MLVector<int> m_second_bin_counts;
    MLVector<double> m_second_bin_densities;

    HistogramViewCoord2PixOpts m_coord2pix_opts;

    HistogramViewDataRenderer m_data_renderer;
    bool m_data_render_needed = false;
    QImage m_current_data_image;

    BinCounter m_bin_counter;

    BinInfo m_bin_info;
    double m_max_bin_density = 0;
    double m_time_constant = 30;
    HistogramView::TimeScaleMode m_time_scale_mode = HistogramView::Uniform;

    bool m_update_bin_counts_required = false;
    QColor m_fill_color = QColor(120, 120, 150);
    QColor m_line_color = QColor(100, 100, 130);
    int m_hovered_bin_index = -1;
    int m_margin_left = 5, m_margin_right = 5, m_margin_top = 5, m_margin_bottom = 5;
    bool m_draw_caption = false;
    QString m_title;
    QString m_caption;
    QMap<QString, QColor> m_colors;
    bool m_hovered = false;
    bool m_current = false;
    bool m_selected = false;
    bool m_draw_vertical_axis_at_zero = false;
    QList<double> m_vertical_lines;
    QList<double> m_tick_marks;
    MVRange m_xrange = MVRange(0, 0);

    static QPointF coord2pix(QPointF pt, HistogramViewCoord2PixOpts ooo);
    static QPointF pix2coord(QPointF pt, HistogramViewCoord2PixOpts ooo);
    int get_bin_index_at(QPointF pt);
    void export_image();
    void do_paint(QPainter& painter, int W, int H);

    //double transform1(double t); //for example log, or identity
    //double transform2(double x); //for example exp, or identity

    void set_bins();
};

HistogramView::HistogramView(QWidget* parent)
    : RenderableWidget(parent)
{
    d = new HistogramViewPrivate;
    d->q = this;

    connect(&d->m_data_renderer, SIGNAL(finished()), this, SLOT(update()));
    connect(&d->m_bin_counter, SIGNAL(finished()), this, SLOT(slot_bin_counter_finished()));

    this->setMouseTracking(true);
    //this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_context_menu(QPoint)));
}

HistogramView::~HistogramView()
{
    d->m_data_renderer.requestInterruption();
    d->m_data_renderer.wait();

    d->m_bin_counter.requestInterruption();
    d->m_bin_counter.wait();

    delete d;
}

void HistogramView::setData(const MLVector<double>& values)
{
    d->m_data = values;
    d->m_update_bin_counts_required = true;
    update();
}

void HistogramView::appendData(const MLVector<double>& values)
{
    d->m_data.append(values);
    d->m_update_bin_counts_required = true;
    update();
}

void HistogramView::setSecondData(const MLVector<double>& values)
{
    d->m_second_data = values;
    d->m_update_bin_counts_required = true;
    update();
}

void HistogramView::setBinInfo(double bin_min, double bin_max, int num_bins)
{
    d->m_bin_info.bin_min = bin_min;
    d->m_bin_info.bin_max = bin_max;
    d->m_bin_info.num_bins = num_bins;
    d->m_xrange = MVRange(bin_min, bin_max);
    d->set_bins();
}

void HistogramViewPrivate::set_bins()
{
    double bin_min = m_bin_info.bin_min;
    double bin_max = m_bin_info.bin_max;
    double num_bins = m_bin_info.num_bins;

    int N = num_bins;

    m_bin_lefts = MLVector<double>(N);
    m_bin_rights = MLVector<double>(N);
    m_bin_counts = MLVector<int>(N);
    m_bin_densities = MLVector<double>(N);

    m_second_bin_counts = MLVector<int>(N);
    m_second_bin_densities = MLVector<double>(N);

    if (N <= 2)
        return;
    //double spacing = (transform1(bin_max) - transform1(bin_min)) / N;
    double spacing = (bin_max - bin_min) / N;
    for (int i = 0; i < N; i++) {
        //m_bin_lefts[i] = transform2(transform1(bin_min) + i * spacing);
        //m_bin_rights[i] = transform2(transform1(bin_min) + (i + 1) * spacing);
        m_bin_lefts[i] = bin_min + i * spacing;
        m_bin_rights[i] = bin_min + (i + 1) * spacing;
    }

    m_update_bin_counts_required = true;
    q->update();
}

void HistogramView::setFillColor(const QColor& col)
{
    d->m_fill_color = col;
    update();
}

void HistogramView::setLineColor(const QColor& col)
{
    d->m_line_color = col;
    update();
}

void HistogramView::setTitle(const QString& title)
{
    d->m_title = title;
    update();
}

void HistogramView::setCaption(const QString& caption)
{
    d->m_caption = caption;
    update();
}

void HistogramView::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
    update();
}

void HistogramView::setTimeScaleMode(HistogramView::TimeScaleMode mode)
{
    if (d->m_time_scale_mode == mode)
        return;
    d->m_time_scale_mode = mode;
    d->m_update_bin_counts_required = true;
    d->set_bins();
}

void HistogramView::setTimeConstant(double timepoints)
{
    if (d->m_time_constant == timepoints)
        return;
    d->m_time_constant = timepoints;
    d->m_update_bin_counts_required = true;
    d->set_bins();
}

MVRange HistogramView::xRange() const
{
    return d->m_xrange;
}

void HistogramView::setXRange(MVRange range)
{
    if (range == d->m_xrange)
        return;
    d->m_xrange = range;
    update();
}

#include "mlcommon.h"
#include "mvthreadmanager.h"
void HistogramView::autoCenterXRange()
{
    double mean_value = mean(d->m_data);
    MVRange xrange = this->xRange();
    double center1 = (xrange.min + xrange.max) / 2;
    xrange = xrange + (mean_value - center1);
    this->setXRange(xrange);
}

void HistogramView::setDrawVerticalAxisAtZero(bool val)
{
    if (d->m_draw_vertical_axis_at_zero == val)
        return;
    d->m_draw_vertical_axis_at_zero = val;
    update();
}

void HistogramView::setVerticalLines(const QList<double>& vals)
{
    d->m_vertical_lines = vals;
    update();
}

void HistogramView::setTickMarks(const QList<double>& vals)
{
    d->m_tick_marks = vals;
    update();
}

void HistogramView::setCurrent(bool val)
{
    if (d->m_current != val) {
        d->m_current = val;
        this->update();
    }
}
void HistogramView::setSelected(bool val)
{
    if (d->m_selected != val) {
        d->m_selected = val;
        this->update();
    }
}

QImage HistogramView::renderImage(int W, int H)
{
    QImage ret = QImage(W, H, QImage::Format_RGB32);
    ret.fill(Qt::white);
    QPainter painter(&ret);
    painter.setFont(this->font());

    bool selected = d->m_selected;
    bool hovered = d->m_hovered;
    bool current = d->m_current;
    int hovered_bin_index = d->m_hovered_bin_index;

    d->m_selected = false;
    d->m_hovered = false;
    d->m_hovered_bin_index = -1;
    d->m_current = false;

    d->do_paint(painter, W, H);

    d->m_selected = selected;
    d->m_hovered = hovered;
    d->m_hovered_bin_index = hovered_bin_index;
    d->m_current = current;

    return ret;
}

QRectF make_rect2(QPointF p1, QPointF p2)
{
    double x = qMin(p1.x(), p2.x());
    double y = qMin(p1.y(), p2.y());
    double w = qAbs(p2.x() - p1.x());
    double h = qAbs(p2.y() - p1.y());
    return QRectF(x, y, w, h);
}

QColor lighten2(QColor col, int dr, int dg, int db)
{
    int r = col.red() + dr;
    if (r > 255)
        r = 255;
    if (r < 0)
        r = 0;
    int g = col.green() + dg;
    if (g > 255)
        g = 255;
    if (g < 0)
        g = 0;
    int b = col.blue() + db;
    if (b > 255)
        b = 255;
    if (b < 0)
        b = 0;
    return QColor(r, g, b);
}

void HistogramView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);

    d->do_paint(painter, width(), height());
}

void HistogramView::mousePressEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt);
    if (evt->button() == Qt::LeftButton)
        emit clicked(evt->modifiers());
    else if (evt->button() == Qt::RightButton)
        emit rightClicked(evt->modifiers());
    /*
    if ((evt->modifiers() & Qt::ControlModifier) || (evt->modifiers() & Qt::ShiftModifier)) {
        emit control_clicked();
    }
    else {
        emit clicked();
    }
    */

    RenderableWidget::mousePressEvent(evt);
}

void HistogramView::mouseMoveEvent(QMouseEvent* evt)
{
    QPointF pt1 = evt->pos();
    int bin_index = d->get_bin_index_at(pt1);
    if (d->m_hovered_bin_index != bin_index) {
        d->m_hovered_bin_index = bin_index;
        this->update();
    }
    RenderableWidget::mouseMoveEvent(evt);
}

void HistogramView::enterEvent(QEvent* evt)
{
    Q_UNUSED(evt)
    d->m_hovered = true;
    update();
}

void HistogramView::leaveEvent(QEvent* evt)
{
    Q_UNUSED(evt)
    d->m_hovered = false;
    update();
}

void HistogramView::mouseDoubleClickEvent(QMouseEvent* evt)
{
    emit activated(evt->pos());
}

void HistogramView::slot_context_menu(const QPoint& pos)
{
    QMenu M;
    QAction* export_image = M.addAction("Export Histogram Image");
    QAction* export_matrix_image = M.addAction("Export Histogram Matrix Image");
    QAction* selected = M.exec(this->mapToGlobal(pos));
    if (selected == export_image) {
        d->export_image();
    }
    else if (selected == export_matrix_image) {
        emit this->signalExportHistogramMatrixImage();
    }
}

void HistogramView::slot_bin_counter_finished()
{
    d->m_bin_counts = d->m_bin_counter.bin_counts;
    d->m_second_bin_counts = d->m_bin_counter.second_bin_counts;
    d->m_bin_densities = d->m_bin_counter.bin_densities;
    d->m_second_bin_densities = d->m_bin_counter.second_bin_densities;
    d->m_max_bin_density = d->m_bin_counter.max_bin_density;
    d->m_data_render_needed = true;
    update();
}

QPointF HistogramViewPrivate::coord2pix(QPointF pt, HistogramViewCoord2PixOpts ooo)
{
    int caption_height = 9;
    if (!ooo.draw_caption)
        caption_height = 0;

    if (ooo.num_bins < 2) {
        return QPointF(0, 0);
    }
    if ((!ooo.W) || (!ooo.H))
        return QPointF(0, 0);
    if (ooo.W <= ooo.margin_left + ooo.margin_right + 5)
        return QPointF(0, 0);
    if (ooo.H <= ooo.margin_top + ooo.margin_bottom + caption_height + 5)
        return QPointF(0, 0);

    double xmin = ooo.xrange.min;
    double xmax = ooo.xrange.max;
    double ymin = 0;
    double ymax = ooo.max_bin_density;

    double xfrac = 0.5;
    if (xmax > xmin) {
        //xfrac = (transform1(pt.x()) - transform1(xmin)) / (transform1(xmax) - transform1(xmin));
        xfrac = (pt.x() - xmin) / (xmax - xmin);
    }
    double yfrac = 0.5;
    if (ymax > ymin)
        yfrac = (pt.y() - ymin) / (ymax - ymin);

    double x0 = ooo.margin_left + xfrac * (ooo.W - ooo.margin_left - ooo.margin_right);
    double y0 = ooo.H - (ooo.margin_bottom + caption_height + yfrac * (ooo.H - ooo.margin_top - ooo.margin_bottom - caption_height));

    return QPointF(x0, y0);
}

QPointF HistogramViewPrivate::pix2coord(QPointF pt, HistogramViewCoord2PixOpts ooo)
{

    int caption_height = 9;
    if (!ooo.draw_caption)
        caption_height = 0;

    if (ooo.num_bins < 2) {
        return QPointF(0, 0);
    }
    if ((!ooo.W) || (!ooo.H))
        return QPointF(0, 0);
    if (ooo.W <= ooo.margin_left + ooo.margin_right + 5)
        return QPointF(0, 0);
    if (ooo.H <= ooo.margin_top + ooo.margin_bottom + caption_height + 5)
        return QPointF(0, 0);

    double xmin = ooo.xrange.min;
    double xmax = ooo.xrange.max;
    double ymin = 0;
    double ymax = ooo.max_bin_density;

    double xfrac = (pt.x() - ooo.margin_left) / (ooo.W - ooo.margin_left - ooo.margin_right);
    double yfrac = 1 - (pt.y() - ooo.margin_top) / (ooo.H - ooo.margin_top - ooo.margin_bottom - caption_height);

    //double x0 = transform2(transform1(xmin) + xfrac * (transform1(xmax) - transform1(xmin)));
    double x0 = xmin + xfrac * (xmax - xmin);
    double y0 = ymin + yfrac * (ymax - ymin);

    return QPointF(x0, y0);
}

int HistogramViewPrivate::get_bin_index_at(QPointF pt_pix)
{
    int num_bins = m_bin_lefts.count();
    if (num_bins < 2) {
        return -1;
    }
    HistogramViewCoord2PixOpts ooo = m_coord2pix_opts;
    QPointF pt = pix2coord(pt_pix, ooo);
    for (int i = 0; i < num_bins; i++) {
        if ((pt.x() >= m_bin_lefts[i]) && (pt.x() <= m_bin_rights[i])) {
            //if ((0<=pt.y())&&(pt.y()<=m_bin_counts[i])) {
            return i;
            //}
        }
    }
    return -1;
}

void HistogramViewPrivate::export_image()
{
    QImage img = q->renderImage(800, 600);
    user_save_image(img);
}

QColor modify_color_for_second_histogram2(QColor col)
{
    QColor ret = col;
    ret.setGreen(qMin(255, ret.green() + 30)); //more green
    ret = lighten2(ret, -20, -20, -20); //darker
    return ret;
}

MLVector<double> sum(const MLVector<double>& V1, const MLVector<double>& V2)
{
    MLVector<double> ret;
    ret.resize(qMax(V1.size(), V2.size()));
    for (int i = 0; i < ret.count(); i++) {
        ret[i] = V1.value(i) + V2.value(i);
    }
    return ret;
}

void HistogramViewPrivate::do_paint(QPainter& painter, int W, int H)
{
    //d->m_colors["view_background"]=QColor(245,245,245);
    //d->m_colors["view_background_highlighted"]=QColor(250,220,200);
    //d->m_colors["view_background_hovered"]=QColor(240,245,240);

    //	QColor hover_color=QColor(150,150,150,80);
    //	QColor current_color=QColor(150,200,200,80);
    //	QColor hover_current_color=QColor(170,200,200,80);

    int margin_bottom = m_margin_bottom;
    if (q->exportMode())
        margin_bottom = qMax(margin_bottom, H / 10);

    QRect R(0, 0, W, H);

    if (!q->exportMode()) {
        if (m_current) {
            painter.fillRect(R, m_colors["view_background_highlighted"]);
        }
        else if (m_selected) {
            painter.fillRect(R, m_colors["view_background_selected"]);
        }
        else if (m_hovered) {
            painter.fillRect(R, m_colors["view_background_hovered"]);
        }
        else {
            painter.fillRect(R, m_colors["view_background"]);
        }

        if (m_selected) {
            painter.setPen(QPen(m_colors["view_frame_selected"], 2));
        }
        else {
            painter.setPen(QPen(m_colors["view_frame"], 1));
        }
        painter.drawRect(R);
    }

    HistogramViewCoord2PixOpts ooo;
    ooo.draw_caption = m_draw_caption;
    ooo.H = q->height();
    ooo.margin_bottom = m_margin_bottom;
    ooo.margin_left = m_margin_left;
    ooo.margin_right = m_margin_right;
    ooo.margin_top = m_margin_top;
    ooo.max_bin_density = m_max_bin_density;
    ooo.num_bins = m_bin_lefts.count();
    ooo.W = q->width();
    ooo.xrange = m_xrange;
    m_coord2pix_opts = ooo;

    if (m_draw_vertical_axis_at_zero) {
        QPointF pt0 = coord2pix(QPointF(0, 0), ooo);
        QPointF pt1 = coord2pix(QPointF(0, m_max_bin_density), ooo);
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    foreach (double val, m_vertical_lines) {
        QPointF pt0 = coord2pix(QPointF(val, 0), ooo);
        QPointF pt1 = coord2pix(QPointF(val, m_max_bin_density), ooo);
        QPen pen = painter.pen();
        pen.setColor(Qt::gray);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    foreach (double val, m_tick_marks) {
        QPointF pt0 = coord2pix(QPointF(val, 0), ooo);
        QPointF pt1 = pt0;
        pt1.setY(pt1.y() + 5);
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    if (!m_title.isEmpty()) {
        //int text_height = 14;
        QFont font = painter.font();
        QRect R(m_margin_left, 5, W - m_margin_left - m_margin_right, font.pixelSize());

        font.setFamily("Arial");
        //font.setPixelSize(text_height);
        painter.setFont(font);
        painter.setPen(Qt::darkGray);
        painter.drawText(R, m_title, Qt::AlignLeft | Qt::AlignTop);
    }
    if ((!m_caption.isEmpty()) && (m_draw_caption)) {
        int caption_height = 9;
        if (!m_draw_caption)
            caption_height = 0;

        //int text_height = 11;
        QRect R(0, H - margin_bottom - caption_height, W, margin_bottom + caption_height);
        QFont font = painter.font();
        font.setFamily("Arial");
        //font.setPixelSize(text_height);
        painter.setFont(font);
        painter.setPen(Qt::darkGray);
        painter.drawText(R, m_caption, Qt::AlignCenter | Qt::AlignVCenter);
    }

    if ((m_data_render_needed) || (m_update_bin_counts_required) || (m_data_renderer.window_size != QSize(W, H))) {
        if (m_data_renderer.isRunning()) {
            m_data_renderer.requestInterruption();
            m_data_renderer.wait();
        }
        if (m_update_bin_counts_required) {
            m_bin_counter.requestInterruption();
            m_bin_counter.wait();
            m_bin_counter.bin_lefts = m_bin_lefts;
            m_bin_counter.bin_rights = m_bin_rights;
            m_bin_counter.data = m_data;
            m_bin_counter.second_data = m_second_data;
            //MVThreadManager::globalInstance()->addThread(&m_bin_counter);
            m_bin_counter.start();
            m_update_bin_counts_required = false;
        }
        m_data_renderer.coord2pix_opts = ooo;
        m_data_renderer.window_size = QSize(W, H);
        m_data_renderer.mm_bin_lefts = m_bin_lefts;
        m_data_renderer.mm_bin_rights = m_bin_rights;
        m_data_renderer.mm_bin_densities = m_bin_densities;
        m_data_renderer.mm_second_bin_densities = m_second_bin_densities;
        m_data_renderer.mm_fill_color = m_fill_color;
        m_data_renderer.mm_line_color = m_line_color;
        m_data_renderer.start();
        m_data_render_needed = false;
        painter.drawImage(0, 0, m_current_data_image);
    }
    else {
        if (!m_data_renderer.isRunning()) {
            painter.drawImage(0, 0, m_data_renderer.image);
            m_current_data_image = m_data_renderer.image;
        }
        else {
            painter.drawImage(0, 0, m_current_data_image);
        }
    }
}

/*
double HistogramViewPrivate::transform1(double t)
{
    if (m_time_scale_mode == HistogramView::Uniform)
        return t;
    if (t < 0)
        return -transform1(-t); //now we can assume non-negative
    if (t == 0)
        return 0; //now we can assume positive

    switch (m_time_scale_mode) {
    case HistogramView::Uniform:
        return t;
        break;
    case HistogramView::Log:
        if (t < m_time_constant)
            return t;
        else
            return (1 + log(t / m_time_constant)) * m_time_constant;
        break;
    case HistogramView::Cubic:
        double val = t / m_time_constant;
        return exp(log(val) / 3);
    }
    return t;
}
*/

/*
double HistogramViewPrivate::transform2(double x)
{
    if (m_time_scale_mode == HistogramView::Uniform)
        return x;
    if (x < 0)
        return -transform2(-x); //now we can assume non-negative
    if (x == 0)
        return 0; //now we can assume positive

    switch (m_time_scale_mode) {
    case HistogramView::Uniform:
        return x;
        break;
    case HistogramView::Log:
        if (x < m_time_constant)
            return x;
        else
            return m_time_constant * (exp(x / m_time_constant - 1));
        break;
    case HistogramView::Cubic:
        return x * x * x * m_time_constant;
    }
    return x;
}
*/

void HistogramViewDataRenderer::run()
{
    image = QImage(window_size, QImage::Format_ARGB32);
    image.fill(QColor(0, 0, 0, 0));
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    int num_bins = mm_bin_lefts.count();
    if (num_bins < 2)
        return;

    //int W=window_size.width();
    //int H=window_size.height();

    for (int pass = 0; pass <= 2; pass++) {
        MLVector<double> bin_densities;
        QColor col, line_color;
        if (pass == 0) {
            bin_densities = sum(mm_bin_densities, mm_second_bin_densities);
            col = Qt::gray;
            line_color = Qt::gray;
        }
        else if (pass == 1) {
            bin_densities = mm_bin_densities;
            col = mm_fill_color;
            line_color = mm_line_color;
        }
        else if (pass == 2) {
            bin_densities = mm_second_bin_densities;
            col = modify_color_for_second_histogram2(mm_fill_color);
            line_color = modify_color_for_second_histogram2(mm_line_color);
        }

        for (int i = 0; i < num_bins; i++) {
            if (MLUtil::threadInterruptRequested())
                return;
            QPointF pt1 = HistogramViewPrivate::coord2pix(QPointF(mm_bin_lefts[i], 0), coord2pix_opts);
            QPointF pt2 = HistogramViewPrivate::coord2pix(QPointF(mm_bin_rights[i], bin_densities[i]), coord2pix_opts);
            QRectF R = make_rect2(pt1, pt2);
            //if (i == mm_hovered_bin_index)
            //    painter.fillRect(R, lighten2(col, 25, 25, 25));
            //else
            painter.fillRect(R, col);
            painter.setPen(line_color);
            painter.drawRect(R);
        }
    }
}

void BinCounter::run()
{
    int num_bins = bin_lefts.count();
    bin_counts.resize(num_bins);
    second_bin_counts.resize(num_bins);
    bin_densities.resize(num_bins);
    second_bin_densities.resize(num_bins);
    for (int i = 0; i < num_bins; i++) {
        bin_counts[i] = 0;
        second_bin_counts[i] = 0;
        bin_densities[i] = 0;
        second_bin_densities[i] = 0;
    }
    for (int pass = 1; pass <= 2; pass++) {
        MLVector<double> list;
        if (pass == 1) {
            list = data;
        }
        else {
            list = second_data;
        }
        qSort(list);
        if (num_bins < 2)
            return;
        int jj = 0;
        for (int i = 0; i < list.count(); i++) {
            if (MLUtil::threadInterruptRequested())
                return;
            double val = list[i];
            while ((jj + 1 < num_bins) && (bin_rights[jj] < val)) {
                jj++;
            }
            if ((val >= bin_lefts[jj]) && (val <= bin_rights[jj])) {
                if (pass == 1) {
                    bin_counts[jj]++;
                }
                else {
                    second_bin_counts[jj]++;
                }
            }
        }
    }
    for (int i = 0; i < num_bins; i++) {
        double len = bin_rights[i] - bin_lefts[i];
        if (!len)
            len = 1;
        bin_densities[i] = bin_counts[i] / len;
        second_bin_densities[i] = second_bin_counts[i] / len;
    }
    max_bin_density = qMax(max(bin_densities), max(second_bin_densities));
}
