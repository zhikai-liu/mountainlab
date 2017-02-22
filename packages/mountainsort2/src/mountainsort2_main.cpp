/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/

#include "mlcommon.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include "mountainsort2_main.h"
#include "p_extract_neighborhood_timeseries.h"
#include "p_bandpass_filter.h"
#include "p_detect_events.h"
#include "p_extract_clips.h"
#include "p_sort_clips.h"
#include "p_consolidate_clusters.h"
#include "p_create_firings.h"
#include "p_combine_firings.h"
#include "p_fit_stage.h"

QJsonObject get_spec() {
    QJsonArray processors;

    {
        ProcessorSpec X("mountainsort.extract_neighborhood_timeseries","0.1");
        X.addInputs("timeseries");
        X.addOutputs("timeseries_out");
        X.addRequiredParameters("channels");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.bandpass_filter","0.1");
        X.addInputs("timeseries");
        X.addOutputs("timeseries_out");
        X.addRequiredParameters("samplerate","freq_min","freq_max","freq_wid");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.detect_events","0.1");
        X.addInputs("timeseries");
        X.addOutputs("event_times_out");
        X.addRequiredParameters("central_channel","detect_threshold","detect_interval","sign");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.extract_clips","0.1");
        X.addInputs("timeseries","event_times");
        X.addOutputs("clips_out");
        X.addRequiredParameters("clip_size");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.sort_clips","0.1");
        X.addInputs("clips");
        X.addOutputs("labels_out");
        //X.addRequiredParameters();
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.consolidate_clusters","0.1");
        X.addInputs("clips","labels");
        X.addOutputs("labels_out");
        X.addRequiredParameters("central_channel");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.create_firings","0.1");
        X.addInputs("event_times","labels");
        X.addOutputs("firings_out");
        X.addRequiredParameters("central_channel");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.combine_firings","0.1");
        X.addInputs("firings_list");
        X.addOutputs("firings_out");
        //X.addRequiredParameters();
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.fit_stage","0.1");
        X.addInputs("timeseries","firings");
        X.addOutputs("firings_out");
        //X.addRequiredParameters();
        processors.push_back(X.get_spec());
    }

    QJsonObject ret;
    ret["processors"]=processors;
    return ret;
}


int main(int argc,char *argv[]) {
    QCoreApplication app(argc,argv);

    CLParams CLP(argc,argv);

    QString arg1=CLP.unnamed_parameters.value(0);

    if (arg1 == "spec") {
        QJsonObject spec=get_spec();
        QString json=QJsonDocument(spec).toJson(QJsonDocument::Indented);
        printf("%s\n",json.toUtf8().data());
        return 0;
    }

    bool ret=false;

    if (arg1=="mountainsort.extract_neighborhood_timeseries") {
        QString timeseries=CLP.named_parameters["timeseries"].toString();
        QString timeseries_out=CLP.named_parameters["timeseries_out"].toString();
        QStringList channels_str=CLP.named_parameters["channels"].toString().split(",");
        QList<int> channels=MLUtil::stringListToIntList(channels_str);
        ret = p_extract_neighborhood_timeseries(timeseries,timeseries_out,channels);
    }
    else if (arg1=="mountainsort.bandpass_filter") {
        QString timeseries=CLP.named_parameters["timeseries"].toString();
        QString timeseries_out=CLP.named_parameters["timeseries_out"].toString();
        Bandpass_filter_opts opts;
        opts.samplerate=CLP.named_parameters["samplerate"].toDouble();
        opts.freq_min=CLP.named_parameters["freq_min"].toDouble();
        opts.freq_max=CLP.named_parameters["freq_max"].toDouble();
        opts.freq_wid=CLP.named_parameters["freq_wid"].toDouble();
        ret = p_bandpass_filter(timeseries,timeseries_out,opts);
    }
    else if (arg1=="mountainsort.detect_events") {
        QString timeseries=CLP.named_parameters["timeseries"].toString();
        QString event_times_out=CLP.named_parameters["event_times_out"].toString();
        int central_channel=CLP.named_parameters["central_channel"].toInt();
        double detect_threshold=CLP.named_parameters["detect_threshold"].toDouble();
        double detect_interval=CLP.named_parameters["detect_interval"].toDouble();
        int sign=CLP.named_parameters["sign"].toInt();
        ret = p_detect_events(timeseries,event_times_out,central_channel,detect_threshold,detect_interval,sign);
    }
    else if (arg1=="mountainsort.extract_clips") {
        QString timeseries=CLP.named_parameters["timeseries"].toString();
        QString event_times=CLP.named_parameters["event_times"].toString();
        QString clips_out=CLP.named_parameters["clips_out"].toString();
        ret = p_extract_clips(timeseries,event_times,clips_out,CLP.named_parameters);
    }
    else if (arg1=="mountainsort.sort_clips") {
        QString clips=CLP.named_parameters["clips"].toString();
        QString labels_out=CLP.named_parameters["labels_out"].toString();
        Sort_clips_opts opts;
        ret = p_sort_clips(clips,labels_out,opts);
    }
    else if (arg1=="mountainsort.consolidate_clusters") {
        QString clips=CLP.named_parameters["clips"].toString();
        QString labels=CLP.named_parameters["labels"].toString();
        QString labels_out=CLP.named_parameters["labels_out"].toString();
        Consolidate_clusters_opts opts;
        opts.central_channel=CLP.named_parameters["central_channel"].toInt();
        ret = p_consolidate_clusters(clips,labels,labels_out,opts);
    }
    else if (arg1=="mountainsort.create_firings") {
        QString event_times=CLP.named_parameters["event_times"].toString();
        QString labels=CLP.named_parameters["labels"].toString();
        QString firings_out=CLP.named_parameters["firings_out"].toString();
        int central_channel=CLP.named_parameters["central_channel"].toInt();
        ret = p_create_firings(event_times,labels,firings_out,central_channel);
    }
    else if (arg1=="mountainsort.combine_firings") {
        QStringList firings_list=MLUtil::toStringList(CLP.named_parameters["firings_list"]);
        QString firings_out=CLP.named_parameters["firings_out"].toString();
        ret = p_combine_firings(firings_list,firings_out);
    }
    else if (arg1=="mountainsort.fit_stage") {
        QString timeseries=CLP.named_parameters["timeseries"].toString();
        QString firings=CLP.named_parameters["firings"].toString();
        QString firings_out=CLP.named_parameters["firings_out"].toString();
        Fit_stage_opts opts;
        ret = p_fit_stage(timeseries,firings,firings_out,opts);
    }
    else {
        qWarning() << "Unexpected processor name: "+arg1;
        return -1;
    }

    if (!ret) return -1;

    return 0;
}


