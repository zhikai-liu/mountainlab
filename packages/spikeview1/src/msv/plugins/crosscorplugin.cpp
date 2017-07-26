/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "crosscorplugin.h"
#include "templatesview.h"

#include <QMessageBox>
#include <QThread>
#include <crosscorview.h>
#include <crosscorcontrol.h>

class CrosscorPluginPrivate {
public:
    CrosscorPlugin* q;
};

CrosscorPlugin::CrosscorPlugin()
{
    d = new CrosscorPluginPrivate;
    d->q = this;
}

CrosscorPlugin::~CrosscorPlugin()
{
    delete d;
}

QString CrosscorPlugin::name()
{
    return "Crosscor";
}

QString CrosscorPlugin::description()
{
    return "";
}

void CrosscorPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new AutocorViewFactory(mw, (QWidget*)0, true));
    mw->registerViewFactory(new AutocorViewFactory(mw, (QWidget*)0, false));
    mw->registerViewFactory(new CrosscorMatrixViewFactory(mw));
    mw->addControl(new CrosscorControl(mw->mvContext(), mw), true);
}

/////////////////////////////////////////////////////////////////////////
AutocorViewFactory::AutocorViewFactory(MVMainWindow* mw, QObject* parent, bool all_mode)
    : MVAbstractViewFactory(mw, parent)
{
    m_all_mode = all_mode;
}

QString AutocorViewFactory::id() const
{
    if (m_all_mode)
        return QStringLiteral("open-all-autocor-view");
    else
        return QStringLiteral("open-autocor-view");
}

QString AutocorViewFactory::name() const
{
    if (m_all_mode)
        return tr("All autocor");
    else
        return tr("Autocor");
}

QString AutocorViewFactory::title() const
{
    if (m_all_mode)
        return tr("All autocor");
    else
        return tr("Autocor");
}

MVAbstractView* AutocorViewFactory::createView(MVAbstractContext* context)
{
    SVContext* c = qobject_cast<SVContext*>(context);
    Q_ASSERT(c);

    QList<int> keys = c->selectedClusters();
    if ((keys.isEmpty()) || (m_all_mode))
        keys = c->clusterAttributesKeys();
    QList<int> k1s, k2s;
    for (int i = 0; i < keys.count(); i++) {
        k1s << keys[i];
        k2s << keys[i];
    }

    CrosscorView* X = new CrosscorView(context);
    X->setKs(k1s, k2s, k1s);

    return X;
}

/////////////////////////////////////////////////////////////////////////
CrosscorMatrixViewFactory::CrosscorMatrixViewFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString CrosscorMatrixViewFactory::id() const
{
    return QStringLiteral("open-crosscor-matrix-view");
}

QString CrosscorMatrixViewFactory::name() const
{
    return tr("Crosscor Matrix");
}

QString CrosscorMatrixViewFactory::title() const
{
    return tr("Crosscor matrix");
}

MVAbstractView* CrosscorMatrixViewFactory::createView(MVAbstractContext* context)
{
    SVContext* c = qobject_cast<SVContext*>(context);
    Q_ASSERT(c);

    QList<int> keys = c->selectedClusters();
    if (keys.isEmpty())
        keys = c->clusterAttributesKeys();
    QList<int> k1s, k2s;
    for (int i = 0; i < keys.count(); i++) {
        for (int j = 0; j < keys.count(); j++) {
            k1s << keys[i];
            k2s << keys[j];
        }
    }

    if (k1s.count() > 300) {
        QMessageBox::information(0, "Too many cross-correlograms", "Please select a smaller number of cross-correlograms to display");
        return 0;
    }

    CrosscorView* X = new CrosscorView(context);
    X->setKs(k1s, k2s, k1s);
    X->setForceSquareMatrix(true);

    return X;
}
