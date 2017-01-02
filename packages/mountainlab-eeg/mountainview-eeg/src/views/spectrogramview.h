/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef SPECTROGRAMVIEW_H
#define SPECTROGRAMVIEW_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvabstractviewfactory.h"
#include "mvtimeseriesviewbase.h"

class SpectrogramViewPrivate;
class SpectrogramView : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class SpectrogramViewPrivate;
    SpectrogramView(MVAbstractContext* context);
    virtual ~SpectrogramView();

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
    void slot_brightness_slider_changed(int val);

private:
    SpectrogramViewPrivate* d;
};

class SpectrogramDataFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    SpectrogramDataFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
};

#endif // SPECTROGRAMVIEW_H
