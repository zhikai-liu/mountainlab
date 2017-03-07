/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/22/2017
*******************************************************/

#include "p_extract_clips.h"
#include "diskreadmda32.h"
#include "diskreadmda.h"

bool p_extract_clips(QStringList timeseries_list, QString event_times, QString clips_out, const QVariantMap& params)
{
    DiskReadMda32 X(2, timeseries_list);
    DiskReadMda ET(event_times);

    int M = X.N1();
    //int N = X.N2();
    int T = params["clip_size"].toInt();
    int L = ET.totalSize();

    if (!T) {
        qWarning() << "Unexpected: Clip size is zero.";
        return false;
    }

    int Tmid = (int)((T + 1) / 2) - 1;
    printf("Extracting clips (%d,%d,%d)...\n", M, T, L);
    Mda32 clips(M, T, L);
    for (int i = 0; i < L; i++) {
        int t1 = ET.value(i) - Tmid;
        //int t2 = t1 + T - 1;
        Mda32 tmp;
        X.readChunk(tmp, 0, t1, M, T);
        clips.setChunk(tmp, 0, 0, i);
    }

    return clips.write32(clips_out);
}
