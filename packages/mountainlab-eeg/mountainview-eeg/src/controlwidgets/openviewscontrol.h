/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#ifndef OPENVIEWSCONTROL_H
#define OPENVIEWSCONTROL_H

#include "mvabstractcontrol.h"

class OpenViewsControlPrivate;
class OpenViewsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class OpenViewsControlPrivate;
    OpenViewsControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~OpenViewsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_open_view(QObject* obj);
    void slot_update_enabled();
    void slot_close_all_views();

private:
    OpenViewsControlPrivate* d;
};

#endif // OPENVIEWSCONTROL_H
