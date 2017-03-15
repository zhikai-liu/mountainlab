/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/

#include "exporttimeintervaldlg.h"
#include "ui_exporttimeintervaldlg.h"

#include <QFileDialog>

class ExportTimeIntervalDlgPrivate {
public:
    ExportTimeIntervalDlg* q;
    Ui_ExportTimeIntervalDlg ui;
};

ExportTimeIntervalDlg::ExportTimeIntervalDlg()
{
    d = new ExportTimeIntervalDlgPrivate;
    d->q = this;

    d->ui.setupUi(this);

    connect(d->ui.select_directory_button, SIGNAL(clicked(bool)), this, SLOT(slot_select_directory()));
}

ExportTimeIntervalDlg::~ExportTimeIntervalDlg()
{
    delete d;
}

double ExportTimeIntervalDlg::startTimeSec() const
{
    return d->ui.start_time->text().toDouble();
}

double ExportTimeIntervalDlg::durationSec() const
{
    return d->ui.duration->text().toDouble();
}

QString ExportTimeIntervalDlg::directory() const
{
    return d->ui.directory->text();
}

QString ExportTimeIntervalDlg::timeseriesFileName() const
{
    return d->ui.timeseries_file->text();
}

QString ExportTimeIntervalDlg::firingsFileName() const
{
    return d->ui.firings_file->text();
}

void ExportTimeIntervalDlg::slot_select_directory()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select directory for export", this->directory());
    if (path.isEmpty())
        return;
    d->ui.directory->setText(path);
}
