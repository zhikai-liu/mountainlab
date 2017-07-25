/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef CrosscorView_H
#define CrosscorView_H

#include "mvabstractview.h"
#include "mvgridview.h"
#include "mvgridview.h"

#include <mvabstractviewfactory.h>
#include <paintlayer.h>

class CrosscorViewPrivate;
class CrosscorView : public MVGridView {
    Q_OBJECT
public:
    friend class CrosscorViewPrivate;
    CrosscorView(MVAbstractContext* mvcontext);
    virtual ~CrosscorView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void zoomAllTheWayOut();

protected:
    void keyPressEvent(QKeyEvent* evt) Q_DECL_OVERRIDE;
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos) Q_DECL_OVERRIDE;

private slots:
    void slot_update_panels();
    void slot_update_highlighting();
    void slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers);
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    CrosscorViewPrivate* d;
};

#endif // CrosscorView_H
