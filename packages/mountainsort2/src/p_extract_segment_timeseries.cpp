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
QVector<bigint> get_durations(QStringList timeseries_list);
Mda32 extract_channels_from_chunk(const Mda32 &chunk,const QList<int> &channels);
}

bool p_extract_segment_timeseries(QString timeseries, QString timeseries_out, bigint t1, bigint t2,const QList<int> &channels)
{
    DiskReadMda32 X(timeseries);
    int M = X.N1();
    //bigint N=X.N2();
    bigint N2 = t2 - t1 + 1;

    if (!channels.isEmpty()) {
        M=channels.count();
    }

    //do it this way so we can specify the datatype
    DiskWriteMda Y;
    Y.open(X.mdaioHeader().data_type, timeseries_out, M, N2);

    //conserve memory as of 3/1/17 -- jfm
    bigint chunk_size = 10000; //conserve memory!
    for (bigint t = 0; t < N2; t += chunk_size) {
        bigint sz = chunk_size;
        if (t + sz > N2)
            sz = N2 - t;
        Mda32 chunk;
        X.readChunk(chunk, 0, t1 + t, M, sz);
        if (!channels.isEmpty()) {
            chunk=P_extract_segment_timeseries::extract_channels_from_chunk(chunk,channels);
        }
        Y.writeChunk(chunk, 0, t);
        Y.close();
    }

    return true;
}

bool p_extract_segment_timeseries_from_concat_list(QStringList timeseries_list, QString timeseries_out, bigint t1, bigint t2,const QList<int> &channels)
{
    QString ts0 = timeseries_list.value(0);
    DiskReadMda32 X(ts0);
    int M = X.N1();
    if (!channels.isEmpty()) {
        M=channels.count();
    }
    QVector<bigint> durations = P_extract_segment_timeseries::get_durations(timeseries_list);
    QVector<bigint> start_timepoints, end_timepoints;
    start_timepoints << 0;
    end_timepoints << durations.value(0) - 1;
    for (int i = 1; i < durations.count(); i++) {
        start_timepoints << end_timepoints[i - 1] + 1;
        end_timepoints << start_timepoints[i] + durations[i] - 1;
    }

    int ii1 = 0;
    while ((ii1 < timeseries_list.count()) && (t1 > end_timepoints[ii1])) {
        ii1++;
    }
    int ii2 = ii1;
    while ((ii2 + 1 < timeseries_list.count()) && (start_timepoints[ii2 + 1] <= t2)) {
        ii2++;
    }

    Mda32 out(M, t2 - t1 + 1);
    for (int ii = ii1; ii <= ii2; ii++) {
        DiskReadMda32 Y(timeseries_list.value(ii));
        Mda32 chunk;

        //ttA,ttB, the range to read from Y
        //ssA, the position to write it in "out"
        bigint ttA, ttB, ssA;
        if (ii == ii1) {
            bigint offset = t1 - start_timepoints[ii];
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

        Y.readChunk(chunk, 0, ttA, Y.N1(), ttB - ttA + 1);
        if (!channels.isEmpty()) {
            chunk=P_extract_segment_timeseries::extract_channels_from_chunk(chunk,channels);
        }
        out.setChunk(chunk, 0, ssA);
    }

    //do it this way so we can specify the datatype
    DiskWriteMda Y;
    if (!Y.open(X.mdaioHeader().data_type, timeseries_out, M, t2 - t1 + 1)) {
        qWarning() << "Error opening file for writing: " << timeseries_out << M << t2 - t1 + 1;
        return false;
    }
    Y.writeChunk(out, 0, 0);
    Y.close();

    return true;
}

namespace P_extract_segment_timeseries {
QVector<bigint> get_durations(QStringList timeseries_list)
{
    QVector<bigint> ret;
    foreach (QString ts, timeseries_list) {
        DiskReadMda32 X(ts);
        ret << X.N2();
    }
    return ret;
}
Mda32 extract_channels_from_chunk(const Mda32 &chunk,const QList<int> &channels) {
    Mda32 ret(channels.count(),chunk.N2());
    for (bigint i=0; i<chunk.N2(); i++) {
        for (int j=0; j<channels.count(); j++) {
            ret.setValue(chunk.value(channels[j]-1,i),j,i);
        }
    }
    return ret;
}
}

bool p_extract_segment_firings(QString firings_path, QString firings_out_path, bigint t1, bigint t2)
{
    Mda firings(firings_path);
    QVector<int> inds;
    for (int i = 0; i < firings.N2(); i++) {
        double t0 = firings.value(1, i);
        if ((t1 <= t0) && (t0 <= t2))
            inds << i;
    }
    Mda ret(firings.N1(), inds.count());
    for (int i = 0; i < inds.count(); i++) {
        int j = inds[i];
        for (int r = 0; r < firings.N1(); r++) {
            ret.setValue(firings.value(r, j), r, i);
        }
        ret.setValue(ret.value(1, i) - t1, 1, i);
        ;
    }
    return ret.write64(firings_out_path);
}
