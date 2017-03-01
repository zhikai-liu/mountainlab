/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "sslvclustermetricsview.h"

#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QRadioButton>
#include <QTreeWidget>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"

class SSLVClusterMetricsViewPrivate {
public:
    SSLVClusterMetricsView* q;
    QTreeWidget* m_tree;

    void refresh_tree();
};

SSLVClusterMetricsView::SSLVClusterMetricsView(MVAbstractContext* sslvcontext)
    : MVAbstractView(sslvcontext)
{
    d = new SSLVClusterMetricsViewPrivate;
    d->q = this;

    QHBoxLayout* hlayout = new QHBoxLayout;
    this->setLayout(hlayout);

    d->m_tree = new QTreeWidget;
    d->m_tree->setSortingEnabled(true);
    d->m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    hlayout->addWidget(d->m_tree);

    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    this->recalculateOn(c, SIGNAL(clusterAttributesChanged(int)), false);
    this->recalculateOn(c, SIGNAL(clusterVisibilityChanged()), false);

    d->refresh_tree();
    this->recalculate();

    QObject::connect(d->m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(slot_item_selection_changed()));
    QObject::connect(d->m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slot_current_item_changed()));
    QObject::connect(c, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_current_cluster()));
    QObject::connect(c, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_selected_clusters()));
}

SSLVClusterMetricsView::~SSLVClusterMetricsView()
{
    this->stopCalculation();
    delete d;
}

void SSLVClusterMetricsView::prepareCalculation()
{
    if (!mvContext())
        return;
}

void SSLVClusterMetricsView::runCalculation()
{
}

void SSLVClusterMetricsView::onCalculationFinished()
{
    d->refresh_tree();
}

void SSLVClusterMetricsView::keyPressEvent(QKeyEvent* evt)
{
    Q_UNUSED(evt)
}

void SSLVClusterMetricsView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);

    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    ds << c->selectedClusters();
    mimeData.setData("application/x-msv-clusters", ba); // selected cluster data

    MVAbstractView::prepareMimeData(mimeData, pos); // call base class implementation
}

void SSLVClusterMetricsView::slot_current_item_changed()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    QTreeWidgetItem* it = d->m_tree->currentItem();
    if (it) {
        int k = it->data(0, Qt::UserRole).toInt();
        c->setCurrentCluster(k);
    }
    else {
        c->setCurrentCluster(-1);
    }
}

void SSLVClusterMetricsView::slot_item_selection_changed()
{
    //QList<QTreeWidgetItem*> items=d->m_tree->selectedItems();

    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    QSet<int> selected = c->selectedClusters().toSet();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        if (it->isSelected()) {
            selected.insert(k);
        }
        else {
            selected.remove(k);
        }
    }

    c->setSelectedClusters(selected.toList());
}

void SSLVClusterMetricsView::slot_update_current_cluster()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    int current = c->currentCluster();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        if (k == current) {
            d->m_tree->setCurrentItem(it);
        }
    }
}

void SSLVClusterMetricsView::slot_update_selected_clusters()
{
    SSLVContext* c = qobject_cast<SSLVContext*>(mvContext());
    Q_ASSERT(c);

    QSet<int> selected = c->selectedClusters().toSet();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        it->setSelected(selected.contains(k));
    }
}

class NumericSortTreeWidgetItem : public QTreeWidgetItem {
public:
    NumericSortTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent)
    {
    }

private:
    bool operator<(const QTreeWidgetItem& other) const
    {
        int column = treeWidget()->sortColumn();
        return text(column).toDouble() > other.text(column).toDouble();
    }
};

void SSLVClusterMetricsViewPrivate::refresh_tree()
{
    m_tree->clear();

    SSLVContext* c = qobject_cast<SSLVContext*>(q->mvContext());
    Q_ASSERT(c);

    QJsonObject obj = c->clusterMetrics();

    QJsonArray clusters=obj["clusters"].toArray();

    QSet<QString> metric_names_set;
    for (int ii = 0; ii < clusters.count(); ii++) {
        QJsonObject cluster=clusters[ii].toObject();
        QJsonObject metrics = cluster["metrics"].toObject();
        QStringList nnn = metrics.keys();
        foreach (QString name, nnn) {
            metric_names_set.insert(name);
        }
    }

    QStringList metric_names = metric_names_set.toList();
    qSort(metric_names);

    QStringList headers;
    headers << "Cluster";
    headers.append(metric_names);
    m_tree->setHeaderLabels(headers);

    for (int ii = 0; ii < clusters.count(); ii++) {
        QJsonObject cluster=clusters[ii].toObject();
        QJsonObject metrics = cluster["metrics"].toObject();
        int k = cluster["label"].toInt();
        if (c->clusterIsVisible(k)) {
            NumericSortTreeWidgetItem* it = new NumericSortTreeWidgetItem(m_tree);
            it->setText(0, QString("%1").arg(k));
            it->setData(0, Qt::UserRole, k);
            for (int j = 0; j < metric_names.count(); j++) {
                it->setText(j + 1, QString("%1").arg(metrics[metric_names[j]].toDouble()));
            }
            m_tree->addTopLevelItem(it);
        }
    }

    for (int c = 0; c < m_tree->columnCount(); c++) {
        m_tree->resizeColumnToContents(c);
    }
}
