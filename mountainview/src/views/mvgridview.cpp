/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/

#include "mvgridview.h"

class MVGridViewPrivate {
public:
    MVGridView* q;
    QWidgetList m_views;
};

MVGridView::MVGridView(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVGridViewPrivate;
    d->q = this;
}

MVGridView::~MVGridView()
{
    clearViews();
    delete d;
}

void MVGridView::clearViews()
{
    qDeleteAll(d->m_views);
    d->m_views.clear();
}

void MVGridView::addView(QWidget* W)
{
    d->m_views << W;
}

int MVGridView::viewCount() const
{
    return d->m_views.count();
}

QWidget* MVGridView::view(int j) const
{
    return d->m_views[j];
}
