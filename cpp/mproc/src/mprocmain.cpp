#include "mprocmain.h"
#include "processormanager.h"

#include <QCoreApplication>
#include <mlcommon.h>
#include <objectregistry.h>
#include <qprocessmanager.h>
#include <QDir>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include "cachemanager.h"

#include "signal.h"
#include <unistd.h>

Q_LOGGING_CATEGORY(MP, "mproc.main")

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    CLParams CLP(argc, argv);

    ObjectRegistry registry;

    //The qprocess manager (very different from the ProcessManager!)
    QProcessManager* qprocessManager = new QProcessManager;
    registry.addAutoReleasedObject(qprocessManager);
    signal(SIGINT, sig_handler);
    signal(SIGKILL, sig_handler);
    signal(SIGTERM, sig_handler);

    // If _working_path is specified then we change the current directory
    QString working_path = CLP.named_parameters.value("_working_path").toString();
    if (working_path.isEmpty())
        working_path = QDir::currentPath();
    if (!working_path.isEmpty()) {
        if (!QDir::setCurrent(working_path)) {
            qCWarning(MP) << "Unable to set working path to: " << working_path;
            return -1;
        }
    }

    // for convenience
    QString arg1 = CLP.unnamed_parameters.value(0);
    QString arg2 = CLP.unnamed_parameters.value(1);
    QString arg3 = CLP.unnamed_parameters.value(2);

    // this may have been important
    setbuf(stdout, NULL);

    if (arg1 == "list-processors") { //Provide a human-readable list of the available processors
        if (list_processors())
            return 0;
        else
            return -1;
    }
    else if (arg1 == "spec") { // Show the spec for a single processor
        if (spec(arg2))
            return 0;
        else
            return -1;
    }
    else if ((arg1 == "exec")||(arg1 == "run")||(arg1 == "queue")) {
        return exec_run_or_queue(arg1,arg2,CLP.named_parameters);
    }
    /*
    else if ((arg1 == "run-process") || (arg1 == "exec-process")) { //Run a process synchronously
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        bool exec_mode = (arg1 == "exec-process");
        //printf("Initializing process manager...\n");
        if (!initialize_process_manager()) { // load the processor plugins etc
            //log_end();
            return -1;
        }
        QString output_fname = CLP.named_parameters.value("_process_output").toString(); //maybe the user/framework specified where output is to be saved
        if (!output_fname.isEmpty()) {
            output_fname = QDir::current().absoluteFilePath(output_fname); //make it absolute
        }
        QString processor_name = arg2; //name of the processor is the second user-supplied arg
        QVariantMap process_parameters = CLP.named_parameters;
        remove_system_parameters(process_parameters); //remove parameters starting with "_"
        int ret = 0;
        QString error_message;
        MLProcessInfo info;
        int parent_pid = CLP.named_parameters.value("_parent_pid", 0).toLongLong();

        bool preserve_tempdir = CLP.named_parameters.contains("_preserve_tempdir");

        bool force_run = CLP.named_parameters.contains("_force_run"); // do not check for already computed
        int request_num_threads = CLP.named_parameters.value("_request_num_threads", 0).toInt(); // the processor may or may not respect this request. But mountainsort/omp does.
        PM->setDefaultParameters(processor_name, process_parameters);
        if ((!force_run) && (!exec_mode) && (PM->processAlreadyCompleted(processor_name, process_parameters))) {
            // We have a record of this procesor already completed. If so, we save a lot of time by not re-running
            qCInfo(MP) << "Process already completed: " + processor_name;
            printf("Process already completed: %s\n", processor_name.toLatin1().data());
        }
        else {
            QString id;

            //check to see that all our parameters are in order for the given processor
            if (!PM->checkParameters(processor_name, process_parameters)) {
                error_message = "Problem checking process: " + processor_name;
                ret = -1;
            }
            else {
                RequestProcessResources RPR;
                RPR.request_num_threads = request_num_threads;
                qCInfo(MP) << "Starting process: " + processor_name;
                id = PM->startProcess(processor_name, process_parameters, RPR, exec_mode, preserve_tempdir); //start the process and retrieve a unique id
                if (id.isEmpty()) {
                    error_message = "Problem starting process: " + processor_name;
                    ret = -1;
                }
                if (!PM->waitForFinished(id, parent_pid)) { //wait for the process to finish
                    error_message = "Problem waiting for process to finish: " + processor_name;
                    ret = -1;
                }
                info = PM->processInfo(id); //get the info about the process from the process manager
                if (info.exit_status == QProcess::CrashExit) {
                    error_message = "Process crashed: " + processor_name;
                    ret = -1;
                }
                if (info.exit_code != 0) {
                    error_message = "Exit code is non-zero: " + processor_name;
                    ret = -1;
                }
                PM->clearProcess(id); //clean up
            }

            printf("---------------------------------------------------------------\n");
            printf("PROCESS COMPLETED (exit code = %d): %s\n", info.exit_code, info.processor_name.toLatin1().data());
            if (!error_message.isEmpty())
                printf("ERROR: %s\n", error_message.toLatin1().data());
            int mb = compute_peak_mem_bytes(info.monitor_stats) / 1000000;
            double cpu = compute_peak_cpu_pct(info.monitor_stats);
            double cpu_avg = compute_avg_cpu_pct(info.monitor_stats);
            double sec = info.start_time.msecsTo(info.finish_time) * 1.0 / 1000;
            printf("Peak RAM: %d MB. Peak CPU: %g%%. Avg CPU: %g%%. Elapsed time: %g seconds.\n", mb, cpu, cpu_avg, sec);
            printf("---------------------------------------------------------------\n");
        }
        QJsonObject obj; //the output info to be saved
        obj["exe_command"] = info.exe_command;
        obj["exit_code"] = info.exit_code;
        if (info.exit_status == QProcess::CrashExit)
            obj["exit_status"] = "CrashExit";
        else
            obj["exit_status"] = "NormalExit";
        obj["finished"] = QJsonValue(info.finished);
        obj["parameters"] = variantmap_to_json_obj(info.parameters);
        obj["processor_name"] = info.processor_name;
        obj["standard_output"] = QString(info.standard_output);
        obj["standard_error"] = QString(info.standard_error);
        obj["success"] = error_message.isEmpty();
        obj["error"] = error_message;
        obj["peak_mem_bytes"] = (int)compute_peak_mem_bytes(info.monitor_stats);
        obj["peak_cpu_pct"] = compute_peak_cpu_pct(info.monitor_stats);
        obj["avg_cpu_pct"] = compute_avg_cpu_pct(info.monitor_stats);
        obj["start_time"] = info.start_time.toString("yyyy-MM-dd:hh-mm-ss.zzz");
        obj["finish_time"] = info.finish_time.toString("yyyy-MM-dd:hh-mm-ss.zzz");
        //obj["monitor_stats"]=monitor_stats_to_json_array(info.monitor_stats); -- at some point we can include this in the file. For now we only worry about the computed peak values
        if (!output_fname.isEmpty()) { //The user wants the results to go in this file
            qCInfo(MP) << QString("Writing process output to file: %1 (%2)").arg(output_fname).arg(processor_name);
            QFile::remove(output_fname); //important -- added 9/9/16
            QString obj_json = QJsonDocument(obj).toJson();
            if (!TextFile::write(output_fname, obj_json)) {
                qCCritical(MP) << "Unable to write results to: " + output_fname;
                return -1;
            }
        }
        else {
            qCInfo(MP) << "Not writing process output to any file because output_fname is empty.";
        }
        if (!error_message.isEmpty()) {
            qCCritical(MP) << "Error in mountainprocessmain" << error_message;
        }
        printf("\n");

        //log_end();
        return ret; //returns exit code 0 if okay
    }
    else if (arg1 == "run-script") { // run a script synchronously (although note that individual processes will be queued (unless --_nodaemon is specified), but the script will wait for them to complete before exiting)
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        QString daemon_id = qgetenv("MP_DAEMON_ID");
        if (!CLP.named_parameters.contains("_nodaemon")) {
            if (daemon_id.isEmpty()) {
                printf("*****************************************************************\n");
                printf("Cannot run script because...\n");
                printf("\nNo daemon has been specified.\n");
                printf("Use the --_nodaemon flag or do the following:\n");
                print_daemon_instructions();
                return -1;
            }
            QJsonObject state = get_daemon_state(daemon_id);
            if (!state["is_running"].toBool()) {
                printf("*****************************************************************\n");
                printf("Cannot run script because...\n");
                printf("\nDaemon \"%s\" is not running.\n", daemon_id.toUtf8().data());
                printf("Use the --_nodaemon flag or do the following:\n");
                print_daemon_instructions();
                return -1;
            }
        }

        if (!initialize_process_manager()) { // load the processor plugins etc
            //log_end();
            return -1;
        }

        ProcessManager::globalInstance()->cleanUpCompletedProcessRecords(); //those .json files in completed_process tmp directory

        QString output_fname = CLP.named_parameters.value("_script_output").toString(); //maybe the user or framework specified where output is to be saved
        if (!output_fname.isEmpty()) {
            output_fname = QDir::current().absoluteFilePath(output_fname); //make it absolute
        }

        int ret = 0;
        QString error_message;

        QStringList script_fnames; //names of the javascript source files
        QVariantMap params; //parameters to be passed into the main() function of the javascript
        for (int i = 0; i < CLP.unnamed_parameters.count(); i++) {
            QString str = CLP.unnamed_parameters[i];
            if ((str.endsWith(".js")) || (str.endsWith(".pipeline"))) { // it is a javascript source file
                if (!QFile::exists(str)) {
                    // a final holdout for mountainlabBasePath(), but scripts will go away anyway
                    QString str2 = MLUtil::mountainlabBasePath() + "/mountainprocess/scripts/" + str;
                    if (QFile::exists(str2)) {
                        str = str2;
                    }
                }
                if (QFile::exists(str)) {
                    script_fnames << str;
                }
                else {
                    qCCritical(MP) << "Unable to find script file: " + str;
                    //log_end();
                    return -1;
                }
            }
            if ((str.endsWith(".par")) || (str.endsWith(".json"))) { // note that we can have multiple parameter files! the later ones override the earlier ones.
                if (!load_parameter_file(params, str)) {
                    qCWarning(MP) << "Unable to load parameter file: " + str;
                    //log_end();
                    return -1;
                }
            }
        }
        // load the command-line parameters into params. These will override parameters set by .par or .json files
        QStringList keys0 = CLP.named_parameters.keys();
        foreach (QString key0, keys0) {
            params[key0] = CLP.named_parameters[key0];
        }
        remove_system_parameters(params);

        run_script_opts opts;
        opts.nodaemon = CLP.named_parameters.contains("_nodaemon");
        opts.force_run = CLP.named_parameters.contains("_force_run");
        opts.working_path = QDir::currentPath(); // this should get passed through to the processors
        opts.preserve_tempdir = CLP.named_parameters.contains("_preserve_tempdir");
        QJsonObject results;
        // actually run the script
        if (!run_script(script_fnames, params, opts, error_message, results)) {
            ret = -1;
        }

        QJsonObject obj; //the output info
        obj["script_fnames"] = QJsonArray::fromStringList(script_fnames);
        obj["parameters"] = variantmap_to_json_obj(params);
        obj["success"] = (ret == 0);
        obj["results"] = results;
        obj["error"] = error_message;
        if (!output_fname.isEmpty()) { //The user wants the results to go in this file
            QFile::remove(output_fname); //important -- added 9/9/16
            QString obj_json = QJsonDocument(obj).toJson();
            if (!TextFile::write(output_fname, obj_json)) {
                qCCritical(MP) << "Unable to write results to: " + output_fname;
                return -1;
            }
        }

        QJsonArray PP = results["processes"].toArray();

        if (ret == 0) { //success

            printf("\nPeak Memory (MB):\n");
            for (int i = 0; i < PP.count(); i++) {
                QJsonObject QQ = PP[i].toObject();
                QJsonObject RR = QQ["results"].toObject();
                QString processor_name = QQ["processor_name"].toString();
                double mem = RR["peak_mem_bytes"].toDouble() / 1e6;
                if (!processor_name.isEmpty()) {
                    QString tmp = QString("  %1 (%2)").arg(mem).arg(processor_name);
                    printf("%s\n", tmp.toUtf8().data());
                }
            }

            printf("\nPeak CPU percent:\n");
            for (int i = 0; i < PP.count(); i++) {
                QJsonObject QQ = PP[i].toObject();
                QJsonObject RR = QQ["results"].toObject();
                QString processor_name = QQ["processor_name"].toString();
                double cpu = RR["peak_cpu_pct"].toDouble();
                if (!processor_name.isEmpty()) {
                    QString tmp = QString("  %1 (%2)").arg(cpu).arg(processor_name);
                    printf("%s\n", tmp.toUtf8().data());
                }
            }

            printf("\nAvg CPU (pct):\n");
            for (int i = 0; i < PP.count(); i++) {
                QJsonObject QQ = PP[i].toObject();
                QJsonObject RR = QQ["results"].toObject();
                QString processor_name = QQ["processor_name"].toString();
                double cpu = RR["avg_cpu_pct"].toDouble();
                if (!processor_name.isEmpty()) {
                    QString tmp = QString("  %1 (%2)").arg(cpu).arg(processor_name);
                    printf("%s\n", tmp.toUtf8().data());
                }
            }

            printf("\nElapsed time (sec):\n");
            for (int i = 0; i < PP.count(); i++) {
                QJsonObject QQ = PP[i].toObject();
                QJsonObject RR = QQ["results"].toObject();
                QString processor_name = QQ["processor_name"].toString();
                QDateTime start_time = QDateTime::fromString(RR["start_time"].toString(), "yyyy-MM-dd:hh-mm-ss.zzz");
                QDateTime finish_time = QDateTime::fromString(RR["finish_time"].toString(), "yyyy-MM-dd:hh-mm-ss.zzz");
                double elapsed = start_time.msecsTo(finish_time) * 1.0 / 1000;
                if (!processor_name.isEmpty()) {
                    QString tmp = QString("  %1 (%2)").arg(elapsed).arg(processor_name);
                    printf("%s\n", tmp.toUtf8().data());
                }
            }
        }

        //log_end();
        return ret;
    }
    else if (arg1 == "set-default-daemon") {
        set_default_daemon_id(arg2);
        printf("default daemon id is set to: ");
        print_default_daemon_id();
        return 0;
    }
    else if (arg1 == "get-default-daemon") {
        print_default_daemon_id();
        return 0;
    }
    else if (arg1 == "queue-script") { //Queue a script -- to be executed when resources are available
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        QString daemon_id = qgetenv("MP_DAEMON_ID");
        if (daemon_id.isEmpty()) {
            printf("\nCANNOT QUEUE SCRIPT: No daemon has been specified.\n");
            printf("Use the --_nodaemon flag or do the following:\n");
            print_daemon_instructions();
            return -1;
        }

        if (queue_pript(ScriptType, CLP, working_path)) {
            //log_end();
            return 0;
        }
        else {
            //log_end();
            return -1;
        }
    }
    else if (arg1 == "queue-process") {
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        QString daemon_id = qgetenv("MP_DAEMON_ID");
        if (daemon_id.isEmpty()) {
            printf("\nCANNOT QUEUE PROCESS: No daemon has been specified.\n");
            printf("Use the --_nodaemon flag or do the following:\n");
            print_daemon_instructions();
            return -1;
        }

        if (!initialize_process_manager()) { // load the processor plugins etc... needed because we check to see if the processor exists, and attach the spec (for later checking), before it is queued
            //log_end();
            return -1;
        }

        if (queue_pript(ProcessType, CLP, working_path)) {
            //log_end();
            return 0;
        }
        else {
            //log_end();
            return -1;
        }
    }
    else if (arg1 == "list-daemons") {
        // hack by jfm to temporarily implement mp-list-daemons
        {
            QSettings settings("Magland", "MountainLab");
            QStringList candidates = settings.value("mp-list-daemons-candidates").toStringList();
            foreach (QString candidate, candidates) {
                qputenv("MP_DAEMON_ID", candidate.toUtf8().data());
                MPDaemonClient client;
                QJsonObject state = client.state();
                if (!state.isEmpty()) {
                    printf("%s\n", candidate.toUtf8().data());
                }
            }
        }
        return 0;
    }
    else if (arg1 == "daemon-start") {
        QString daemon_id = arg2;

        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("daemon-start [some_id]\n");
            printf("For example you can use your user name\n");
            exit(-1);
        }
        /// Witold, I am changing the behavior to the following so that there is no interaction with stdin. I believe this is best.
        if (daemon_id != get_default_daemon_id()) {
            QString msg;
            if (get_default_daemon_id().isEmpty())
                msg = "Setting default daemon to: " + daemon_id;
            else
                msg = QString("Changing default daemon id from %1 to %2. To change it back use mp-set-default-daemon-id.").arg(get_default_daemon_id()).arg(daemon_id);
            printf("%s\n", msg.toUtf8().data());
            set_default_daemon_id(daemon_id);
        }
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        bool do_fork = (!CLP.named_parameters.contains("_nofork"));

        // Start the mountainprocess daemon
        if (do_fork) {
#ifndef NO_FORK
         //  The following magic ensures we detach from the parent process
         //  and from the controlling terminal. This is to prevent process
         //  that spawned us to wait for our children to complete.
#ifdef Q_OS_LINUX
            // fork, setsid(?), redirect stdout to /dev/null
            if (daemon(1, 0)) {
                exit(1);
            }
#else
            // fork, setsid, fork, redirect stdout to /dev/null
            if (fork() > 0) {
                exit(0);
            }
            Q_UNUSED(setsid());
            if (fork() > 0) {
                exit(0);
            }
            int devnull = open("/dev/null", O_WRONLY);
            Q_UNUSED(dup2(devnull, STDOUT_FILENO));
            Q_UNUSED(dup2(devnull, STDERR_FILENO));
#endif
#endif
        }
        qDebug().noquote() << "Initializing process manager...";
        if (!initialize_process_manager()) { // load the processor plugins etc
            //log_end();
            return -1;
        }
        //QString mdaserver_base_path = MLUtil::configResolvedPath("mountainprocess", "mdaserver_base_path");
        //QString mdachunk_data_path = MLUtil::configResolvedPath("mountainprocess", "mdachunk_data_path");
        //figure out what to do about this cleaner business

        //MPDaemonMonitorInterface::setMPDaemonMonitorUrl("http://mpdaemonmonitor.herokuapp.com");
        MPDaemonMonitorInterface::setMPDaemonMonitorUrl(MLUtil::configValue("mountainprocess", "mpdaemonmonitor_url").toString());
        MPDaemonMonitorInterface::setDaemonName(MLUtil::configValue("mountainprocess", "mpdaemon_name").toString());
        MPDaemonMonitorInterface::setDaemonSecret(MLUtil::configValue("mountainprocess", "mpdaemon_secret").toString());

        MountainProcessServer server;
        server.setLogPath(log_path);

        ProcessResources RR; // these are the rules for determining how many processes to run simultaneously
        //RR.num_threads = qMax(0.0, MLUtil::configValue("mountainprocess", "max_num_simultaneous_threads").toDouble());
        //RR.memory_gb = qMax(0.0, MLUtil::configValue("mountainprocess", "max_total_memory_gb").toDouble());
        RR.num_threads = 0;
        RR.memory_gb = 0;
        RR.num_processes = MLUtil::configValue("mountainprocess", "max_num_simultaneous_processes").toDouble();
        server.setTotalResourcesAvailable(RR);
        qDebug().noquote() << "Starting server...";
        if (!server.start())
            return -1;
        return 0;
    }
    else if (arg1 == "daemon-stop") { //Stop the daemon
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            daemon_id = get_default_daemon_id();
        }
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("daemon-stop [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        MPDaemonClient client;
        return client.stop() ? 0 : -1;
    }
    else if (arg1 == "daemon-restart") { //Restart the daemon
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            daemon_id = get_default_daemon_id();
        }
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("daemon-restart [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        MPDaemonClient client;
        if (!client.stop() || !client.start())
            return -1;
        printf("Daemon has been restarted.\n");
        return 0;
    }
    else if (arg1 == "handle-request") {
        QString request_fname = arg2;
        QString response_fname = arg3;
        if (request_fname.isEmpty()) {
            print_usage();
            return -1;
        }
        QJsonObject response;
        response["success"] = false; //assume the worst

        QString prvbucket_path = CLP.named_parameters["prvbucket_path"].toString();

        QString request_json = TextFile::read(request_fname);
        if (request_json.isEmpty()) {
            response["error"] = "Request file is empty or does not exist.";
        }
        else {
            QJsonParseError parse_error;
            QJsonObject request = QJsonDocument::fromJson(request_json.toUtf8(), &parse_error).object();
            if (parse_error.error != QJsonParseError::NoError) {
                response["error"] = "Error parsing request json.";
            }
            else {
                if (!initialize_process_manager()) { // load the processor plugins etc
                    response["error"] = "Failed to initialize process manager.";
                }
                else {
                    response = handle_request(request, prvbucket_path);
                }
            }
        }
        QString response_json = QJsonDocument(response).toJson();
        if (response_fname.isEmpty()) {
            printf("%s\n", response_json.toUtf8().data());
            return 0;
        }
        else {
            if (!TextFile::write(response_fname, response_json)) {
                qCWarning(MP) << "Error writing response to file";
                return -1;
            }
            return 0;
        }
    }
    else if (arg1 == "run-process-remotely") {
        //clean up expired files
        CacheManager::globalInstance()->removeExpiredFiles();

        QString processor_name = arg2; //name of the processor is the second user-supplied arg
        if (processor_name.isEmpty()) {
            qCWarning(MP) << "Processor name is empty";
            return -1;
        }

        qDebug().noquote() << "Running processor remotely: " + processor_name;

        QString server = CLP.named_parameters["_server"].toString();
        if (server.isEmpty()) {
            qCWarning(MP) << "Server is empty";
            return -1;
        }

        QVariantMap process_parameters = CLP.named_parameters;
        remove_system_parameters(process_parameters); //remove parameters starting with "_"

        if (!initialize_process_manager()) { // load the processor plugins etc
            return -1;
        }

        MLProcessor PP = PM->processor(processor_name);
        if (PP.name != processor_name) {
            qCWarning(MP) << "Unable to find processor: " + processor_name;
            return -1;
        }

        QJsonObject request;
        request["action"] = "run_process";
        request["processor_name"] = processor_name;

        QJsonObject inputs;
        foreach (QString key, PP.inputs.keys()) {
            if (process_parameters.contains(key)) {
                QString fname0 = process_parameters[key].toString();
                QJsonObject obj0;
                if (fname0.endsWith(".prv")) {
                    QString json0 = TextFile::read(fname0);
                    QJsonParseError parse_error;
                    obj0 = QJsonDocument::fromJson(json0.toUtf8(), &parse_error).object();
                    if (parse_error.error != QJsonParseError::NoError) {
                        qCWarning(MP) << "Error parsing .prv file: " + fname0;
                        return -1;
                    }
                }
                else {
                    if (!fname0.isEmpty()) {
                        qDebug().noquote() << "Creating prv object for: " + fname0;
                        obj0 = MLUtil::createPrvObject(fname0);
                        //qCWarning(MP) << "Inputs must all be .prv files when using remote processing";
                        //return -1;
                    }
                }
                inputs[key] = obj0;
            }
            else {
                if (!PP.inputs[key].optional) {
                    qCWarning(MP) << "Missing required input: " + key;
                    return -1;
                }
            }
        }
        request["inputs"] = inputs;

        QJsonObject outputs;
        foreach (QString key, PP.outputs.keys()) {
            if (process_parameters.contains(key)) {
                outputs[key] = true;
            }
            else {
                if (!PP.outputs[key].optional) {
                    qCWarning(MP) << "Missing required output: " + key;
                    return -1;
                }
            }
        }
        request["outputs"] = outputs;

        QJsonObject params;
        foreach (QString key, PP.parameters.keys()) {
            if (process_parameters.contains(key)) {
                params[key] = process_parameters[key].toString();
            }
            else {
                if (!PP.parameters[key].optional) {
                    qCWarning(MP) << "Missing required parameter: " + key;
                    return -1;
                }
            }
        }
        request["parameters"] = params;

        QString request_json = QJsonDocument(request).toJson();

        QString server_url;
        QJsonArray prv_servers = MLUtil::configValue("prv", "servers").toArray();
        for (int k = 0; k < prv_servers.count(); k++) {
            QJsonObject obj = prv_servers[k].toObject();
            if (obj["name"].toString() == server) {
                server_url = obj["host"].toString() + ":" + QString("%1").arg(obj["port"].toInt());
            }
        }

        if (server_url.isEmpty()) {
            qCWarning(MP) << "Unable to find server_url associated with server=" + server;
            return -1;
        }

        QString url = server_url + "/mountainprocess/?a=mountainprocess&mpreq=" + request_json;
        QString txt0 = http_get_text_curl_1(url, 1000 * 60 * 60);

        QJsonObject response;
        {
            QJsonParseError parse_error;
            response = QJsonDocument::fromJson(txt0.toUtf8(), &parse_error).object();
            if (parse_error.error != QJsonParseError::NoError) {
                qDebug().noquote() << txt0;
                qCWarning(MP) << "Error parsing response from " + url;
                return -1;
            }
        }
        if (!response["success"].toBool()) {
            qCWarning(MP) << "Error in response: " + response["error"].toString();
            return -1;
        }
        QJsonObject outputs0 = response["outputs"].toObject();
        foreach (QString key, PP.outputs.keys()) {
            if (process_parameters.contains(key)) {
                QString fname0 = process_parameters[key].toString();
                if (fname0.endsWith(".prv")) {
                    QString json0 = QJsonDocument(outputs0[key].toObject()).toJson();
                    if (!TextFile::write(fname0, json0)) {
                        qCWarning(MP) << "Unable to write to output file: " + fname0;
                        return -1;
                    }
                }
                else {
                    QFile::remove(fname0);
                    if (!download_prv(outputs0[key].toObject(), fname0, server)) {
                        qCWarning(MP) << "Problem downloading output: " + key;
                        return -1;
                    }
                }
            }
        }

        return 0; //returns exit code 0 if okay
    }
    else if (arg1 == "print-log") {
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("print-log [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        MPDaemonClient client;
        QJsonArray arr = client.log();
        QTextStream qout(stdout);
        foreach (QJsonValue v, arr) {
            QJsonObject logRecord = v.toObject();
            qout << logRecord["timestamp"].toString() << '\t'
                 << logRecord["record_type"].toString() << '\t'
                 << QJsonDocument(logRecord["data"].toObject()).toJson() << endl;
        }
        return 0;
    }
    else if (arg1 == "log") {
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("log [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        MPDaemonClient client;
        client.contignousLog();
        return 0;
    }
    else if (arg1 == "daemon-state") { //Print some information on the state of the daemon
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            daemon_id = get_default_daemon_id();
        }
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("daemon-state [some_id]\n");
            exit(-1);
        }
        QJsonObject state = get_daemon_state(daemon_id);
        if (CLP.named_parameters.contains("script_id")) {
            QString script_id = CLP.named_parameters["script_id"].toString();
            state = state.value("scripts").toObject().value(script_id).toObject();
        }
        QString json = QJsonDocument(state).toJson();
        printf("%s", json.toLatin1().constData());
        //log_end();
        return 0;
    }
    else if (arg1 == "daemon-state-summary") { //Print some information on the state of the daemon
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            daemon_id = get_default_daemon_id();
        }
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("daemon-state-summary [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        MPDaemonClient client;
        QString txt = get_daemon_state_summary(client.state());
        printf("%s", txt.toLatin1().constData());
        //log_end();
        return 0;
    }
    else if (arg1 == "clear-processing") { // stop all active processing including running or queued pripts
        QString daemon_id = arg2;
        if (daemon_id.isEmpty()) {
            daemon_id = get_default_daemon_id();
        }
        if (daemon_id.isEmpty()) {
            printf("You must specify a daemon id like this:\n");
            printf("clear-processing [some_id]\n");
            exit(-1);
        }
        qputenv("MP_DAEMON_ID", daemon_id.toUtf8().data());

        MPDaemonClient client;
        client.clearProcessing();
        //log_end();
        return 0;
    }
    else if (arg1 == "get-script") { // print information about a script with given id
        if (!log_path.isEmpty()) {
            QString str = TextFile::read(log_path + "/scripts/" + CLP.named_parameters["id"].toString() + ".json");
            printf("%s", str.toLatin1().data());
        }
    }
    else if (arg1 == "get-process") { // print information about a process with given id
        if (!log_path.isEmpty()) {
            QString str = TextFile::read(log_path + "/processes/" + CLP.named_parameters["id"].toString() + ".json");
            printf("%s", str.toLatin1().data());
        }
    }
    else if (arg1 == "cleanup-cache") { // remove old files in the temporary directory
        CacheManager::globalInstance()->cleanUp();
    }
    else if (arg1 == "temp") {
        QString tmp_path = CacheManager::globalInstance()->localTempPath();
        printf("%s\n", tmp_path.toUtf8().data());
        return 0;
    }
    */
    else {
        print_usage(); //print usage information
        return -1;
    }

    return 0;
}

