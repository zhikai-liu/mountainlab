#include "mveegcontext.h"

class MVEEGContextPrivate
{
public:
    MVEEGContext *q;
};

MVEEGContext::MVEEGContext() {
    d=new MVEEGContextPrivate;
    d->q=this;
}

MVEEGContext::~MVEEGContext() {
    delete d;
}

void MVEEGContext::setFromMV2FileObject(QJsonObject obj)
{

}

QJsonObject MVEEGContext::toMV2FileObject() const
{
    QJsonObject ret;
    return ret;
}
