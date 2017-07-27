#ifndef SSLVToolsCONTROL_H
#define SSLVToolsCONTROL_H

#include "mvabstractcontrol.h"

class SSLVToolsControlPrivate;
class SSLVToolsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class SSLVToolsControlPrivate;
    SSLVToolsControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~SSLVToolsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_export_time_interval();

private:
    SSLVToolsControlPrivate* d;
};

#endif // SSLVToolsCONTROL_H