void sig_handler(int signum)
{
    (void)signum;
    QProcessManager* manager = ObjectRegistry::getObject<QProcessManager>();
    if (manager) {
        manager->closeAll();
    }
    abort();
}

void print_usage() {
    printf("Usage:\n");
    printf("mproc exec [processor_name] --[param1]=[val1] --[param2]=[val2] ... [--_force_run] [--_request_num_threads=4]\n");
    printf("mproc run [processor_name] --[param1]=[val1] --[param2]=[val2] ... [--_force_run] [--_request_num_threads=4]\n");
    printf("mproc queue [processor_name] --[param1]=[val1] --[param2]=[val2] ... [--_force_run] [--_request_num_threads=4]\n");
    printf("mproc list-processors\n");
    printf("mproc spec [processor_name]\n");
}

bool initialize_processor_manager(ProcessorManager &PM) {
    // Load the processor paths
    QStringList processor_paths = MLUtil::configResolvedPathList("mountainprocess", "processor_paths");
    if (processor_paths.isEmpty()) {
        qCCritical(MP) << "No processor paths found.";
        return false;
    }

    PM.setProcessorPaths(processor_paths);
    PM.reloadProcessors();
    return true;
}

bool list_processors()
{
    ProcessorManager PM;
    if (!initialize_processor_manager(PM))
        return false;

    QStringList pnames = PM.processorNames();
    qSort(pnames);
    foreach (QString pname, pnames) {
        qDebug().noquote() << pname;
    }
    return true;
}

