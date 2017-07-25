#ifndef TemplatesControl_H
#define TemplatesControl_H

#include "mvabstractcontrol.h"

class TemplatesControlPrivate;
class TemplatesControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class TemplatesControlPrivate;
    TemplatesControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~TemplatesControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:

private:
    TemplatesControlPrivate* d;
};

#endif //
