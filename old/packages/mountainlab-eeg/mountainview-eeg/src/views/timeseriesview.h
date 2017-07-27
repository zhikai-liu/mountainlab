/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef TIMESERIESVIEW_H
#define TIMESERIESVIEW_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvabstractviewfactory.h"
#include "mvtimeseriesviewbase.h"

class TimeseriesViewPrivate;
class TimeseriesView : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class TimeseriesViewPrivate;
    TimeseriesView(MVAbstractContext* context);
    virtual ~TimeseriesView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void paintContent(QPainter* painter);

    void setAmplitudeFactor(double factor); // display range will be between -1/factor and 1/factor, but not clipped (thus channel plots may overlap)
    void autoSetAmplitudeFactor();
    void autoSetAmplitudeFactorWithinTimeRange();

    double amplitudeFactor() const;

    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private slots:
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    TimeseriesViewPrivate* d;
};

class TimeseriesDataFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    TimeseriesDataFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
};

#endif // TIMESERIESVIEW_H
