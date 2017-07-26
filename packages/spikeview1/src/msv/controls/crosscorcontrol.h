#ifndef CrosscorControl_H
#define CrosscorControl_H

#include "mvabstractcontrol.h"

class CrosscorControlPrivate;
class CrosscorControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class CrosscorControlPrivate;
    CrosscorControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~CrosscorControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:

private:
    CrosscorControlPrivate* d;
};

#endif //
