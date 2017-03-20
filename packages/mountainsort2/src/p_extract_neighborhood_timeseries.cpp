/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/

#include "p_extract_neighborhood_timeseries.h"

#include <diskreadmda32.h>
#include "diskwritemda.h"

namespace P_extract_neighborhood_timeseries {
Mda32 extract_neighborhood(const Mda32& X, QList<int> channels);
}

bool p_extract_neighborhood_timeseries(QString timeseries, QString timeseries_out, QList<int> channels)
{
    DiskReadMda32 X(timeseries);
    bigint M = X.N1();
    bigint N = X.N2();
    bigint M2 = channels.count();

    DiskWriteMda Y;
    Y.open(X.mdaioHeader().data_type, timeseries_out, M2, N);

    // TODO: don't load the whole thing into memory
    Mda32 chunk;
    X.readChunk(chunk, 0, 0, M, N);
    Mda32 chunk2 = P_extract_neighborhood_timeseries::extract_neighborhood(chunk, channels);
    Y.writeChunk(chunk2, 0, 0);
    Y.close();

    return true;
}

namespace P_extract_neighborhood_timeseries {
Mda32 extract_neighborhood(const Mda32& X, QList<int> channels)
{
    bigint M2 = channels.count();
    bigint N = X.N2();
    Mda32 ret(M2, N);
    for (bigint i = 0; i < N; i++) {
        for (bigint m = 0; m < M2; m++) {
            bigint m2 = channels[m] - 1;
            ret.setValue(X.value(m2, i), m, i);
        }
    }
    return ret;
}
}