void silent_message_output(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(msg)
    return;
}

bool spec(QString arg2)
{
    qInstallMessageHandler(silent_message_output);
    ProcessorManager PM;
    if (!initialize_processor_manager(PM)) {
        return false;
    }
    if (!arg2.isEmpty()) {
        MLProcessor MLP = PM.processor(arg2);
        QString json = QJsonDocument(MLP.spec).toJson(QJsonDocument::Indented);
        printf("%s\n", json.toLatin1().data());
    }
    else {
        QJsonArray processors_array;
        QStringList pnames = PM.processorNames();
        foreach (QString pname, pnames) {
            MLProcessor MLP = PM.processor(pname);
            processors_array.push_back(MLP.spec);
        }
        QJsonObject obj;
        obj["processors"] = processors_array;
        QString json = QJsonDocument(obj).toJson(QJsonDocument::Indented);
        printf("%s\n", json.toLatin1().data());
    }
    return true;
}

int exec_run_or_queue(QString arg1, QString arg2, const QMap<QString, QVariant> &clp)
{
    QString processor_name=arg2;
    ProcessorManager PM;
    if (!initialize_processor_manager(PM)) {
        return -1;
    }
    if (!PM.checkParameters(processor_name,clp)) {
        return -1;
    }

    MLProcessor MLP=PM.processor(processor_name);
    if (MLP.name!=processor_name) {
        qCWarning(MP).noquote() << "Unable to find processor: "+processor_name;
        return -1;
    }

    bool force_run=clp.contains("_force_run");
    if (((arg1=="run")||(arg1=="queue"))&&(!force_run)) {
        if (process_already_completed(MLP,clp)) {
            qDebug().noquote() << "Process already completed: "+processor_name;
            return 0;
        }
    }

    if (arg1=="queue") {
        int ret=coordinate_with_daemon_to_run_process(MLP,clp);
        return ret;
    }

    if ((arg1=="exec")||(arg1=="run")) {
        int ret=launch_process_and_wait(MLP,clp);
        if (ret!=0) {
            qCWarning(MP).noquote() << QString("Process returned with non-zero exit code (%1)").arg(ret);
            return ret;
        }
    }

    if (arg1=="run") {
        record_completed_process(MLP,clp);
    }

    return 0;
}

