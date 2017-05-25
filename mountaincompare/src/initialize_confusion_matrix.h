/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/29/2017
*******************************************************/
#ifndef INITIALIZE_CONFUSION_MATRIX_H
#define INITIALIZE_CONFUSION_MATRIX_H

#include <QSet>
#include <QThread>
#include <mountainprocessrunner.h>
#include <QDebug>

class Initialize_confusion_matrix : public QThread {
    Q_OBJECT
public:
    Initialize_confusion_matrix()
    {
        QObject::connect(this, SIGNAL(finished()), this, SIGNAL(finishedInGui())); //so we can use lambda expressions
    }

    // input
    bool relabel = false;
    QString firings1;
    QString firings2;
    QSet<int> clusters1_to_use;
    QSet<int> clusters2_to_use;

    // output
    QString confusion_matrix;
    QString matched_firings;
    QString label_map;
    QString firings1_out;
    QString firings2_out;
    QString firings2_relabeled;

    void run()
    {
        if (clusters1_to_use.isEmpty()) {
            firings1_out = firings1;
        }
        else {
            QStringList clusters_str;
            foreach (int k, clusters1_to_use)
                clusters_str << QString("%1").arg(k);
            MountainProcessRunner MPR;
            MPR.setProcessorName("mountainsort.extract_firings");
            QVariantMap params;
            params["firings"] = firings1;
            params["clusters"] = clusters_str.join(",");
            MPR.setInputParameters(params);
            firings1_out = MPR.makeOutputFilePath("firings_out");
            MPR.runProcess();
        }
        if (clusters2_to_use.isEmpty()) {
            firings2_out = firings2;
        }
        else {
            QStringList clusters_str;
            foreach (int k, clusters2_to_use)
                clusters_str << QString("%1").arg(k);
            MountainProcessRunner MPR;
            MPR.setProcessorName("mountainsort.extract_firings");
            QVariantMap params;
            params["firings"] = firings2;
            params["clusters"] = clusters_str.join(",");
            MPR.setInputParameters(params);
            firings2_out = MPR.makeOutputFilePath("firings_out");
            MPR.runProcess();
        }

        {
            MountainProcessRunner MPR;
            MPR.setProcessorName("mountainsort.confusion_matrix");
            QVariantMap params;
            params["firings1"] = firings1_out;
            params["firings2"] = firings2_out;
            params["match_matching_offset"] = 30;
            if (relabel)
                params["relabel_firings2"] = "true";
            //params["_force_run"] = "true";
            MPR.setInputParameters(params);
            confusion_matrix = MPR.makeOutputFilePath("confusion_matrix_out");
            matched_firings = MPR.makeOutputFilePath("matched_firings_out");
            label_map = MPR.makeOutputFilePath("label_map_out");
            if (relabel)
                firings2_relabeled = MPR.makeOutputFilePath("firings2_relabeled_out");
            else
                firings2_relabeled = firings2;
            MPR.runProcess();
        }
    }
signals:
    void finishedInGui(); //so we can use lambda expressions
};

#endif // INITIALIZE_CONFUSION_MATRIX_H
