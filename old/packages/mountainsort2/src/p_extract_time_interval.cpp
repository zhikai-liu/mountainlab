#include "p_extract_time_interval.h"

#include <diskreadmda32.h>
#include <diskwritemda.h>
#include <mda.h>

bool p_extract_time_interval(QStringList timeseries_list, QString firings, QString timeseries_out, QString firings_out, bigint t1, bigint t2)
{
    Mda firings0(firings);

    QVector<bigint> event_inds_to_use;
    for (bigint i = 0; i < firings0.N2(); i++) {
        double t0 = firings0.value(1, i);
        if ((t1 <= t0) && (t0 <= t2)) {
            event_inds_to_use << i;
        }
    }
    qDebug().noquote() << "p_extract_time_interval" << t1 << t2 << event_inds_to_use.count() << firings;
    bigint L2 = event_inds_to_use.count();
    Mda firings2(firings0.N1(), L2);
    for (bigint i = 0; i < L2; i++) {
        bigint j = event_inds_to_use[i];
        for (bigint r = 0; r < firings0.N1(); r++) {
            firings2.setValue(firings0.value(r, j), r, i);
        }
        firings2.setValue(firings2.value(1, i) - t1, 1, i);
    }

    if (!firings2.write64(firings_out))
        return false;

    DiskReadMda32 X(2, timeseries_list);
    DiskWriteMda Y(X.mdaioHeader().data_type, timeseries_out, X.N1(), t2 - t1 + 1);
    Mda32 chunk;
    if (!X.readChunk(chunk, 0, t1, X.N1(), t2 - t1 + 1)) {
        qWarning() << "Problem reading chunk.";
        return false;
    }
    if (!Y.writeChunk(chunk, 0, 0)) {
        qWarning() << "Problem writing chunk.";
        return false;
    }
    return true;
}