QStringList get_local_search_paths_2()
{
    QStringList local_search_paths = MLUtil::configResolvedPathList("prv", "local_search_paths");
    QString temporary_path = MLUtil::tempPath();
    if (!temporary_path.isEmpty()) {
        local_search_paths << temporary_path;
    }
    return local_search_paths;
}

QString resolve_file_name_prv(QString fname) {
    if (fname.endsWith(".prv")) {
        QString txt = TextFile::read(fname);
        QJsonParseError err;
        QJsonObject obj = QJsonDocument::fromJson(txt.toUtf8(), &err).object();
        if (err.error != QJsonParseError::NoError) {
            qCWarning(MP) << "Error parsing .prv file: " + fname;
            return "";
        }
        QString path0 = MLUtil::locatePrv(obj, get_local_search_paths_2());
        if (path0.isEmpty()) {
            qCWarning(MP) << "Unable to locate prv file originally at: " + obj["original_path"].toString();
            return "";
        }
        return path0;
    }
    else
        return fname;
}

QVariantMap resolve_file_names_in_parameters(const MLProcessor &MLP, const QVariantMap& parameters_in)
{
    QVariantMap parameters = parameters_in;

    foreach (MLParameter P, MLP.inputs) {
        QStringList list = MLUtil::toStringList(parameters[P.name]);
        if (list.count() == 1) {
            parameters[P.name] = resolve_file_name_prv(list[0]);
        }
        else {
            QVariantList list2;
            foreach (QString str, list) {
                list2 << resolve_file_name_prv(str);
            }
            parameters[P.name] = list2;
        }
    }
    foreach (MLParameter P, MLP.outputs) {
        QStringList list = MLUtil::toStringList(parameters[P.name]);
        if (list.count() == 1) {
            parameters[P.name] = resolve_file_name_prv(list[0]);
        }
        else {
            QVariantList list2;
            foreach (QString str, list) {
                list2 << resolve_file_name_prv(str);
            }
            parameters[P.name] = list2;
        }
    }
    return parameters;
}

