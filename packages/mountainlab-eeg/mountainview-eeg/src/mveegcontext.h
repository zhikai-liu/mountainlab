#ifndef MVEEGCONTEXT_H
#define MVEEGCONTEXT_H

#include "mvabstractcontext.h"

class MVEEGContextPrivate;
class MVEEGContext : public MVAbstractContext
{
public:
    friend class MVEEGContextPrivate;
    MVEEGContext();
    virtual ~MVEEGContext();

    void setFromMV2FileObject(QJsonObject obj) Q_DECL_OVERRIDE;
    QJsonObject toMV2FileObject() const Q_DECL_OVERRIDE;
private:
    MVEEGContextPrivate *d;
};

#endif // MVEEGCONTEXT_H
