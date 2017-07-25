/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef CrosscorViewPanel_H
#define CrosscorViewPanel_H

#include "paintlayer.h"
#include <QWidget>
#include <mda.h>
#include "svcontext.h"

class CrosscorViewPanelPrivate;
class CrosscorViewPanel : public PaintLayer {
public:
    friend class CrosscorViewPanelPrivate;
    CrosscorViewPanel();
    virtual ~CrosscorViewPanel();
    void setTemplate(const Mda32& X);
    void setElectrodeGeometry(const ElectrodeGeometry& geom);
    void setVerticalScaleFactor(double factor);
    void setChannelColors(const QList<QColor>& colors);
    void setColors(const QMap<QString, QColor>& colors);
    void setCurrent(bool val);
    void setSelected(bool val);
    void setTitle(const QString& txt);
    void setFiringRateDiskDiameter(double val);

protected:
    void paint(QPainter* painter) Q_DECL_OVERRIDE;

private:
    CrosscorViewPanelPrivate* d;
};

#endif // CrosscorViewPanel_H