void set_defaults_for_optional_parameters(const MLProcessor &MLP, QVariantMap &params) {
    QStringList pkeys=MLP.parameters.keys();
    foreach (QString key,pkeys) {
        MLParameter pp=MLP.parameters[key];
        if (pp.optional) {
            if (!params.contains(key))
                params[key]=pp.default_value;
        }
    }
}

bool run_command_as_bash_script(QProcess *qprocess,const QString &exe_command) {
    QString bash_script_fname = CacheManager::globalInstance()->makeLocalFile();

    int this_pid=QCoreApplication::applicationPid();

    QString script;
    script += "#!/bin/bash\n\n";
    script += exe_command + " &\n";
    script += "cmdpid=$!\n"; //get the pid of the exe_command
    script += "trap \"kill $cmdpid; exit 255;\" SIGINT SIGTERM\n"; //capture the terminate signal and pass it on
    script += QString("while kill -0 %1 >/dev/null 2>&1; do\n").arg(this_pid); //while the (parent) pid still exists
    script += "    if kill -0 $cmdpid > /dev/null 2>&1; then\n";
    script += "        sleep 1;\n"; //we are still going
    script += "    else\n"; //else the exe process is done
    script += "        wait $cmdpid\n"; //get the return code for the process that has already completed
    script += "        exit $?\n";
    script += "    fi\n";
    script += "done ;\n";
    script += "kill $cmdpid\n"; //the parent pid is gone
    script += "exit 255\n"; //return error exit code

    TextFile::write(bash_script_fname, script);
    qprocess->start("/bin/bash", QStringList(bash_script_fname));
    if (!qprocess->waitForStarted(2000)) {
        qCWarning(MP).noquote() << "Error starting process script for: "+exe_command;
        QFile::remove(bash_script_fname);
        return false;
    }
    CacheManager::globalInstance()->setTemporaryFileExpirePid(bash_script_fname, qprocess->processId());
    return true;
}

