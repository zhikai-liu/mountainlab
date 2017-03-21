/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/

#include "p_extract_clips.h"
#include "diskreadmda32.h"
#include "diskreadmda.h"

#include <diskwritemda.h>

bool p_extract_clips(QStringList timeseries_list, QString event_times, QString clips_out, const QVariantMap& params)
{
    DiskReadMda32 X(2, timeseries_list);
    DiskReadMda ET(event_times);

    bigint M = X.N1();
    //bigint N = X.N2();
    bigint T = params["clip_size"].toInt();
    bigint L = ET.totalSize();

    if (!T) {
        qWarning() << "Unexpected: Clip size is zero.";
        return false;
    }

    bigint Tmid = (bigint)((T + 1) / 2) - 1;
    printf("Extracting clips (%ld,%ld,%ld)...\n", M, T, L);
    DiskWriteMda clips;
    clips.open(MDAIO_TYPE_FLOAT32, clips_out, M, T, L);
    for (bigint i = 0; i < L; i++) {
        bigint t1 = ET.value(i) - Tmid;
        //bigint t2 = t1 + T - 1;
        Mda32 tmp;
        X.readChunk(tmp, 0, t1, M, T);
        if (!clips.writeChunk(tmp,0,0,i)) {
            qWarning() << "Problem writing chunk" << i;
            return false;
        }
    }

    return true;
}
