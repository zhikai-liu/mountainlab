/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef CorrelogramWidget_H
#define CorrelogramWidget_H

#include "mvabstractviewfactory.h"
#include "mvhistogramgrid.h"

#include <QWidget>

enum CrossCorrelogramMode3 {
    Undefined3,
    All_Auto_Correlograms3,
    Selected_Auto_Correlograms3,
    Cross_Correlograms3,
    Matrix_Of_Cross_Correlograms3,
    Selected_Cross_Correlograms3
};

struct CrossCorrelogramOptions3 {
    CrossCorrelogramMode3 mode = Undefined3;
    QList<int> ks;

    QJsonObject toJsonObject();
    void fromJsonObject(const QJsonObject& X);
};

class CorrelogramWidgetPrivate;
class CorrelogramWidget : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class CorrelogramWidgetPrivate;
    CorrelogramWidget(MVAbstractContext* context);
    virtual ~CorrelogramWidget();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setOptions(CrossCorrelogramOptions3 opts);
    void setTimeScaleMode(HistogramView::TimeScaleMode mode);
    HistogramView::TimeScaleMode timeScaleMode() const;

    QJsonObject exportStaticView() Q_DECL_OVERRIDE;
    void loadStaticView(const QJsonObject& X) Q_DECL_OVERRIDE;

signals:
private slots:
    void slot_log_time_scale();
    void slot_warning();
    void slot_export_static_view();

private:
    CorrelogramWidgetPrivate* d;
};

#endif // CorrelogramWidget_H
