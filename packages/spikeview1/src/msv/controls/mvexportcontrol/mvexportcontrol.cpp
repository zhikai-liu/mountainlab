/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "flowlayout.h"
#include "mvexportcontrol.h"
#include "mlcommon.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTextBrowser>
#include <QProcess>
#include "taskprogress.h"
#include <QThread>
#include <cachemanager.h>
#include "diskreadmda.h"
#include "svcontext.h"
//#include "mvcontext.h"

class MVExportControlPrivate {
public:
    MVExportControl* q;
};

MVExportControl::MVExportControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVExportControlPrivate;
    d->q = this;

    QFont fnt = this->font();
    fnt.setPixelSize(qMax(14, fnt.pixelSize() - 6));
    this->setFont(fnt);

    FlowLayout* flayout = new FlowLayout;
    this->setLayout(flayout);
    {
        QPushButton* B = new QPushButton("Export .mv2 document");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_export_mv2_document()));
        flayout->addWidget(B);
    }
    /*
    {
        QPushButton* B = new QPushButton("Export firings");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_export_firings()));
        flayout->addWidget(B);
    }
    {
        QPushButton* B = new QPushButton("Export curated firings");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_export_curated_firings()));
        flayout->addWidget(B);
    }
    {
        QPushButton* B = new QPushButton("Open PRV manager");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_open_prv_manager()));
        flayout->addWidget(B);
    }
    */

    updateControls();
}

MVExportControl::~MVExportControl()
{
    delete d;
}

QString MVExportControl::title() const
{
    return "Export / Upload / Download";
}

void MVExportControl::updateContext()
{
}

void MVExportControl::updateControls()
{
}

void MVExportControl::slot_export_mv2_document()
{
    //MVContext* c = qobject_cast<MVContext*>(mvContext());
    //Q_ASSERT(c);

    //QSettings settings("SCDA", "MountainView");
    //QString default_dir = settings.value("default_export_dir", "").toString();
    QString default_dir = QDir::currentPath();
    QString fname = QFileDialog::getSaveFileName(this, "Export mv2 document", default_dir, "*.mv2");
    if (fname.isEmpty())
        return;
    //settings.setValue("default_export_dir", QFileInfo(fname).path());
    if (QFileInfo(fname).suffix() != "mv2")
        fname = fname + ".mv2";
    TaskProgress task("export mv2 document");
    task.log() << "Writing: " + fname;
    QJsonObject obj = mvContext()->toMV2FileObject();
    QString json = QJsonDocument(obj).toJson();
    if (TextFile::write(fname, json)) {
        mvContext()->setMV2FileName(fname);
        task.log() << QString("Wrote %1 kilobytes").arg(json.count() * 1.0 / 1000);
    }
    else {
        task.error("Error writing .mv2 file: " + fname);
    }
}

QString get_local_path_of_firings_file_or_current_path(const DiskReadMda& X)
{
    Q_UNUSED(X)
    /*QString path = X.makePath();
    if (!path.isEmpty()) {
        if (!path.startsWith("http:")) {
            if (QFile::exists(path)) {
                return QFileInfo(path).path();
            }
        }
    }*/
    return QDir::currentPath();
}

