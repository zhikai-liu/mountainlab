#include "viewimageexporter.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QPdfWriter>
#include <QScrollArea>
#include <QSpinBox>
#include <QToolBar>
#include <QVBoxLayout>

namespace {
class PreviewWidget : public QWidget {
public:
    PreviewWidget(QWidget *parent = 0) : QWidget(parent) {
        setBackgroundRole(QPalette::Base);
        setAutoFillBackground(true);
    }
    void setView(MVAbstractView *view) {
        m_view = view;
        resize(m_view->size());
        update();
    }

protected:
    void paintEvent(QPaintEvent *pe) {
        QWidget::paintEvent(pe);
        if (!m_view) return;
        QPainter painter(this);
        painter.save();
        if (m_view->viewFeatures() & MVAbstractView::RenderView)
            m_view->renderView(&painter);
        else
            m_view->render(&painter);
        painter.restore();
    }

private:
    MVAbstractView *m_view;
};

}

ViewImageExporter::ViewImageExporter(QObject *parent) : QObject(parent)
{
}

QDialog *ViewImageExporter::createViewExportDialog(MVAbstractView *view, QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(bbox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), dialog, SLOT(reject()));
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QScrollArea *sarea = new QScrollArea;
    QToolBar *toolBar = new QToolBar;
    QSpinBox *widthBox = new QSpinBox;
    QSpinBox *heightBox = new QSpinBox;
    widthBox->setRange(100, 9999);
    heightBox->setRange(100, 9999);
    widthBox->setValue(view->width());
    heightBox->setValue(view->height());
    toolBar->addWidget(widthBox);
    toolBar->addWidget(heightBox);
    layout->addWidget(toolBar);
    layout->addWidget(sarea);
    layout->addWidget(bbox);
    PreviewWidget *widget = new PreviewWidget;
    widget->setView(view);
    sarea->setWidget(widget);
    sarea->setWidgetResizable(false);
    dialog->resize(800,600);
    connect(widthBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [widget, widthBox, heightBox]{
        widget->resize(widthBox->value(), heightBox->value());
    });
    connect(heightBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [widget, widthBox, heightBox]{
        widget->resize(widthBox->value(), heightBox->value());
    });
    connect(dialog, &QDialog::accepted, [view, widthBox, heightBox]{
        QPdfWriter writer("/tmp/export.pdf");
        writer.setResolution(100);
        writer.setPageSize(QPageSize(QSize(widthBox->value(), heightBox->value())));
//        writer.setPageSize(QPdfWriter::A4);
//        writer.setPageOrientation(QPageLayout::Landscape);
        QPainter painter(&writer);
        view->renderView(&painter);
#if 0
        QImage img(widthBox->value(), heightBox->value(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        view->renderView(&painter);
        img.save("/tmp/export.png", "PNG");
#endif
    });
    return dialog;
}

bool ViewImageExporter::simpleExport(MVAbstractView *view)
{
    return true;
}