void remove_output_files(const MLProcessor &MLP, const QMap<QString, QVariant> &clp) {
    QStringList okeys=MLP.outputs.keys();
    foreach (QString key,okeys) {
        QString fname=clp.value(key).toString();
        if ((!fname.isEmpty())&&(QFile::exists(fname)))
            QFile::remove(fname);
    }
}

int launch_process_and_wait(const MLProcessor &MLP, const QMap<QString, QVariant> &clp_in, QString daemon_fname)
{
    QVariantMap clp = resolve_file_names_in_parameters(MLP, clp_in);

    set_defaults_for_optional_parameters(MLP, clp);

    QString id = MLUtil::makeRandomId();
    QString tempdir = CacheManager::globalInstance()->makeLocalFile("tempdir_" + id, CacheManager::ShortTerm);
    MLUtil::mkdirIfNeeded(tempdir);
    if (!QFile::exists(tempdir)) {
        qCWarning(MP) << "Error creating temporary directory for process: " + tempdir;
        return -1;
    }

    QString exe_command = MLP.exe_command;
    exe_command.replace(QRegExp("\\$\\(basepath\\)"), MLP.basepath);
    exe_command.replace(QRegExp("\\$\\(tempdir\\)"), tempdir);
    {
        QString ppp;
        {
            QStringList keys = MLP.inputs.keys();
            foreach (QString key, keys) {
                QStringList list = MLUtil::toStringList(clp[key]);
                exe_command.replace(QRegExp(QString("\\$%1\\$").arg(key)), list.value(0)); //note that only the first file name is inserted here
                foreach (QString str, list) {
                    ppp += QString("--%1=%2 ").arg(key).arg(str);
                }
            }
        }
        {
            QStringList keys = MLP.outputs.keys();
            foreach (QString key, keys) {
                QStringList list = MLUtil::toStringList(clp[key]);
                exe_command.replace(QRegExp(QString("\\$%1\\$").arg(key)), list.value(0)); //note that only the first file name is inserted here
                foreach (QString str, list) {
                    ppp += QString("--%1=%2 ").arg(key).arg(str);
                }
            }
        }
        {
            QStringList keys = MLP.parameters.keys();
            foreach (QString key, keys) {
                exe_command.replace(QRegExp(QString("\\$%1\\$").arg(key)), clp[key].toString());
                ppp += QString("--%1=%2 ").arg(key).arg(clp[key].toString());
            }
        }

        int rnt=clp.value("_request_num_threads").toInt();
        if (rnt) {
            ppp += QString("--_request_num_threads=%1 ").arg(rnt);
        }
        ppp += QString("--_tempdir=%1 ").arg(tempdir);

        exe_command.replace(QRegExp("\\$\\(arguments\\)"), ppp);
    }

    remove_output_files(MLP,clp);

    QProcess qprocess;
    if (!run_command_as_bash_script(&qprocess,exe_command)) {
        return -1;
    }
    while (qprocess.waitForFinished(200)) {
        QString str=qprocess.readAll();
        if (!str.isEmpty()) {
            qDebug().noquote() << str;
        }
    }

    return qprocess.exitCode();
}

