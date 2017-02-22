#include "p_consolidate_clusters.h"
#include <QFile>

namespace P_consolidate_clusters {

}

bool p_consolidate_clusters(QString clips, QString labels, QString labels_out, Consolidate_clusters_opts opts)
{
    QFile::copy(labels,labels_out);
    return true;
}

namespace P_consolidate_clusters {

}