QJsonObject ProcessorSpecFile::get_spec() {
    QJsonObject ret;
    ret["name"]=name;
    ret["description"]=description;
    return ret;
}

QJsonObject ProcessorSpecParam::get_spec() {
    QJsonObject ret;
    ret["name"]=name;
    ret["description"]=description;
    ret["optional"]=optional;
    return ret;
}

void ProcessorSpec::addInputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addInput(name1);
    if (!name2.isEmpty()) addInput(name2);
    if (!name3.isEmpty()) addInput(name3);
    if (!name4.isEmpty()) addInput(name4);
    if (!name5.isEmpty()) addInput(name5);
}

void ProcessorSpec::addOutputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addOutput(name1);
    if (!name2.isEmpty()) addOutput(name2);
    if (!name3.isEmpty()) addOutput(name3);
    if (!name4.isEmpty()) addOutput(name4);
    if (!name5.isEmpty()) addOutput(name5);
}

void ProcessorSpec::addRequiredParameters(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addRequiredParameter(name1);
    if (!name2.isEmpty()) addRequiredParameter(name2);
    if (!name3.isEmpty()) addRequiredParameter(name3);
    if (!name4.isEmpty()) addRequiredParameter(name4);
    if (!name5.isEmpty()) addRequiredParameter(name5);
}

void ProcessorSpec::addOptionalParameters(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addOptionalParameter(name1);
    if (!name2.isEmpty()) addOptionalParameter(name2);
    if (!name3.isEmpty()) addOptionalParameter(name3);
    if (!name4.isEmpty()) addOptionalParameter(name4);
    if (!name5.isEmpty()) addOptionalParameter(name5);
}

void ProcessorSpec::addInput(QString name,QString description)
{
    ProcessorSpecFile X;
    X.name=name;
    X.description=description;
    inputs.append(X);
}

void ProcessorSpec::addOutput(QString name,QString description)
{
    ProcessorSpecFile X;
    X.name=name;
    X.description=description;
    outputs.append(X);
}

void ProcessorSpec::addRequiredParameter(QString name,QString description)
{
    ProcessorSpecParam X;
    X.name=name;
    X.description=description;
    X.optional=false;
    parameters.append(X);
}

void ProcessorSpec::addOptionalParameter(QString name,QString description)
{
    ProcessorSpecParam X;
    X.name=name;
    X.description=description;
    X.optional=true;
    parameters.append(X);
}

QJsonObject ProcessorSpec::get_spec() {
    QJsonObject ret;
    ret["name"]=processor_name;
    ret["version"]=version;
    ret["description"]=description;
    QJsonArray inputs0;
    for (int i=0; i<inputs.count(); i++) {
        inputs0.push_back(inputs[i].get_spec());
    }
    ret["inputs"]=inputs0;
    QJsonArray outputs0;
    for (int i=0; i<outputs.count(); i++) {
        outputs0.push_back(outputs[i].get_spec());
    }
    ret["outputs"]=outputs0;
    QJsonArray parameters0;
    for (int i=0; i<parameters.count(); i++) {
        parameters0.push_back(parameters[i].get_spec());
    }
    ret["parameters"]=parameters0;
    ret["exe_command"]=qApp->applicationFilePath()+" "+processor_name+" $(arguments)";
    return ret;
}
