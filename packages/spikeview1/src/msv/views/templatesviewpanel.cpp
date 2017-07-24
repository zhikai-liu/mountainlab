/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "templatesviewpanel.h"
#include "mlcommon.h"

#include <QThread>

class TemplatesViewDataRenderer : public QThread {
public:
    void run();

    void setup_electrode_boxes(double W, double H);

    //input
    QSize window_size;
    ElectrodeGeometry electrode_geometry;
    Mda32 template0;
    int top_section_height=0;
    int bottom_section_height=0;
    double vertical_scale_factor = 1;
    QList<QColor> channel_colors;

    //output
    QMap<int,QRectF> electrode_boxes;
    QImage image;

    QPointF coord2pix(int m, int t, double val);
};

class TemplatesViewPanelPrivate {
public:
    TemplatesViewPanel* q;
    Mda32 m_template;
    ElectrodeGeometry m_electrode_geometry;
    QList<QColor> m_channel_colors;
    int m_clip_size = 0;
    double m_vertical_scale_factor = 1;
    bool m_current = false;
    bool m_selected = false;
    QMap<QString, QColor> m_colors;
    QString m_title;
    double m_bottom_section_height = 0;
    double m_firing_rate_disk_diameter = 0;
    bool m_draw_ellipses = false;
    bool m_draw_disks = false;
    QMap<int,QRectF> m_electrode_boxes;

    bool m_data_render_needed=false;
    TemplatesViewDataRenderer m_data_renderer;

    QPointF coord2pix(int m, int t, double val);
};

TemplatesViewPanel::TemplatesViewPanel()
{
    d = new TemplatesViewPanelPrivate;
    d->q = this;

    connect(&d->m_data_renderer,SIGNAL(finished()),this,SIGNAL(repaintNeeded()));
}

TemplatesViewPanel::~TemplatesViewPanel()
{
    delete d;
}

void TemplatesViewPanel::setTemplate(const Mda32& X)
{
    d->m_template = X;
    d->m_data_render_needed=true;
    d->m_clip_size=d->m_template.N2();
}

void TemplatesViewPanel::setElectrodeGeometry(const ElectrodeGeometry& geom)
{
    d->m_electrode_geometry = geom;
    d->m_data_render_needed=true;
}

void TemplatesViewPanel::setVerticalScaleFactor(double factor)
{
    d->m_vertical_scale_factor = factor;
    d->m_data_render_needed=true;
    emit this->repaintNeeded();
}

void TemplatesViewPanel::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
    d->m_data_render_needed=true;
}

void TemplatesViewPanel::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
    d->m_data_render_needed=true;
}

void TemplatesViewPanel::setCurrent(bool val)
{
    d->m_current = val;
}

void TemplatesViewPanel::setSelected(bool val)
{
    d->m_selected = val;
}

void TemplatesViewPanel::setTitle(const QString& txt)
{
    d->m_title = txt;
}

void TemplatesViewPanel::setFiringRateDiskDiameter(double val)
{
    d->m_firing_rate_disk_diameter = val;
}

