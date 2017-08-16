/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/
#ifndef ExportToMountainviewPROPERTIESDlg_H
#define ExportToMountainviewPROPERTIESDlg_H

#include <QDialog>

struct ExportToMountainviewProperties {
    double start_time_sec = 0, duration_sec = 3600;
};

class ExportToMountainviewDlgPrivate;
class ExportToMountainviewDlg : public QDialog {
    Q_OBJECT
public:
    friend class ExportToMountainviewDlgPrivate;
    ExportToMountainviewDlg();
    virtual ~ExportToMountainviewDlg();
    void setProperties(ExportToMountainviewProperties properties);
    ExportToMountainviewProperties properties() const;
private slots:
    void slot_update_enabled();

private:
    ExportToMountainviewDlgPrivate* d;
};

#endif // ExportToMountainviewPROPERTIESDlg_H
