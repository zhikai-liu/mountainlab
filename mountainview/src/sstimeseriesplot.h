#ifndef SSTIMESERIESPLOT_H
#define SSTIMESERIESPLOT_H

#include <QWidget>
#include "plotarea.h" //for Vec2
#include "sslabelsmodel.h"
#include "ssabstractplot.h"
#include "sslabelsmodel1.h"

class SSTimeSeriesPlotPrivate;
class OverlayPainter;
class SSTimeSeriesPlot : public SSAbstractPlot
{
	Q_OBJECT
public:
	friend class SSTimeSeriesPlotPrivate;
	explicit SSTimeSeriesPlot(QWidget *parent = 0);
	~SSTimeSeriesPlot();

	//virtual from base class
	void updateSize(int W,int H);
	Vec2 coordToPix(Vec2 coord);
	Vec2 pixToCoord(Vec2 pix);
	void setXRange(const Vec2 &range);
	void setYRange(const Vec2 &range);
	void initialize();

	DiskArrayModel_New *data();

	void setChannelLabels(const QStringList &labels);
	void setUniformVerticalChannelSpacing(bool val);
    void setFixedVerticalChannelSpacing(double fixed_val);
    bool uniformVerticalChannelSpacing();
    void setShowMarkerLines(bool val);
	void setControlPanelVisible(bool val);

	void setData(SSARRAY *data);
	void setLabels(SSLabelsModel1 *L,bool is_owner=false);
    void setCompareLabels(SSLabelsModel1 *L,bool is_owner=false);
	int pixToChannel(Vec2 pix);
	void setMargins(int left,int right,int top,int bottom);
	void setConnectZeros(bool val);



private slots:
	void slot_replot_needed();
    void slot_setup_plot_area();

private:
	SSTimeSeriesPlotPrivate *d;

protected:
	virtual void paintPlot(QPainter *painter,int W,int H);
	void mousePressEvent(QMouseEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);

signals:
	void requestMoveToTimepoint(int t0);

public slots:
};


#endif // SSTIMESERIESPLOT_H