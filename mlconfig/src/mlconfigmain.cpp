/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/4/2016
*******************************************************/

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "mlconfigpage.h"

class Page_tempdir : public MLConfigPage {
    Page_tempdir(QJsonObject* config)
        : MLConfigPage(config){};
    QString title() Q_DECL_OVERRIDE
    {
        return "Temporary Directory";
    }
    QString description() Q_DECL_OVERRIDE
    {
        return "This is the directory where all temporary and intermediate files are stored. "
               "It should be in a location with a lot of free disk space. You may safely "
               "delete the data periodically, but it is a good idea not to do this during "
               "processing.";
    }
};

QString format_description(QString str, int line_length)
{
    QString ret;
    int i = 0;
    while (i < str.count()) {
        int j = i + line_length;
        if (j >= str.count())
            j = str.count() - 1;
        QString line = str.mid(i, j - i + 1);
        int aa = line.lastIndexOf(" ");
        if (aa > 0) {
            j = i + aa - 1;
        }
        line = line.mid(0, j - i + 1);
        ret += line.trimmed() + "\n";
        i += line.count();
    }
    return ret;
}

int main(int argc, char* argv[])
{
    QJsonObject config = read_config();

    QList<MLConfigPage*> pages;
    pages << new Page_tempdir(config);

    qDebug().noquote() << "___MountainLab interactive configuration utility___";
    qDebug().noquote() << "";

    foreach (MLConfigPage* page, pages) {
        qDebug().noquote() << "*** " + page->title() + " ***";
        QString descr = format_description(page->description(), 60);
        qDebug().noquote() << descr;
        qDebug().noquote() << "";
        for (int i = 0; i < page->questionCount(); i++) {
            page->questions(i)->ask();
            QString resp = get_keyboard_response();
            page->questions(i)->processResponse();
        }
    }

    qDeleteAll(pages);
    pages.clear();

    return 0;
}
