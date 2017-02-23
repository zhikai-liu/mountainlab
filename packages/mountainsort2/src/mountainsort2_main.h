/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/
#ifndef MOUNTAINSORT2_MAIN_H
#define MOUNTAINSORT2_MAIN_H

#include <QJsonObject>

QJsonObject get_spec();

struct ProcessorSpecFile {
    QString name;
    QString description;

    QJsonObject get_spec();
};

struct ProcessorSpecParam {
    QString name;
    QString description;
    bool optional = false;

    QJsonObject get_spec();
};

struct ProcessorSpec {
    ProcessorSpec(QString name, QString version_in)
    {
        processor_name = name;
        version = version_in;
    }

    QString processor_name;
    QString version;
    QString description;
    QList<ProcessorSpecFile> inputs;
    QList<ProcessorSpecFile> outputs;
    QList<ProcessorSpecParam> parameters;

    void addInputs(QString name1, QString name2 = "", QString name3 = "", QString name4 = "", QString name5 = "");
    void addOutputs(QString name1, QString name2 = "", QString name3 = "", QString name4 = "", QString name5 = "");
    void addRequiredParameters(QString name1, QString name2 = "", QString name3 = "", QString name4 = "", QString name5 = "");
    void addOptionalParameters(QString name1, QString name2 = "", QString name3 = "", QString name4 = "", QString name5 = "");

    void addInput(QString name, QString description = "");
    void addOutput(QString name, QString description = "");
    void addRequiredParameter(QString name, QString description = "");
    void addOptionalParameter(QString name, QString description = "");

    QJsonObject get_spec();
};

#endif // MOUNTAINSORT2_MAIN_H
