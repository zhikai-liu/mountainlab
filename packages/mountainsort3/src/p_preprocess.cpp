#include "p_preprocess.h"
#include "omp.h"
#include "mlcommon.h"

#include <QTime>
#include <diskreadmda32.h>
#include <diskwritemda.h>

namespace Preprocess {

struct TimeChunkInfo {
    bigint t_padding; //t1 corresponds to index t_padding (because we have padding/overlap)
    bigint t1; //starting timepoint (corresponding to index t_padding)
    bigint size; //number of timepoints (excluding padding on left and right)
};
QList<TimeChunkInfo> get_time_chunk_infos(bigint M, bigint N, int num_simultaneous, bigint min_num_chunks, bigint overlap_size);
void get_whitening_matrix_by_sampling(Mda &matrix,const DiskReadMda32 &X,P_preprocess_opts opts);
void preprocess_time_chunk(Mda32 &Y,const Mda32 &X,const Mda &whitening_matrix,P_preprocess_opts opts);

struct ProgressReporter {
    void startProcessingStep(QString name,double total_expected_bytes_to_process=0)
    {
        qDebug().noquote() << "";
        qDebug().noquote() << QString("Starting  [%1]...").arg(name);
        m_processing_step_name = name;
        m_processing_step_timer.start();
        m_progress_timer.start();
        m_bytes_processed = 0;
        m_bytes_read = 0;
        m_bytes_written = 0;
        m_read_time_sec = 0;
        m_total_expected_bytes_to_process=total_expected_bytes_to_process;
    }
    void addBytesProcessed(double bytes)
    {
        double elapsed_sec = m_progress_timer.elapsed() * 1.0 / 1000;
        double mb = bytes / 1e6;
        m_bytes_processed += bytes;
        QString status;
        if (m_total_expected_bytes_to_process) {
            status=QString("%1%").arg(m_bytes_processed*100.0/m_total_expected_bytes_to_process);
        }
        qDebug().noquote() << QString("Processed %2 MB in %3 sec (%4 MB/sec): Total %5 MB [%1] (%6)...").arg(m_processing_step_name).arg(mb).arg(elapsed_sec).arg(mb / elapsed_sec).arg(m_bytes_processed / 1e6).arg(status);
        m_progress_timer.restart();
    }
    void addBytesRead(double bytes)
    {
        double elapsed_sec = m_progress_timer.elapsed() * 1.0 / 1000;
        double mb = bytes / 1e6;
        m_read_time_sec += elapsed_sec;
        m_bytes_read += bytes;
        qDebug().noquote() << QString("Read %2 MB in %3 sec (%4 MB/sec): Total %5 MB [%1]...").arg(m_processing_step_name).arg(mb).arg(elapsed_sec).arg(mb / elapsed_sec).arg(m_bytes_read / 1e6);
        m_progress_timer.restart();
    }
    void addBytesWritten(double bytes)
    {
        double elapsed_sec = m_progress_timer.elapsed() * 1.0 / 1000;
        double mb = bytes / 1e6;
        m_write_time_sec += elapsed_sec;
        m_bytes_written += bytes;
        qDebug().noquote() << QString("Wrote %2 MB in %3 sec (%4 MB/sec): Total %5 MB [%1]...").arg(m_processing_step_name).arg(mb).arg(elapsed_sec).arg(mb / elapsed_sec).arg(m_bytes_written / 1e6);
        m_progress_timer.restart();
    }

    void endProcessingStep()
    {
        double elapsed_sec = m_processing_step_timer.elapsed() * 1.0 / 1000;
        double mb = m_bytes_processed / 1e6;
        qDebug().noquote() << QString("Elapsed time -- %2 sec (%3 MB/sec) [%1]...\n").arg(m_processing_step_name).arg(elapsed_sec).arg(mb / elapsed_sec);
        qDebug().noquote() << "";
        m_summary_text+=QString("%2 sec (includes %3 sec reading, %4 sec writing) [%1]\n").arg(m_processing_step_name).arg(elapsed_sec).arg(m_read_time_sec).arg(m_write_time_sec);
    }
    void printSummaryText() {
        qDebug().noquote() << "";
        qDebug().noquote() << m_summary_text;
        qDebug().noquote() << "";
    }

private:
    QString m_processing_step_name;
    QTime m_processing_step_timer;
    QTime m_progress_timer;
    double m_total_expected_bytes_to_process = 0;
    double m_bytes_processed = 0;
    double m_bytes_read = 0;
    double m_bytes_written = 0;
    double m_read_time_sec =0;
    double m_write_time_sec =0;
    QString m_summary_text;
};

}

using namespace Preprocess;

bool p_preprocess(QString timeseries,QString timeseries_out,QString temp_path,P_preprocess_opts opts) {

    DiskReadMda32 X(timeseries);
    int M=X.N1();
    bigint N=X.N2();

    int tot_threads=omp_get_max_threads();
    QList<TimeChunkInfo> time_chunk_infos;

    Mda whitening_matrix;

    if (opts.whiten)
        get_whitening_matrix_by_sampling(whitening_matrix,X,opts);

    ProgressReporter PR;

    DiskWriteMda Y(MDAIO_TYPE_FLOAT32,timeseries_out,M,N);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    PR.startProcessingStep("Preprocess",M*N*sizeof(float));
    time_chunk_infos = get_time_chunk_infos(M, N, tot_threads, tot_threads, 10000);
    for (bigint i = 0; i < time_chunk_infos.count(); i+=tot_threads) {
        QList<TimeChunkInfo> infos=time_chunk_infos.mid(i,tot_threads);
        QList<Mda32> time_chunks;
        QList<Mda32> time_chunks_out;
        double bytes0=0;
        for (int j=0; j<infos.count(); j++) {
            Mda32 time_chunk;
            X.readChunk(time_chunk, 0, infos[j].t1 - infos[j].t_padding, M, infos[j].size + 2 * infos[j].t_padding);
            time_chunks << time_chunk;
            time_chunks_out << Mda32();
            bytes0+=time_chunk.totalSize() * sizeof(float);
        }
        PR.addBytesRead(bytes0);

#pragma omp parallel for num_threads(tot_threads)
        for (int j=0; j<infos.count(); j++) {
            Mda32 tmp;
            preprocess_time_chunk(tmp,time_chunks[i],whitening_matrix,opts);
            tmp.getChunk(time_chunks_out[j],0,infos[j].t_padding,M,infos[j].size);
        }
        PR.addBytesProcessed(bytes0);

        for (int j=0; j<infos.count(); j++) {
            Y.writeChunk(time_chunks_out[j], 0, infos[j].t1);
        }
        PR.addBytesWritten(bytes0);
    }
    PR.endProcessingStep();

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

void get_whitening_matrix_by_sampling(Mda &matrix,const DiskReadMda32 &X,P_preprocess_opts opts) {
    //finish
}

void preprocess_time_chunk(Mda32 &Y,const Mda32 &X,const Mda &whitening_matrix,P_preprocess_opts opts) {
    //finish
    Y=X;
}

}
