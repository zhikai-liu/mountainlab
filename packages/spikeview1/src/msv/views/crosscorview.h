/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef CrosscorView_H
#define CrosscorView_H

#include "mvabstractview.h"
#include "mvgridview.h"
#include "mvgridview.h"

#include <QMutex>
#include <QThread>
#include <mvabstractviewfactory.h>
#include <paintlayer.h>

class CrosscorViewPrivate;
class CrosscorView : public MVGridView {
    Q_OBJECT
public:
    friend class CrosscorViewPrivate;
    CrosscorView(MVAbstractContext* mvcontext);
    virtual ~CrosscorView();

    void setKs(const QList<int> &k1s,const QList<int> &k2s,const QList<int> &ks);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

protected:
    void keyPressEvent(QKeyEvent* evt) Q_DECL_OVERRIDE;
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos) Q_DECL_OVERRIDE;

private slots:
    void slot_update_panels();
    void slot_update_highlighting();
    void slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers);
    void slot_worker_data_update();

private:
    CrosscorViewPrivate* d;
};

class CrosscorWorker : public QThread {
    Q_OBJECT
public:
    void run();

    //stuff
    QMutex mutex;

    //input
    QVector<double> times;
    QVector<int> labels;
    QList<int> k1s,k2s;
    double dt_max=30*50;

    //output
    QMap<int,QMap<int,QVector<double>>> hist_data;
signals:
    void data_update();
};


#endif // CrosscorView_H
