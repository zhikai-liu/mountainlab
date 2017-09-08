#ifndef MPROCMAIN_H
#define MPROCMAIN_H

#include "processormanager.h"

#include <QString>
#include <QVariant>

void sig_handler(int signum);

bool list_processors();
bool spec(QString arg2);
int exec_run_or_queue(QString arg1, QString arg2, const QMap<QString, QVariant>& clp);

void print_usage();
int launch_process_and_wait(const MLProcessor& MLP, const QMap<QString, QVariant>& clp, QString monitor_file_name);
bool process_already_completed(const MLProcessor& MLP, const QMap<QString, QVariant>& clp);
void record_completed_process(const MLProcessor& MLP, const QMap<QString, QVariant>& clp);
QString wait_until_ready_to_run(const MLProcessor& MLP, const QMap<QString, QVariant>& clp, bool* already_completed); //returns monitor file name

#endif // MPROCMAIN_H
