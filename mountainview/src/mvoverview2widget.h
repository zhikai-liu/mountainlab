#ifndef MVOVERVIEW2WIDGET_H
#define MVOVERVIEW2WIDGET_H

#include <QWidget>
#include "mda.h"
#include <QTabWidget>
#include "mvutils.h"

/** \class MVOverview2Widget
 *  \brief The main window (for now) showing an overview of the results of sorting by providing a number of interactive and synchronized views.
 *
 *  Presents user with a rich set of views. Cross-correlograms, raw data, cluster details, rotatable 3D views, firing rate vs. time view, etc.
 */

class MVOverview2WidgetPrivate;
class CustomTabWidget;
class MVOverview2Widget : public QWidget
{
	Q_OBJECT
public:
	friend class MVOverview2WidgetPrivate;
	friend class CustomTabWidget;
	MVOverview2Widget(QWidget *parent=0);
	virtual ~MVOverview2Widget();
	///The path to the timeseries that was sorted. For example, raw, filtered, or pre-processed. Usually all three of these are set, so user can choose between them in dropdown selection box
    void addTimeseriesPath(const QString &name,const QString &path);
	///The name of the timeseries being viewed... corresponds to name in addTimeseriesPath()
    void setCurrentTimeseriesName(const QString &name);
	///Set the path to the results of sorting. TODO: refer to docs for info on firings array
	void setFiringsPath(const QString &firings);
	///The sample rate for the dataset
	void setSampleRate(float freq);
	///Open the initial views
	void setDefaultInitialization();
	///Corresponds to MVFiringRateView::setEpochs()
    void setEpochs(const QList<Epoch> &epochs);

protected:
	void resizeEvent(QResizeEvent *evt);

signals:

public slots:

private slots:
	void slot_control_panel_button_clicked(QString str);
    void slot_control_panel_combobox_activated(QString str);
	void slot_auto_correlogram_activated(int k);
    //void slot_templates_clicked();
	void slot_details_current_k_changed();
	void slot_details_selected_ks_changed();
    void slot_details_template_activated();
    void slot_cross_correlogram_current_label_changed();
	void slot_cross_correlogram_selected_labels_changed();
	void slot_clips_view_current_event_changed();
	void slot_cluster_view_current_event_changed();

private:
	MVOverview2WidgetPrivate *d;
};

/** \class CustomTabWidget
 *  \brief Responds to clicking and double clicking of tabs. Used by MVOverview2Widget
 */

class CustomTabWidget : public QTabWidget {
	Q_OBJECT
public:
	MVOverview2Widget *q;
	CustomTabWidget(MVOverview2Widget *q);
protected:
	void mousePressEvent(QMouseEvent *evt);
private slots:
	void slot_tab_close_requested(int num);
	void slot_tab_bar_clicked();
	void slot_tab_bar_double_clicked();
	void slot_switch_to_other_tab_widget();
};


#endif // MVOVERVIEW2WIDGET_H