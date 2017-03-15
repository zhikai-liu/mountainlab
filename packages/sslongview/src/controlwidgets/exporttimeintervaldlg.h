/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/25/2016
*******************************************************/
#ifndef ExportTimeIntervalDlg_H
#define ExportTimeIntervalDlg_H

#include <QDialog>

class ExportTimeIntervalDlgPrivate;
class ExportTimeIntervalDlg : public QDialog {
    Q_OBJECT
public:
    friend class ExportTimeIntervalDlgPrivate;
    ExportTimeIntervalDlg();
    virtual ~ExportTimeIntervalDlg();
    double startTimeSec() const;
    double durationSec() const;
    QString directory() const;
    QString timeseriesFileName() const;
    QString firingsFileName() const;
private slots:
    void slot_select_directory();

private:
    ExportTimeIntervalDlgPrivate* d;
};

#endif // ExportTimeIntervalDlg_H
