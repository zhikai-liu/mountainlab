/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/

#include "mlcommon.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include "mountainsort3_main.h"
//#include "p_multineighborhood_sort.h"
#include "p_preprocess.h"
#include "p_mountainsort3.h"
#include "p_run_metrics_script.h"
#include "p_spikeview_metrics.h"
#include "p_spikeview_templates.h"
#include "omp.h"
#include "p_synthesize_timeseries.h"

QJsonObject get_spec()
{
    QJsonArray processors;

    /*
    {
        ProcessorSpec X("mountainsort.multineighborhood_sort", "0.15j");
        X.addInputs("timeseries", "geom");
        X.addOutputs("firings_out");
        X.addOptionalParameter("adjacency_radius", "", 0);
        X.addOptionalParameter("consolidate_clusters", "", "true");
        X.addOptionalParameter("consolidation_factor", "", 0.9);
        X.addOptionalParameter("clip_size", "", 50);
        X.addOptionalParameter("detect_interval", "", 10);
        X.addOptionalParameter("detect_threshold", "", 3);
        X.addOptionalParameter("detect_sign", "", 0);
        X.addOptionalParameter("merge_across_channels", "", "true");
        X.addOptionalParameter("fit_stage", "", "true");
        processors.push_back(X.get_spec());
    }
    */
    {
        ProcessorSpec X("mountainsort.preprocess", "0.1");
        X.addInputs("timeseries");
        X.addOutputs("timeseries_out");
        X.addOptionalParameter("bandpass_filter", "", "true");
        X.addOptionalParameter("freq_min", "", 300);
        X.addOptionalParameter("freq_max", "", 6000);
        X.addOptionalParameter("mask_out_artifacts", "", "false");
        X.addOptionalParameter("mask_out_artifacts_threshold", "", 6);
        X.addOptionalParameter("mask_out_artifacts_interval", "", 2000);
        X.addOptionalParameter("whiten", "", "true");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.mountainsort3", "0.14");
        X.addInputs("timeseries", "geom");
        X.addOutputs("firings_out");
        X.addOptionalParameter("adjacency_radius", "", 0);
        X.addOptionalParameter("consolidate_clusters", "", "true");
        X.addOptionalParameter("consolidation_factor", "", 0.9);
        X.addOptionalParameter("clip_size", "", 50);
        X.addOptionalParameter("detect_interval", "", 10);
        X.addOptionalParameter("detect_threshold", "", 3);
        X.addOptionalParameter("detect_sign", "", 0);
        X.addOptionalParameter("merge_across_channels", "", "true");
        X.addOptionalParameter("fit_stage", "", "true");
        X.addOptionalParameter("t1", "Start timepoint to do the sorting (default 0 means start at beginning)", 0);
        X.addOptionalParameter("t2", "End timepoint for sorting (default -1 means go to end of timeseries)", -1);
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.run_metrics_script", "0.1");
        X.addInputs("metrics", "script");
        X.addOutputs("metrics_out");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("spikeview.metrics1", "0.1");
        X.addInputs("firings");
        X.addOutputs("metrics_out");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("spikeview.templates", "0.16");
        X.addInputs("timeseries", "firings");
        X.addOutputs("templates_out");
        X.addOptionalParameter("clip_size", "", 100);
        X.addOptionalParameter("max_events_per_template", "", 500);
        X.addOptionalParameters("samplerate", "freq_min", "freq_max");
        X.addOptionalParameter("subtract_temporal_mean", "", "false");
        processors.push_back(X.get_spec());
    }
    {
        ProcessorSpec X("mountainsort.synthesize_timeseries", "0.1");
        X.addInputs("firings", "waveforms");
        X.addOutputs("timeseries_out");
        X.addOptionalParameter("noise_level", "", 0);
        X.addOptionalParameter("duration", "", 0);
        X.addOptionalParameter("waveform_upsample_factor", "", 13);
        processors.push_back(X.get_spec());
    }

    QJsonObject ret;
    ret["processors"] = processors;
    return ret;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    CLParams CLP(argc, argv);

    QString arg1 = CLP.unnamed_parameters.value(0);

    if (arg1 == "spec") {
        QJsonObject spec = get_spec();
        QString json = QJsonDocument(spec).toJson(QJsonDocument::Indented);
        printf("%s\n", json.toUtf8().data());
        return 0;
    }

    bool ret = false;

    if (CLP.named_parameters.contains("_request_num_threads")) {
        int num_threads = CLP.named_parameters.value("_request_num_threads", 0).toInt();
        if (num_threads) {
            qDebug().noquote() << "Setting num threads:" << num_threads;
            omp_set_num_threads(num_threads);
        }
    }

    /*
    if (arg1 == "mountainsort.multineighborhood_sort") {
        QString timeseries = CLP.named_parameters["timeseries"].toString();
        QString geom = CLP.named_parameters["geom"].toString();
        QString firings_out = CLP.named_parameters["firings_out"].toString();
        P_multineighborhood_sort_opts opts;
        opts.adjacency_radius = CLP.named_parameters.value("adjacency_radius").toDouble();
        opts.consolidate_clusters = (CLP.named_parameters.value("consolidate_clusters").toString() == "true");
        opts.consolidation_factor = CLP.named_parameters.value("consolidation_factor").toDouble();
        opts.clip_size = CLP.named_parameters.value("clip_size").toDouble();
        opts.detect_interval = CLP.named_parameters.value("detect_interval").toDouble();
        opts.detect_threshold = CLP.named_parameters.value("detect_threshold").toDouble();
        opts.detect_sign = CLP.named_parameters.value("detect_sign").toInt();
        opts.merge_across_channels = (CLP.named_parameters.value("merge_across_channels").toString() == "true");
        opts.fit_stage = (CLP.named_parameters.value("fit_stage").toString() == "true");
        QString temp_path = CLP.named_parameters.value("_tempdir").toString();
        ret = p_multineighborhood_sort(timeseries, geom, firings_out, temp_path, opts);
    }
    */
    if (arg1 == "mountainsort.preprocess") {
        QString timeseries = CLP.named_parameters["timeseries"].toString();
        QString timeseries_out = CLP.named_parameters["timeseries_out"].toString();
        P_preprocess_opts opts;
        opts.bandpass_filter = (CLP.named_parameters.value("bandpass_filter").toString() == "true");
        opts.freq_min = CLP.named_parameters.value("freq_min").toDouble();
        opts.freq_max = CLP.named_parameters.value("freq_max").toDouble();
        opts.mask_out_artifacts = (CLP.named_parameters.value("mask_out_artifacts").toString() == "true");
        opts.mask_out_artifacts_threshold = CLP.named_parameters.value("mask_out_artifacts_threshold").toDouble();
        opts.mask_out_artifacts_interval = CLP.named_parameters.value("mask_out_artifacts_interval").toDouble();
        opts.whiten = (CLP.named_parameters.value("whiten").toString() == "true");
        QString temp_path = CLP.named_parameters.value("_tempdir").toString();
        ret = p_preprocess(timeseries, timeseries_out, temp_path, opts);
    }
    else if (arg1 == "mountainsort.mountainsort3") {
        QString timeseries = CLP.named_parameters["timeseries"].toString();
        QString geom = CLP.named_parameters["geom"].toString();
        QString firings_out = CLP.named_parameters["firings_out"].toString();
        P_mountainsort3_opts opts;
        opts.adjacency_radius = CLP.named_parameters.value("adjacency_radius").toDouble();
        opts.consolidate_clusters = (CLP.named_parameters.value("consolidate_clusters").toString() == "true");
        opts.consolidation_factor = CLP.named_parameters.value("consolidation_factor").toDouble();
        opts.clip_size = CLP.named_parameters.value("clip_size").toDouble();
        opts.detect_interval = CLP.named_parameters.value("detect_interval").toDouble();
        opts.detect_threshold = CLP.named_parameters.value("detect_threshold").toDouble();
        opts.detect_sign = CLP.named_parameters.value("detect_sign").toInt();
        opts.merge_across_channels = (CLP.named_parameters.value("merge_across_channels").toString() == "true");
        opts.fit_stage = (CLP.named_parameters.value("fit_stage").toString() == "true");
        opts.t1 = CLP.named_parameters.value("t1").toDouble();
        opts.t2 = CLP.named_parameters.value("t2").toDouble();
        QString temp_path = CLP.named_parameters.value("_tempdir").toString();
        ret = p_mountainsort3(timeseries, geom, firings_out, temp_path, opts);
    }
    else if (arg1 == "mountainsort.run_metrics_script") {
        QString metrics = CLP.named_parameters["metrics"].toString();
        QString script = CLP.named_parameters["script"].toString();
        QString metrics_out = CLP.named_parameters["metrics_out"].toString();
        ret = p_run_metrics_script(metrics, script, metrics_out);
    }
    else if (arg1 == "spikeview.metrics1") {
        QString firings = CLP.named_parameters["firings"].toString();
        QString metrics_out = CLP.named_parameters["metrics_out"].toString();
        ret = p_spikeview_metrics1(firings, metrics_out);
    }
    else if (arg1 == "spikeview.templates") {
        QString timeseries = CLP.named_parameters["timeseries"].toString();
        QString firings = CLP.named_parameters["firings"].toString();
        QString templates_out = CLP.named_parameters["templates_out"].toString();
        P_spikeview_templates_opts opts;
        opts.clip_size = CLP.named_parameters["clip_size"].toInt();
        opts.max_events_per_template = CLP.named_parameters["max_events_per_template"].toDouble();
        opts.samplerate = CLP.named_parameters.value("samplerate", 0).toDouble();
        opts.freq_min = CLP.named_parameters.value("freq_min", 0).toDouble();
        opts.freq_max = CLP.named_parameters.value("freq_max", 0).toDouble();
        if ((opts.samplerate != 0) && ((opts.freq_min != 0) || (opts.freq_max != 0)))
            opts.filt_padding = 50;
        opts.subtract_temporal_mean = (CLP.named_parameters.value("subtract_temporal_mean") == "true");
        ret = p_spikeview_templates(timeseries, firings, templates_out, opts);
    }
    else if (arg1 == "mountainsort.synthesize_timeseries") {
        QString firings = CLP.named_parameters["firings"].toString();
        QString waveforms = CLP.named_parameters["waveforms"].toString();
        QString timeseries_out = CLP.named_parameters["timeseries_out"].toString();
        P_synthesize_timeseries_opts opts;
        opts.noise_level = CLP.named_parameters["noise_level"].toDouble();
        opts.duration = CLP.named_parameters["duration"].toDouble();
        opts.waveform_upsample_factor = CLP.named_parameters.value("waveform_upsample_factor",13).toDouble();
        ret = p_synthesize_timeseries(firings, waveforms, timeseries_out, opts);
    }
    else {
        qWarning() << "Unexpected processor name: " + arg1;
        return -1;
    }

    if (!ret)
        return -1;

    return 0;
}

