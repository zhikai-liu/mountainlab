#include "p_preprocess.h"
#include "omp.h"
#include "mlcommon.h"

namespace Preprocess {

struct TimeChunkInfo {
    bigint t_padding; //t1 corresponds to index t_padding (because we have padding/overlap)
    bigint t1; //starting timepoint (corresponding to index t_padding)
    bigint size; //number of timepoints (excluding padding on left and right)
};
QList<TimeChunkInfo> get_time_chunk_infos(bigint M, bigint N, int num_threads, bigint min_num_chunks, bigint overlap_size);


}

using namespace Preprocess;

bool p_preprocess(QString timeseries,QString timeseries_out,QString temp_path,P_preprocess_opts opts) {

    int tot_threads=omp_get_max_threads();


    return false;
}

namespace Preprocess {
QList<TimeChunkInfo> get_time_chunk_infos(bigint M, bigint N, int num_simultaneous, bigint min_num_chunks, bigint overlap_size)
{
    bigint RAM_available_for_chunks_bytes = 10e9;
    bigint chunk_size = RAM_available_for_chunks_bytes / (M * num_simultaneous * 4);
    if (chunk_size > N)
        chunk_size = N;
    if (N<chunk_size*min_num_chunks) {
        chunk_size=N/min_num_chunks;
    }
    if (chunk_size < 20000)
        chunk_size = 20000;
    bigint chunk_overlap_size = overlap_size;

    // Prepare the information on the time chunks
    QList<TimeChunkInfo> time_chunk_infos;
    for (bigint t = 0; t < N; t += chunk_size) {
        TimeChunkInfo info;
        info.t1 = t;
        info.t_padding = chunk_overlap_size;
        info.size = chunk_size;
        if (t + info.size > N)
            info.size = N - t;
        time_chunk_infos << info;
    }

    return time_chunk_infos;
}

}
