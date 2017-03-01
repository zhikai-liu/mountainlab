/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef SSLVTIMESPANVIEW_H
#define SSLVTIMESPANVIEW_H

#include <QWidget>
#include <diskreadmda.h>
#include <paintlayer.h>
#include "mvabstractviewfactory.h"
#include "sslvtimeseriesviewbase.h"

class SSLVTimeSpanViewPrivate;
class SSLVTimeSpanView : public SSLVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class SSLVTimeSpanViewPrivate;
    SSLVTimeSpanView(MVAbstractContext* context);
    virtual ~SSLVTimeSpanView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setClustersToShow(const QList<int>& clusters_to_show);

    void paintContent(QPainter* painter);

protected:
    void mouseMoveEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void resizeEvent(QResizeEvent* evt);

private slots:
    void slot_update_image();

private:
    SSLVTimeSpanViewPrivate* d;
};

class TimeSpanImageCalculator;
class TimeSpanContentLayer : public PaintLayer {
    Q_OBJECT
public:
    TimeSpanContentLayer();
    virtual ~TimeSpanContentLayer();

    void paint(QPainter* painter) Q_DECL_OVERRIDE;
    void setWindowSize(QSize size) Q_DECL_OVERRIDE;

    TimeSpanImageCalculator* calculator;

    //void updateImage();

    QImage output_image;

private slots:
    void slot_calculator_finished();
};

#endif // SSLVTIMESPANVIEW