QJsonObject ProcessorSpecFile::get_spec()
{
    QJsonObject ret;
    ret["name"] = name;
    ret["description"] = description;
    ret["optional"] = optional;
    return ret;
}

QJsonObject ProcessorSpecParam::get_spec()
{
    QJsonObject ret;
    ret["name"] = name;
    ret["description"] = description;
    ret["optional"] = optional;
    if (default_value.isValid())
        ret["default_value"] = QJsonValue::fromVariant(default_value);
    return ret;
}

void ProcessorSpec::addInputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addInput(name1);
    if (!name2.isEmpty())
        addInput(name2);
    if (!name3.isEmpty())
        addInput(name3);
    if (!name4.isEmpty())
        addInput(name4);
    if (!name5.isEmpty())
        addInput(name5);
}

void ProcessorSpec::addOptionalInputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addInput(name1, "", true);
    if (!name2.isEmpty())
        addInput(name2, "", true);
    if (!name3.isEmpty())
        addInput(name3, "", true);
    if (!name4.isEmpty())
        addInput(name4, "", true);
    if (!name5.isEmpty())
        addInput(name5, "", true);
}

void ProcessorSpec::addOutputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addOutput(name1);
    if (!name2.isEmpty())
        addOutput(name2);
    if (!name3.isEmpty())
        addOutput(name3);
    if (!name4.isEmpty())
        addOutput(name4);
    if (!name5.isEmpty())
        addOutput(name5);
}

