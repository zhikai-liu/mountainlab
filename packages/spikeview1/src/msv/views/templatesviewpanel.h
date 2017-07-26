/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef TemplatesViewPanel_H
#define TemplatesViewPanel_H

#include "paintlayer.h"
#include <QWidget>
#include <mda.h>
#include "svcontext.h"

class TemplatesViewPanelPrivate;
class TemplatesViewPanel : public PaintLayer {
public:
    friend class TemplatesViewPanelPrivate;
    TemplatesViewPanel();
    virtual ~TemplatesViewPanel();
    void setTemplate(const Mda32& X);
    void setElectrodesToShow(const QSet<int> &electrodes_to_show);
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
    TemplatesViewPanelPrivate* d;
};

#endif // TemplatesViewPanel_H
