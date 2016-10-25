/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/
#ifndef MVGRIDVIEW_H
#define MVGRIDVIEW_H

#include <mvabstractview.h>

class MVGridViewPrivate;
class MVGridView : public MVAbstractView {
public:
    friend class MVGridViewPrivate;
    MVGridView(MVContext* context);
    virtual ~MVGridView();

protected:
    void clearViews();
    void addView(QWidget* W);
    int viewCount() const;
    QWidget* view(int j) const;

private:
    MVGridViewPrivate* d;
};

#endif // MVGRIDVIEW_H
