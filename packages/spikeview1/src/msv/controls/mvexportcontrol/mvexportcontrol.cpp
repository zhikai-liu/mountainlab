/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "flowlayout.h"
#include "mvexportcontrol.h"
#include "mlcommon.h"
#include "exporttomountainviewdlg.h"

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
#include <mlvector.h>
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
        QPushButton* B = new QPushButton("Export subset to mountainview");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_export_subset_to_mountainview()));
        flayout->addWidget(B);
    }
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

QString make_subfirings(QString firings_path, ExportToMountainviewProperties properties, double samplerate)
{
    TaskProgress task;
    double t1 = properties.start_time_sec * samplerate;
    double t2plus = (properties.start_time_sec + properties.duration_sec) * samplerate;
    QString subfirings_path = CacheManager::globalInstance()->makeExpiringFile("subfirings_" + MLUtil::makeRandomId(10) + ".mda", 60 * 60);
    Mda firings(firings_path);
    MLVector<bigint> inds_to_use;
    task.setLabel("Making subfirings...");
    for (bigint i = 0; i < firings.N2(); i++) {
        if (i % 1000 == 0)
            task.setProgress(i * 1.0 / firings.N2());
        double t0 = firings.get(1, i);
        //int k0=firings.get(2,i);
        if ((t1 <= t0) && (t0 < t2plus)) {
            inds_to_use << i;
        }
    }
    task.setLabel("Setting subfirings...");
    Mda subfirings(firings.N1(), inds_to_use.count());
    for (bigint i = 0; i < inds_to_use.count(); i++) {
        if (i % 1000 == 0)
            task.setProgress(i * 1.0 / inds_to_use.count());
        for (int j = 0; j < firings.N1(); j++) {
            subfirings.set(firings.get(j, inds_to_use[i]), j, i);
        }
    }
    task.setLabel("Writing subfirings...");
    task.setProgress(0);
    subfirings.write64(subfirings_path);
    return subfirings_path;
}

void MVExportControl::slot_export_subset_to_mountainview()
{
    SVContext* c = qobject_cast<SVContext*>(mvContext());
    Q_ASSERT(c);

    ExportToMountainviewDlg dlg;
    {
        ExportToMountainviewProperties properties;
        properties.start_time_sec = 0;
        properties.duration_sec = c->currentTimeseries().N2() / c->sampleRate();
        dlg.setProperties(properties);
    }
    if (dlg.exec() == QDialog::Accepted) {
        ExportToMountainviewProperties properties = dlg.properties();
        QString subfirings = make_subfirings(c->firings().makePath(), properties, c->sampleRate());
        QStringList args;
        args << QString("--samplerate=%1").arg(c->sampleRate());
        args << QString("--firings=%1").arg(subfirings);
        if (c->currentTimeseriesName() == "Raw Data") {
            args << QString("--raw=%1").arg(c->currentTimeseries().makePath());
        }
        if (c->currentTimeseriesName() == "Filtered Data") {
            args << QString("--filt=%1").arg(c->currentTimeseries().makePath());
        }
        if (c->currentTimeseriesName() == "Preprocessed Data") {
            args << QString("--pre=%1").arg(c->currentTimeseries().makePath());
        }
        QString exe = "mountainview";
        args << QString("--t1=%1 --t2=%2").arg(properties.start_time_sec * c->sampleRate()).arg((properties.start_time_sec + properties.duration_sec) * c->sampleRate() - 1);
        QProcess::startDetached(exe, args);
        //QString cmd=QString("%1 %2").arg(exe).arg(args.join(" "));

        //int ret = system(cmd.toUtf8().data());
        //(void)ret;
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
