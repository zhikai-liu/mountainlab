/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef TemplatesView_H
#define TemplatesView_H

#include "mvabstractview.h"
#include "mvgridview.h"
#include "mvgridview.h"

#include <mvabstractviewfactory.h>
#include <paintlayer.h>

class TemplatesViewPrivate;
class TemplatesView : public MVGridView {
    Q_OBJECT
public:
    friend class TemplatesViewPrivate;
    TemplatesView(MVAbstractContext* mvcontext);
    virtual ~TemplatesView();

    enum DisplayMode {
        All,
        Single,
        Neighborhood
    };

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void zoomAllTheWayOut();
    void setDisplayMode(DisplayMode mode);
    void setKs(const QList<int>& ks);

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
    TemplatesViewPrivate* d;
};

#endif // TemplatesView_H