void TemplatesViewPanel::paint(QPainter* painter)
{
    //qDebug() << "TemplatesViewPanel::paint" << "+++++++++++++++++++++++++++++";

    painter->setRenderHint(QPainter::Antialiasing);

    QSize ss = this->windowSize();
    //QPen pen = painter->pen();

    d->m_bottom_section_height = qMax(20.0, ss.height() * 0.1);

    if (!d->m_draw_disks)
        d->m_bottom_section_height = 0;

    QRect R(0, 0, ss.width(), ss.height());

    //BACKGROUND
    if (!this->exportMode()) {
        if (d->m_current) {
            painter->fillRect(R, d->m_colors["view_background_highlighted"]);
        }
        else if (d->m_selected) {
            painter->fillRect(R, d->m_colors["view_background_selected"]);
        }
        //else if (d->m_hovered) {
        //    painter->fillRect(R, d->m_colors["view_background_hovered"]);
        //}
        else {
            painter->fillRect(R, d->m_colors["view_background"]);
        }
    }

    if (!this->exportMode()) {
        //FRAME
        if (d->m_selected) {
            painter->setPen(QPen(d->m_colors["view_frame_selected"], 1));
        }
        else {
            painter->setPen(QPen(d->m_colors["view_frame"], 1));
        }
        painter->drawRect(R);
    }

    //TOP SECTION
    {
        QString txt = d->m_title;
        QFont fnt = this->font();
        QRectF R(5, 3, ss.width() - 10, fnt.pixelSize());
        //fnt.setPixelSize(qMin(16.0, qMax(10.0, qMin(d->m_top_section_height, ss.width() * 1.0) - 4)));
        //fnt.setPixelSize(14);
        painter->setFont(fnt);

        QPen pen = painter->pen();
        pen.setColor(d->m_colors["cluster_label"]);
        painter->setPen(pen);
        painter->drawText(R, Qt::AlignLeft | Qt::AlignVCenter, txt);
    }

    //BOTTOM SECTION
    if (d->m_bottom_section_height) {
        if (d->m_firing_rate_disk_diameter) {
            QPen pen_hold = painter->pen();
            QBrush brush_hold = painter->brush();
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(d->m_colors["firing_rate_disk"]));
            QRectF R(0, ss.height() - d->m_bottom_section_height, ss.width(), d->m_bottom_section_height);
            double tmp = qMin(R.width(), R.height());
            double rad = tmp * d->m_firing_rate_disk_diameter / 2 * 0.9;
            painter->drawEllipse(R.center(), rad, rad);
            painter->setPen(pen_hold);
            painter->setBrush(brush_hold);
        }
    }

    if ((d->m_data_render_needed)||(ss!=d->m_data_renderer.window_size)) {
        if (d->m_data_renderer.isRunning()) {
            d->m_data_renderer.requestInterruption();
            d->m_data_renderer.wait();
        }
        d->m_data_render_needed=false;
        d->m_data_renderer.window_size=ss;
        d->m_data_renderer.top_section_height=d->q->font().pixelSize();
        d->m_data_renderer.bottom_section_height=d->m_bottom_section_height;
        d->m_data_renderer.electrode_geometry=d->m_electrode_geometry;
        d->m_data_renderer.template0=d->m_template;
        d->m_data_renderer.vertical_scale_factor=d->m_vertical_scale_factor;
        d->m_data_renderer.channel_colors=d->m_channel_colors;
        d->m_data_renderer.start();
    }
    else {
        if (!d->m_data_renderer.isRunning()) {
            d->m_electrode_boxes=d->m_data_renderer.electrode_boxes;
            painter->drawImage(0,0,d->m_data_renderer.image);
        }
    }
    return;

}

double estimate_spacing(const QList<QVector<double> >& coords)
{
    QVector<double> vals;
    for (int m = 0; m < coords.length(); m++) {
        QVector<double> dists;
        for (int i = 0; i < coords.length(); i++) {
            if (i != m) {
                double dx = coords[i].value(0) - coords[m].value(0);
                double dy = coords[i].value(1) - coords[m].value(1);
                double dist = sqrt(dx * dx + dy * dy);
                dists << dist;
            }
        }
        vals << MLCompute::min(dists);
    }
    return MLCompute::mean(vals);
}

