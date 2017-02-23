#include "combine_firings_files_aa.h"

namespace combine_firings_files_aa {

long get_max_event_index(QStringList firings_path_list);

bool run(const QStringList& firings_path_list, const QString& firings_out_path)
{
    if (firings_path_list.count() == 0) {
        qWarning() << "Firings path list is empty.";
        return false;
    }
    long max_event_index = get_max_event_index(firings_path_list);

    QString path0 = firings_path_list[0];
    Mda F0(path0);
    int R = F0.N1();

    Mda firings(R, max_event_index + 1);
    foreach (QString path, firings_path_list) {
        Mda FF(path);
        for (long i = 0; i < FF.N2(); i++) {
            long j = FF.value(3, i);
            for (int r = 0; r < R; r++) {
                firings.setValue(FF.value(r, j), r, j);
            }
        }
    }

    return firings.write64(firings_out_path);
}

long get_max_event_index(QStringList firings_path_list)
{
    long ret = -1;
    foreach (QString path, firings_path_list) {
        Mda F(path);
        for (long i = 0; i < F.N2(); i++) {
            long ind = F.value(3, i);
            if (ind > ret)
                ret = ind;
        }
    }
    return ret;
}
}