bool all_input_and_output_files_exist(MLProcessor P, const QVariantMap& parameters)
{
    QStringList input_file_pnames = P.inputs.keys();
    QStringList output_file_pnames = P.outputs.keys();

    foreach (QString pname, input_file_pnames) {
        QStringList fnames = MLUtil::toStringList(parameters.value(pname));
        foreach (QString fname0, fnames) {
            if (!fname0.isEmpty()) {
                if (!QFile::exists(fname0)) {
                    return false;
                }
            }
        }
    }

    foreach (QString pname, output_file_pnames) {
        QStringList fnames = MLUtil::toStringList(parameters.value(pname));
        foreach (QString fname0, fnames) {
            if (!fname0.isEmpty()) {
                if (!QFile::exists(fname0)) {
                    return false;
                }
            }
        }
    }

    return true;
}

QJsonObject create_file_object(const QString& fname)
{
    QJsonObject obj;
    if (fname.isEmpty())
        return obj;
    obj["path"] = fname;
    if (!QFile::exists(fname)) {
        obj["size"] = 0;
        return obj;
    }
    else {
        obj["size"] = QFileInfo(fname).size();
        obj["last_modified"] = QFileInfo(fname).lastModified().toString("yyyy-MM-dd-hh-mm-ss-zzz");
    }
    if (QFileInfo(fname).isDir()) {
        QStringList fnames = QDir(fname).entryList(QDir::Files, QDir::Name);
        QJsonArray files_array;
        foreach (QString fname2, fnames) {
            QJsonObject obj0;
            obj0["name"] = fname2;
            obj0["object"] = create_file_object(fname + "/" + fname2);
            files_array.push_back(obj0);
        }
        obj["files"] = files_array;

        QStringList dirnames = QDir(fname).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        QJsonArray dirs_array;
        foreach (QString dirname2, dirnames) {
            QJsonObject obj0;
            obj0["name"] = dirname2;
            obj0["object"] = create_file_object(fname + "/" + dirname2);
            dirs_array.push_back(obj0);
        }
        obj["directories"] = dirs_array;
    }
    return obj;
}


