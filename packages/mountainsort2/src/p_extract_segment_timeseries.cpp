/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/23/2017
*******************************************************/

#include "p_extract_segment_timeseries.h"

#include <diskreadmda32.h>
#include <diskwritemda.h>
#include <mda32.h>

namespace P_extract_segment_timeseries {
}

bool p_extract_segment_timeseries(QString timeseries, QString timeseries_out, long t1, long t2)
{
    DiskReadMda32 X(timeseries);
    int M = X.N1();
    //long N=X.N2();
    int N2 = t2 - t1 + 1;

    //do it this way so we can specify the datatype
    DiskWriteMda Y;
    Y.open(X.mdaioHeader().data_type, timeseries_out, M, N2);

    Mda32 chunk;
    X.readChunk(chunk, 0, t1, M, N2);
    Y.writeChunk(chunk, 0, 0);
    Y.close();

    return true;
}

namespace P_extract_neighborhood_timeseries {
}
