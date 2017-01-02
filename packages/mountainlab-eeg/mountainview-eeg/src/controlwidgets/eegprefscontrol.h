#ifndef EEGPrefsCONTROL_H
#define EEGPrefsCONTROL_H

#include "mvabstractcontrol.h"

class EEGPrefsControlPrivate;
class EEGPrefsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class EEGPrefsControlPrivate;
    EEGPrefsControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~EEGPrefsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:

private:
    EEGPrefsControlPrivate* d;
};

#endif // EEGPrefsCONTROL_H