void ProcessorSpec::addOptionalOutputs(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addOutput(name1, "", true);
    if (!name2.isEmpty())
        addOutput(name2, "", true);
    if (!name3.isEmpty())
        addOutput(name3, "", true);
    if (!name4.isEmpty())
        addOutput(name4, "", true);
    if (!name5.isEmpty())
        addOutput(name5, "", true);
}

void ProcessorSpec::addRequiredParameters(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addRequiredParameter(name1);
    if (!name2.isEmpty())
        addRequiredParameter(name2);
    if (!name3.isEmpty())
        addRequiredParameter(name3);
    if (!name4.isEmpty())
        addRequiredParameter(name4);
    if (!name5.isEmpty())
        addRequiredParameter(name5);
}

void ProcessorSpec::addOptionalParameters(QString name1, QString name2, QString name3, QString name4, QString name5)
{
    addOptionalParameter(name1);
    if (!name2.isEmpty())
        addOptionalParameter(name2);
    if (!name3.isEmpty())
        addOptionalParameter(name3);
    if (!name4.isEmpty())
        addOptionalParameter(name4);
    if (!name5.isEmpty())
        addOptionalParameter(name5);
}

void ProcessorSpec::addInput(QString name, QString description, bool optional)
{
    ProcessorSpecFile X;
    X.name = name;
    X.description = description;
    X.optional = optional;
    inputs.append(X);
}