QJsonObject compute_unique_process_object(MLProcessor P, const QVariantMap& parameters)
{
    /*
     * Returns an object that depends uniquely on the following:
     *   1. Version of mountainprocess
     *   2. Processor name and version
     *   3. The paths, sizes, and modification times of the input files (together with their parameter names)
     *   4. Same for the output files
     *   5. The parameters converted to strings
     */

    QJsonObject obj;

    obj["mountainprocess_version"] = "0.1";
    obj["processor_name"] = P.name;
    obj["processor_version"] = P.version;
    {
        QJsonObject inputs;
        QStringList input_pnames = P.inputs.keys();
        qSort(input_pnames);
        foreach (QString input_pname, input_pnames) {
            QStringList fnames = MLUtil::toStringList(parameters[input_pname]);
            if (fnames.count() == 1) {
                inputs[input_pname] = create_file_object(fnames[0]);
            }
            else {
                QJsonArray array;
                foreach (QString fname0, fnames) {
                    array.append(create_file_object(fname0));
                }
                inputs[input_pname] = array;
            }
        }
        obj["inputs"] = inputs;
    }
    {
        QJsonObject outputs;
        QStringList output_pnames = P.outputs.keys();
        qSort(output_pnames);
        foreach (QString output_pname, output_pnames) {
            QStringList fnames = MLUtil::toStringList(parameters[output_pname]);
            if (fnames.count() == 1) {
                outputs[output_pname] = create_file_object(fnames[0]);
            }
            else {
                QJsonArray array;
                foreach (QString fname0, fnames) {
                    array.append(create_file_object(fname0));
                }
                outputs[output_pname] = array;
            }
        }
        obj["outputs"] = outputs;
    }
    {
        QJsonObject parameters0;
        QStringList pnames = P.parameters.keys();
        qSort(pnames);
        foreach (QString pname, pnames) {
            parameters0[pname] = parameters[pname].toString();
        }
        obj["parameters"] = parameters0;
    }
    return obj;
}

QString compute_unique_object_code(QJsonObject obj)
{
    /// Witold I need a string that depends on the json object. However I am worried about the order of the fields. Is there a way to make this canonical?
    /// Jeremy: You can sort all keys in the dictionary, convert that to string and
    ///         calculate hash of that. However this is going to be CPU consuming
    QByteArray json = QJsonDocument(obj).toJson();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(json);
    return QString(hash.result().toHex());
}


bool process_already_completed(const MLProcessor &MLP, const QMap<QString, QVariant> &clp)
{
    if (!all_input_and_output_files_exist(MLP, clp))
        return false;

    QJsonObject obj = compute_unique_process_object(MLP, clp);

    QString code = compute_unique_object_code(obj);

    QString path0 = MLUtil::tempPath() + "/completed_processes";
    MLUtil::mkdirIfNeeded(path0);

    QString path1 = path0+"/" + code + ".json";

    return QFile::exists(path1);
}

void record_completed_process(const MLProcessor &MLP, const QMap<QString, QVariant> &clp)
{
    if (!all_input_and_output_files_exist(MLP, clp)) {
        qCWarning(MP).noquote() << "Unexpected problem in record_completed_process: not all input and output files exist.";
        return;
    }

    QJsonObject obj = compute_unique_process_object(MLP, clp);

    QString code = compute_unique_object_code(obj);

    QString path0 = MLUtil::tempPath() + "/completed_processes";
    MLUtil::mkdirIfNeeded(path0);

    QString path1 = path0+"/" + code + ".json";

    if (!QFile::exists(path1)) {
        QString json=QJsonDocument(obj).toJson();
        if (!TextFile::write(path1,json)) {
            qCWarning(MP).noquote() << "Unexpected problem in record_completed_process: unable to write file: "+path1;
        }
    }
}

void sleep_msec(int msec) {
    int microseconds=msec*1000;
    usleep(microseconds);
}

void touch(const QString& filePath)
{
    QProcess::execute("touch",QStringList(filePath));
}

int coordinate_with_daemon_to_run_process(const MLProcessor &MLP, const QMap<QString, QVariant> &clp)
{
    QString path_queued = MLUtil::tempPath() + "/queued_processes";
    QString path_running = MLUtil::tempPath() + "/running_processes";
    QString path_finished = MLUtil::tempPath() + "/finished_processes";
    MLUtil::mkdirIfNeeded(path_queued);
    MLUtil::mkdirIfNeeded(path_running);
    MLUtil::mkdirIfNeeded(path_finished);

    QJsonObject obj;
    obj["processor_name"]=MLP.name;
    obj["clp"]=QJsonObject::fromVariantMap(clp);
    QString obj_json=QJsonDocument(obj).toJson();

    //QString code=compute_unique_object_code(obj);

    QString code=MLP.name+"_"+MLUtil::makeRandomId();
    QString fname_queued = path_queued+"/"+code+".json";
    if (!TextFile::write(fname_queued,obj_json)) {
        qCWarning(MP).noquote() << "Unable to write file: "+fname_queued;
        return -1;
    }

    QTime timer; timer.start();
    while (QFile::exists(fname_queued)) {
        sleep_msec(10);
        if (timer.elapsed()>2000) {
            touch(fname_queued);
            timer.restart();
        }
    }

    QString fname_running = path_running+"/"+code+".json";
    if (!QFile::exists(fname_running)) {
        sleep_msec(100); //give it a second try
        if (!QFile::exists(fname_running)) {
            qCWarning(MP).noquote() << "Queued file has disappeared and did not reappear in running directory: "+fname_queued;
            return -1;
        }
    }

    launch_process_and_wait(MLP,clp,fname_running);

    QString fname_finished = path_finished+"/"+code+".json";

    if (!QFile::exists(path3)) {
        sleep_msec(100); //give it a second try
        if (!QFile::exists(path3)) {
            qCWarning(MP).noquote() << "Running file has disappeared and did not reappear in finished directory: "+path2;
            return false;
        }
    }

    QString json=TextFile::read(path3);
    if (json.isEmpty()) {
        qCWarning(MP).noquote() << "Finished file is empty (unexpected): "+path3;
        return false;
    }
    QJsonParseError err;
    QJsonObject obj=QJsonDocument::fromJson(json,&err).object();
    if (err.error!=QJsonParseError::NoError) {
        qCWarning(MP).noquote() << "Error parsing json in output file: "+err.errorString();
        return false;
    }

    int exit_code=obj["exit_code"].toInt();

    return exit_code;
}

int run_process_and_report_to_daemon(const MLProcessor &MLP, const QMap<QString, QVariant> &clp)
{

}
