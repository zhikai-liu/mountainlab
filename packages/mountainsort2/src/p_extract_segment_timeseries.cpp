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
QVector<long> get_durations(QStringList timeseries_list);
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

    //conserve memory as of 3/1/17 -- jfm
    long chunk_size = 10000; //conserve memory!
    for (long t = 0; t < N2; t += chunk_size) {
        long sz = chunk_size;
        if (t + sz > N2)
            sz = N2 - t;
        Mda32 chunk;
        X.readChunk(chunk, 0, t1 + t, M, sz);
        Y.writeChunk(chunk, 0, t);
        Y.close();
    }

    return true;
}

bool p_extract_segment_timeseries_from_concat_list(QStringList timeseries_list, QString timeseries_out, long t1, long t2)
{
    QString ts0 = timeseries_list.value(0);
    DiskReadMda32 X(ts0);
    int M = X.N1();
    QVector<long> durations = P_extract_segment_timeseries::get_durations(timeseries_list);
    QVector<long> start_timepoints, end_timepoints;
    start_timepoints << 0;
    end_timepoints << durations.value(0) - 1;
    for (int i = 1; i < durations.count(); i++) {
        start_timepoints << end_timepoints[i - 1] + 1;
        end_timepoints << start_timepoints[i] + durations[i] - 1;
    }

    long ii1 = 0;
    while ((ii1 < timeseries_list.count()) && (t1 > end_timepoints[ii1])) {
        ii1++;
    }
    long ii2 = ii1;
    while ((ii2 + 1 < timeseries_list.count()) && (start_timepoints[ii2 + 1] <= t2)) {
        ii2++;
    }

    Mda32 out(M, t2 - t1 + 1);
    for (int ii = ii1; ii <= ii2; ii++) {
        DiskReadMda32 Y(timeseries_list.value(ii));
        Mda32 chunk;

        //ttA,ttB, the range to read from Y
        //ssA, the position to write it in "out"
        long ttA, ttB, ssA;
        if (ii == ii1) {
            long offset = t1 - start_timepoints[ii];
            ssA = 0;
            if (ii1 < ii2) {
                ttA = offset;
                ttB = durations[ii] - 1;
            }
            else {
                ttA = offset;
                ttB = ttA + (t2 - t1 + 1) - 1;
            }
        }
        else if (ii == ii2) {
            ttA = 0;
            ttB = t2 - start_timepoints[ii];
            ssA = start_timepoints[ii] - t1;
        }
        else {
            ttA = 0;
            ttB = durations[ii] - 1;
            ssA = start_timepoints[ii] - t1;
        }

        Y.readChunk(chunk, 0, ttA, M, ttB - ttA + 1);
        out.setChunk(chunk, 0, ssA);
    }

    //do it this way so we can specify the datatype
    DiskWriteMda Y;
    Y.open(X.mdaioHeader().data_type, timeseries_out, M, t2 - t1 + 1);
    Y.writeChunk(out, 0, 0);
    Y.close();

    return true;
}

namespace P_extract_segment_timeseries {
QVector<long> get_durations(QStringList timeseries_list)
{
    QVector<long> ret;
    foreach (QString ts, timeseries_list) {
        DiskReadMda32 X(ts);
        ret << X.N2();
    }
    return ret;
}
}