void TemplatesViewDataRenderer::setup_electrode_boxes(double W, double H)
{
    electrode_boxes.clear();

    double W1 = W;
    double H1 = H - top_section_height - bottom_section_height;

    QSet<int> electrodes_to_show;
    electrodes_to_show << 0 << 1 << 2 << 3 << 4 << 5;
    electrodes_to_show << 0;

    QList<QVector<double> > coords = electrode_geometry.coordinates;
    if (coords.isEmpty()) {
        int M = template0.N1();
        int num_rows = qMax(1, (int)sqrt(M));
        int num_cols = ceil(M * 1.0 / num_rows);
        for (int m = 0; m < M; m++) {
            if (MLUtil::threadInterruptRequested())
                return;
            QVector<double> tmp;
            tmp << (m % num_cols) + 0.5 * ((m / num_cols) % 2) << m / num_cols;
            coords << tmp;
        }
    }
    if (coords.isEmpty())
        return;

    int D = coords[0].count();
    QVector<double> mins(D), maxs(D);
    for (int d=0; d<D; d++) {
        mins[d]=maxs[d]=0;
        bool first=true;
        for (int m = 0; m < coords.count(); m++) {
            if (electrodes_to_show.contains(m)) {
                if (first) {
                    mins[d]=coords[m][d];
                    maxs[d]=coords[m][d];
                    first=false;
                }
                else {
                    mins[d] = qMin(mins[d], coords[m][d]);
                    maxs[d] = qMax(maxs[d], coords[m][d]);
                }
            }
        }
    }

    double spacing = estimate_spacing(coords);
    spacing = spacing * 0.75;

    //double W0 = maxs.value(0) - mins.value(0);
    //double H0 = maxs.value(1) - mins.value(1);
    double W0 = maxs.value(1) - mins.value(1);
    double H0 = maxs.value(0) - mins.value(0);

    double W0_padded = W0 + spacing * 4 / 3;
    double H0_padded = H0 + spacing * 4 / 3;

    double hscale_factor = 1;
    double vscale_factor = 1;

    if (W0_padded * H1 > W1 * H0_padded) {
        //limited by width
        if (W0_padded) {
            hscale_factor = W1 / W0_padded;
            vscale_factor = W1 / W0_padded;
        }
    }
    else {
        if (H0_padded)
            vscale_factor = H1 / H0_padded;
        if (W0_padded)
            hscale_factor = W1 / W0_padded;
    }

    double offset_x = (W1 - W0 * hscale_factor) / 2;
    double offset_y = top_section_height + (H1 - H0 * vscale_factor) / 2;
    for (int m = 0; m < coords.count(); m++) {
        if (electrodes_to_show.contains(m)) {
            if (MLUtil::threadInterruptRequested())
                return;
            QVector<double> c = coords[m];
            //double x0 = offset_x + (c.value(0) - mins.value(0)) * hscale_factor;
            //double y0 = offset_y + (c.value(1) - mins.value(1)) * vscale_factor;
            double x0 = offset_x + (c.value(1) - mins.value(1)) * hscale_factor;
            double y0 = offset_y + (c.value(0) - mins.value(0)) * vscale_factor;
            double radx = spacing * hscale_factor / 2;
            double rady = spacing * vscale_factor / 3.5;
            electrode_boxes[m]=QRectF(x0 - radx, y0 - rady, radx * 2, rady * 2);
        }
    }
}

QPointF TemplatesViewDataRenderer::coord2pix(int m, int t, double val)
{
    if (!electrode_boxes.contains(m)) {
        return QPointF(-1,-1);
    }
    QRectF R = electrode_boxes.value(m);
    QPointF Rcenter = R.center();
    double pctx = t * 1.0 / template0.N2();
    double x0 = R.left() + (R.width()) * pctx;
    double y0 = Rcenter.y() - val * R.height() / 2 * vertical_scale_factor;
    return QPointF(x0, y0);
}

QPointF TemplatesViewPanelPrivate::coord2pix(int m, int t, double val)
{
    if (!m_electrode_boxes.contains(m)) {
        return QPointF(-1,-1);
    }
    QRectF R = m_electrode_boxes.value(m);
    QPointF Rcenter = R.center();
    double pctx = t * 1.0 / m_clip_size;
    double x0 = R.left() + (R.width()) * pctx;
    double y0 = Rcenter.y() - val * R.height() / 2 * m_vertical_scale_factor;
    return QPointF(x0, y0);
}

void TemplatesViewDataRenderer::run()
{
    qDebug() << "STARTING" << QThread::currentThreadId();
    image=QImage(window_size,QImage::Format_ARGB32);
    image.fill(QColor(0,0,0,0)); //transparent

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    //SETUP ELECTRODE LOCATIONS
    setup_electrode_boxes(window_size.width(),window_size.height());

    //ELECTRODES AND WAVEFORMS
    int M = template0.N1();
    int T = template0.N2();
    for (int m = 0; m < M; m++) {
        if (MLUtil::threadInterruptRequested())
            return;
        QPainterPath path;
        for (int t = 0; t < T; t++) {
            double val = template0.value(m, t);
            QPointF pt = coord2pix(m, t, val);
            if (t == 0)
                path.moveTo(pt);
            else
                path.lineTo(pt);
        }
        QPen pen;
        pen.setColor(channel_colors.value(m, Qt::black));
        pen.setWidth(2);
        //if (this->exportMode())
        //    pen.setWidth(6);
        painter.strokePath(path, pen);

        if (electrode_boxes.contains(m)) {
            QRectF box = electrode_boxes.value(m);
            pen.setColor(QColor(180, 200, 200));
            pen.setWidth(1);
            painter.setPen(pen);
            //if (d->m_draw_ellipses)
            if (true)
                painter.drawEllipse(box);
        }
    }

    qDebug() << "ENDING" << QThread::currentThreadId();
}
