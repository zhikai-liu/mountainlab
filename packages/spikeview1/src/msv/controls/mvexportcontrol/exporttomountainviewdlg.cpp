/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/

#include "exporttomountainviewdlg.h"
#include "ui_exporttomountainviewdlg.h"

class ExportToMountainviewDlgPrivate {
public:
    ExportToMountainviewDlg* q;
    Ui_ExportToMountainviewDlg ui;
    ExportToMountainviewProperties m_properties;
};

ExportToMountainviewDlg::ExportToMountainviewDlg()
{
    d = new ExportToMountainviewDlgPrivate;
    d->q = this;

    d->ui.setupUi(this);
    QObject::connect(d->ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(d->ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

ExportToMountainviewDlg::~ExportToMountainviewDlg()
{
    delete d;
}

ExportToMountainviewProperties ExportToMountainviewDlg::properties() const
{
    d->m_properties.start_time_sec = d->ui.start_time_sec->text().toDouble();
    d->m_properties.duration_sec = d->ui.duration_sec->text().toDouble();
    return d->m_properties;
}

void ExportToMountainviewDlg::slot_update_enabled()
{
}

void ExportToMountainviewDlg::setProperties(ExportToMountainviewProperties properties)
{
    d->m_properties = properties;
    d->ui.start_time_sec->setText(QString("%1").arg(d->m_properties.start_time_sec));
    d->ui.duration_sec->setText(QString("%2").arg(d->m_properties.duration_sec));
    slot_update_enabled();
}
