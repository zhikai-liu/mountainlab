#include "p_generate_background_dataset.h"

#include <diskreadmda32.h>
#include <diskwritemda.h>
#include <mda.h>

namespace P_generate_background_dataset {
struct Segment {
    bigint t1 = 0, t2 = 0;
};
Mda32 blend_chunks(const Mda32& chunk_prev, const Mda32& chunk, int blend_overlap_size);
}

bool p_generate_background_dataset(QString timeseries, QString event_times, QString timeseries_out, P_generate_background_dataset_opts opts)
{
    DiskReadMda32 X(timeseries);
    bigint M = X.N1();
    bigint N = X.N2();

    Mda ET0(event_times);
    bigint L = ET0.totalSize();

    //put extra events at beginning and end
    QVector<bigint> ET(L + 2);
    ET[0] = 0;
    ET[L + 1] = N - 1;
    for (bigint i = 0; i < L; i++) {
        ET[i + 1] = ET0.get(i);
    }
    qSort(ET);
    L += 2; //note

    QList<P_generate_background_dataset::Segment> segments;

    for (bigint i = 0; i < L - 1; i++) {
        bigint time_diff = ET[i + 1] - ET[i];
        if (time_diff > opts.min_segment_size + 2 * opts.segment_buffer) {
            for (bigint jj = opts.segment_buffer; jj < time_diff - opts.segment_buffer; jj += opts.max_segment_size) {
                bigint k1 = jj;
                bigint k2 = jj + opts.max_segment_size;
                if (k2 > time_diff - opts.segment_buffer)
                    k2 = time_diff - opts.segment_buffer;
                if (k2 - k1 >= opts.min_segment_size) {
                    P_generate_background_dataset::Segment SS;
                    SS.t1 = ET[i] + k1;
                    SS.t2 = ET[i] + k2;
                    segments << SS;
                }
            }
        }
    }
    bigint N2 = 0;
    for (bigint i = 0; i < segments.count(); i++) {
        if (i == 0)
            N2 += segments[i].t2 - segments[i].t1 + 1;
        else
            N2 += segments[i].t2 - segments[i].t1 + 1 - opts.blend_overlap_size;
    }

    DiskWriteMda Y(MDAIO_TYPE_FLOAT32, timeseries_out, M, N2);
    bigint offset = 0;
    Mda32 chunk_prev;
    for (bigint i = 0; i < segments.count(); i++) {
        P_generate_background_dataset::Segment S = segments[i];
        Mda32 chunk;
        if (!X.readChunk(chunk, 0, S.t1, M, S.t2 - S.t1 + 1)) {
            qWarning() << "Problem reading chunk";
            return false;
        }
        if (i == 0) {
            if (!Y.writeChunk(chunk, 0, offset)) {
                qWarning() << "Problem writing chunk";
                return false;
            }
            offset += S.t2 - S.t1 - 1;
        }
        else {
            Mda32 chunk2 = P_generate_background_dataset::blend_chunks(chunk_prev, chunk, opts.blend_overlap_size);
            if (!Y.writeChunk(chunk2, 0, offset - opts.blend_overlap_size)) {
                qWarning() << "Problem writing chunk";
                return false;
            }
            offset += S.t2 - S.t1 - 1 - opts.blend_overlap_size;
        }
        chunk_prev = chunk;
    }

    return true;
}

namespace P_generate_background_dataset {
Mda32 blend_chunks(const Mda32& chunk_prev, const Mda32& chunk, int blend_overlap_size)
{
    bigint M = chunk.N1();
    Mda32 ret = chunk;
    for (bigint i = 0; i < blend_overlap_size; i++) {

        double th = sin((M_PI / 2) * i / blend_overlap_size); //sin^2
        th = th * th;

        //factor1^2+factor2^2=1
        double factor1 = sin((M_PI / 2) * th);
        double factor2 = cos((M_PI / 2) * th);

        for (bigint m = 0; m < M; m++) {
            ret.setValue(factor1 * chunk.value(m, i) + factor2 * chunk_prev.value(m, chunk_prev.N2() - blend_overlap_size + i), m, i);
        }
    }
    return ret;
}
}
