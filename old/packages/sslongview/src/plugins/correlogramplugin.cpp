/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "correlogramplugin.h"

#include <sslvcontext.h>
#include <QMessageBox>
#include <correlogramwidget.h>

class CorrelogramPluginPrivate {
public:
    CorrelogramPlugin* q;
};

CorrelogramPlugin::CorrelogramPlugin()
{
    d = new CorrelogramPluginPrivate;
    d->q = this;
}

CorrelogramPlugin::~CorrelogramPlugin()
{
    delete d;
}

QString CorrelogramPlugin::name()
{
    return "Correlogram";
}

QString CorrelogramPlugin::description()
{
    return "";
}

void CorrelogramPlugin::initialize(MVMainWindow* mw)
{
    //mw->registerViewFactory(new AutoCorrelogramsFactory(mw));
    mw->registerViewFactory(new SelectedAutoCorrelogramsFactory(mw));
    mw->registerViewFactory(new MatrixOfCrossCorrelogramsFactory(mw));
}

AutoCorrelogramsFactory::AutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString AutoCorrelogramsFactory::id() const
{
    return QStringLiteral("open-auto-correlograms");
}

QString AutoCorrelogramsFactory::name() const
{
    return tr("All auto-correlograms");
}

QString AutoCorrelogramsFactory::title() const
{
    return tr("All auto-Correlograms");
}

MVAbstractView* AutoCorrelogramsFactory::createView(MVAbstractContext* context)
{
    CorrelogramWidget* X = new CorrelogramWidget(context);
    CrossCorrelogramOptions3 opts;
    opts.mode = All_Auto_Correlograms3;
    X->setOptions(opts);
    return X;
}

bool AutoCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    Q_UNUSED(context)
    return true;
}

/*
QList<QAction*> AutoCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    if (!md.data("x-mv-main").isEmpty()) {
        QAction* action = new QAction("All auto-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-auto-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

SelectedAutoCorrelogramsFactory::SelectedAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SelectedAutoCorrelogramsFactory::id() const
{
    return QStringLiteral("open-selected-auto-correlograms");
}

QString SelectedAutoCorrelogramsFactory::name() const
{
    return tr("Selected auto-correlograms");
}

QString SelectedAutoCorrelogramsFactory::title() const
{
    return tr("Selected auto-correlograms");
}

MVAbstractView* SelectedAutoCorrelogramsFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    CorrelogramWidget* X = new CorrelogramWidget(context);
    QList<int> ks = c->selectedClusters();
    //if (ks.isEmpty())
    //   ks = c->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Selected_Auto_Correlograms3;
    opts.ks = ks;
    X->setOptions(opts);
    return X;
}

bool SelectedAutoCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    return (c->selectedClusters().count() >= 1);
}

/*
QList<QAction*> SelectedAutoCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Selected auto-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-selected-auto-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

CrossCorrelogramsFactory::CrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString CrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-cross-correlograms");
}

QString CrossCorrelogramsFactory::name() const
{
    return tr("Cross-Correlograms");
}

QString CrossCorrelogramsFactory::title() const
{
    return tr("Cross-Correlograms");
}

MVAbstractView* CrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    CorrelogramWidget* X = new CorrelogramWidget(context);
    QList<int> ks = c->selectedClusters();
    if (ks.count() != 1)
        return X;

    CrossCorrelogramOptions3 opts;
    opts.mode = Cross_Correlograms3;
    opts.ks = ks;

    X->setOptions(opts);
    return X;
}

bool CrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    return (c->selectedClusters().count() == 1);
}

/*
QList<QAction*> CrossCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Cross-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-cross-correlograms");
        });
        if (clusters.count() != 1)
            action->setEnabled(false);
        actions << action;
    }
    return actions;
}
*/

MatrixOfCrossCorrelogramsFactory::MatrixOfCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MatrixOfCrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-matrix-of-cross-correlograms");
}

QString MatrixOfCrossCorrelogramsFactory::name() const
{
    return tr("Matrix of Cross-Correlograms");
}

QString MatrixOfCrossCorrelogramsFactory::title() const
{
    return tr("CC Matrix");
}

MVAbstractView* MatrixOfCrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    CorrelogramWidget* X = new CorrelogramWidget(context);
    QList<int> ks = c->selectedClusters();
    //if (ks.isEmpty())
    //    ks = c->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Matrix_Of_Cross_Correlograms3;
    opts.ks = ks;
    X->setOptions(opts);
    return X;
}

bool MatrixOfCrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    return (!c->selectedClusters().isEmpty());
}

/*
QList<QAction*> MatrixOfCrossCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Matrix of cross-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-matrix-of-cross-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

SelectedCrossCorrelogramsFactory::SelectedCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString SelectedCrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-selected-cross-correlograms");
}

QString SelectedCrossCorrelogramsFactory::name() const
{
    return tr("Selected Cross-Correlograms");
}

QString SelectedCrossCorrelogramsFactory::title() const
{
    return tr("CC");
}

MVAbstractView* SelectedCrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    CorrelogramWidget* X = new CorrelogramWidget(context);
    CrossCorrelogramOptions3 opts;
    opts.mode = Selected_Cross_Correlograms3;
    X->setOptions(opts);
    return X;
}

bool SelectedCrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    SSLVContext* c = qobject_cast<SSLVContext*>(context);
    Q_ASSERT(c);

    //return (!c->selectedClusterPairs().isEmpty());
    return (!c->selectedClusters().isEmpty());
}

/*
QList<QAction*> SelectedCrossCorrelogramsFactory::actions(const QMimeData& md)
{
    Q_UNUSED(md)
    QList<QAction*> actions;
    return actions;
}
*/
