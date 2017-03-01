#ifndef SSLVPrefsCONTROL_H
#define SSLVPrefsCONTROL_H

#include "mvabstractcontrol.h"

class SSLVPrefsControlPrivate;
class SSLVPrefsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class SSLVPrefsControlPrivate;
    SSLVPrefsControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~SSLVPrefsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:

private:
    SSLVPrefsControlPrivate* d;
};

#endif // SSLVPrefsCONTROL_H