void ProcessorSpec::addOutput(QString name, QString description, bool optional)
{
    ProcessorSpecFile X;
    X.name = name;
    X.description = description;
    X.optional = optional;
    outputs.append(X);
}

void ProcessorSpec::addRequiredParameter(QString name, QString description)
{
    ProcessorSpecParam X;
    X.name = name;
    X.description = description;
    X.optional = false;
    parameters.append(X);
}

void ProcessorSpec::addOptionalParameter(QString name, QString description, QVariant default_value)
{
    ProcessorSpecParam X;
    X.name = name;
    X.description = description;
    X.optional = true;
    X.default_value = default_value;
    parameters.append(X);
}

QJsonObject ProcessorSpec::get_spec()
{
    QJsonObject ret;
    ret["name"] = processor_name;
    ret["version"] = version;
    ret["description"] = description;
    QJsonArray inputs0;
    for (int i = 0; i < inputs.count(); i++) {
        inputs0.push_back(inputs[i].get_spec());
    }
    ret["inputs"] = inputs0;
    QJsonArray outputs0;
    for (int i = 0; i < outputs.count(); i++) {
        outputs0.push_back(outputs[i].get_spec());
    }
    ret["outputs"] = outputs0;
    QJsonArray parameters0;
    for (int i = 0; i < parameters.count(); i++) {
        parameters0.push_back(parameters[i].get_spec());
    }
    ret["parameters"] = parameters0;
    ret["exe_command"] = qApp->applicationFilePath() + " " + processor_name + " $(arguments)";
    return ret;
}
