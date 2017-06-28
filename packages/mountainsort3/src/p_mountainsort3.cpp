#include "fitstagecomputer.h"
#include "p_mountainsort3.h"

#include <QTime>
#include <diskreadmda32.h>
#include <mda.h>
#include <mda32.h>
#include "omp.h"
#include "neighborhoodsorter.h"
#include "get_sort_indices.h"
#include "merge_across_channels.h"
#include "globaltemplatecomputer.h"

namespace MountainSort3 {

struct TimeChunkInfo {
    bigint t_padding; //t1 corresponds to index t_padding (because we have padding/overlap)
    bigint t1; //starting timepoint (corresponding to index t_padding)
    bigint size; //number of timepoints (excluding padding on left and right)
};

QList<int> get_channels_from_geom(const Mda& geom, bigint m, double adjacency_radius);
void extract_channels(Mda32& ret, const Mda32& X, const QList<int>& channels);
QList<TimeChunkInfo> get_time_chunk_infos(bigint M, bigint N, int num_threads);

QList<QList<int> > get_neighborhood_batches(int M, int batch_size)
{
    QList<QList<int> > batches;
    for (int m = 1; m <= M; m += batch_size) {
        QList<int> batch;
        for (int m2 = m; (m2 < m + batch_size) && (m2 <= M); m2++)
            batch << m2;
        batches << batch;
    }
    return batches;
}

QVector<double> reorder(const QVector<double>& X, const QList<bigint>& inds);
QVector<int> reorder(const QVector<int>& X, const QList<bigint>& inds);
QVector<double> get_subarray(const QVector<double>& X, const QVector<bigint>& inds);
QVector<int> get_subarray(const QVector<int>& X, const QVector<bigint>& inds);

struct ProgressReporter {
    void startProcessingStep(QString name)
    {
        qDebug().noquote() << "";
        qDebug().noquote() << QString("[%1]: Starting...").arg(name);
        m_processing_step_name = name;
        m_processing_step_timer.start();
        m_progress_timer.start();
        m_bytes_processed = 0;
        m_bytes_read = 0;
    }
    void addBytesProcessed(double bytes)
    {
        double elapsed_sec = m_progress_timer.elapsed() * 1.0 / 1000;
        double mb = bytes / 1e6;
        m_bytes_processed += bytes;
        qDebug().noquote() << QString("[%1]: Processed %2 MB (%3 MB/sec): Total %4 MB").arg(m_processing_step_name).arg(mb).arg(mb / elapsed_sec).arg(m_bytes_processed / 1e6);
        m_progress_timer.restart();
    }
    void addBytesRead(double bytes)
    {
        double elapsed_sec = m_progress_timer.elapsed() * 1.0 / 1000;
        double mb = bytes / 1e6;
        m_bytes_read += bytes;
        qDebug().noquote() << QString("[%1]: Read %2 MB (%3 MB/sec): Total %4 MB").arg(m_processing_step_name).arg(mb).arg(mb / elapsed_sec).arg(m_bytes_read / 1e6);
        m_progress_timer.restart();
    }

    void endProcessingStep()
    {
        double elapsed_sec = m_processing_step_timer.elapsed() * 1.0 / 1000;
        double mb = m_bytes_processed / 1e6;
        qDebug().noquote() << QString("[%1]: Elapsed time -- %2 sec (%3 MB/sec)\n").arg(m_processing_step_name).arg(elapsed_sec).arg(mb / elapsed_sec);
        qDebug().noquote() << "";
    }

private:
    QString m_processing_step_name;
    QTime m_processing_step_timer;
    QTime m_progress_timer;
    double m_bytes_processed = 0;
    double m_bytes_read = 0;
};
}

using namespace MountainSort3;

bool p_mountainsort3(QString timeseries, QString geom, QString firings_out, QString temp_path, const P_mountainsort3_opts& opts)
{
    int tot_threads = omp_get_max_threads();
    omp_set_nested(1);

    ProgressReporter PR;

    DiskReadMda32 X(timeseries);
    bigint M = X.N1();
    bigint N = X.N2();
    qDebug().noquote() << QString("Starting sort: %1 channels, %2 timepoints").arg(M).arg(N);

    Mda Geom(geom);

    int num_simultaneous_neighborhoods = qMin(tot_threads, (int)M);
    int num_threads_within_neighboords = qMax(1, tot_threads / num_simultaneous_neighborhoods);
    QList<QList<int> > neighborhood_batches = get_neighborhood_batches(M, num_simultaneous_neighborhoods);
    QList<TimeChunkInfo> time_chunk_infos = get_time_chunk_infos(M, N, num_simultaneous_neighborhoods);

    QMap<int, NeighborhoodSorter*> neighborhood_sorters;
    QMap<int, QList<int> > neighborhood_channels;
    for (bigint m = 1; m <= M; m++) {
        neighborhood_sorters[m] = new NeighborhoodSorter;
        neighborhood_sorters[m]->setOptions(opts);
        neighborhood_sorters[m]->setNumThreads(num_threads_within_neighboords);
        neighborhood_channels[m] = get_channels_from_geom(geom, m, opts.adjacency_radius);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Read data and add to neighborhood sorters
    PR.startProcessingStep("Read data and add to neighborhood sorters");
    for (bigint i = 0; i < time_chunk_infos.count(); i++) {
        TimeChunkInfo TCI = time_chunk_infos[i];
        Mda32 time_chunk;
        X.readChunk(time_chunk, 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
        PR.addBytesRead(time_chunk.totalSize() * sizeof(float));
        for (int j = 0; j < neighborhood_batches.count(); j++) {
            QList<int> neighborhoods = neighborhood_batches[j];
            double bytes0 = 0;
#pragma omp parallel for num_threads(num_simultaneous_neighborhoods)
            for (int k = 0; k < neighborhoods.count(); k++) {
                int m;
#pragma omp critical(m1)
                {
                    m = neighborhoods[k]; //I don't know why this needs to be in a critical section
                }
                Mda32 neighborhood_time_chunk;
                extract_channels(neighborhood_time_chunk, time_chunk, neighborhood_channels[m]);
                neighborhood_sorters[m]->addTimeChunk(neighborhood_time_chunk, TCI.t_padding, TCI.t_padding);
#pragma omp critical(a1)
                {
                    bytes0 += neighborhood_time_chunk.N2() * sizeof(float);
                }
            }
            PR.addBytesProcessed(bytes0);
        }
    }
    PR.endProcessingStep();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sort in each neighborhood sorter
    PR.startProcessingStep("Sort in each neighborhood");
    for (int j = 0; j < neighborhood_batches.count(); j++) {
        QList<int> neighborhoods = neighborhood_batches[j];
        double bytes0 = 0;
#pragma omp parallel for num_threads(num_simultaneous_neighborhoods)
        for (int k = 0; k < neighborhoods.count(); k++) {
            int m;
#pragma omp critical(m2)
            {
                m = neighborhoods[k]; //I don't know why this needs to be in a critical section
            }
            neighborhood_sorters[m]->sort();
#pragma omp critical(a1)
            {
                bytes0 += neighborhood_sorters[m]->numTimepoints() * sizeof(float);
            }
        }
        PR.addBytesProcessed(bytes0);
    }
    PR.endProcessingStep();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Collect the events and sort them by time
    PR.startProcessingStep("Collect events and sort them by time");
    QVector<double> times;
    QVector<int> labels;
    QVector<int> central_channels;
    int K_offset = 0;
    for (bigint m = 1; m <= M; m++) {
        QVector<double> times0 = neighborhood_sorters[m]->times();
        QVector<int> labels0 = neighborhood_sorters[m]->labels();
        for (bigint a = 0; a < labels0.count(); a++)
            labels0[a] += K_offset;
        times.append(times0);
        labels.append(labels0);
        K_offset = qMax(K_offset, MLCompute::max(labels0));
        central_channels.append(QVector<int>(neighborhood_sorters[m]->labels().count(), m));
    }
    QList<bigint> sort_inds = get_sort_indices_bigint(times);
    times = reorder(times, sort_inds);
    labels = reorder(labels, sort_inds);
    central_channels = reorder(central_channels, sort_inds);
    PR.endProcessingStep();

    qDeleteAll(neighborhood_sorters);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Compute global templates
    PR.startProcessingStep("Compute global templates");
    Mda32 templates;
    {
        GlobalTemplateComputer GTC;
        GTC.setNumThreads(tot_threads);
        GTC.setClipSize(opts.clip_size);
        GTC.setTimesLabels(times, labels);
        for (bigint i = 0; i < time_chunk_infos.count(); i++) {
            TimeChunkInfo TCI = time_chunk_infos[i];
            Mda32 time_chunk;
            X.readChunk(time_chunk, 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
            PR.addBytesRead(time_chunk.totalSize() * sizeof(float));
            GTC.addTimeChunk(time_chunk, TCI.t_padding, TCI.t_padding);
            PR.addBytesProcessed(time_chunk.totalSize() * sizeof(float));
        }
        templates = GTC.templates();
    }
    PR.endProcessingStep();


    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Merge across channels
    PR.startProcessingStep("Merge across channels");
    {
        Merge_across_channels_opts oo;
        oo.clip_size = opts.clip_size;
        merge_across_channels(times, labels, central_channels, templates, oo);
    }
    PR.endProcessingStep();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Compute global templates
    PR.startProcessingStep("Compute global templates");
    {
        GlobalTemplateComputer GTC;
        GTC.setNumThreads(tot_threads);
        GTC.setClipSize(opts.clip_size);
        GTC.setTimesLabels(times, labels);
        for (bigint i = 0; i < time_chunk_infos.count(); i++) {
            TimeChunkInfo TCI = time_chunk_infos[i];
            Mda32 time_chunk;
            X.readChunk(time_chunk, 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
            PR.addBytesRead(time_chunk.totalSize() * sizeof(float));
            GTC.addTimeChunk(time_chunk, TCI.t_padding, TCI.t_padding);
            PR.addBytesProcessed(time_chunk.totalSize() * sizeof(float));
        }
        templates = GTC.templates();
    }
    PR.endProcessingStep();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Fit stage
    PR.startProcessingStep("Fit stage");
    FitStageComputer FSC;
    FSC.setTemplates(templates);
    FSC.setTimesLabels(times, labels);
    for (bigint i = 0; i < time_chunk_infos.count(); i++) {
        TimeChunkInfo TCI = time_chunk_infos[i];
        Mda32 time_chunk;
        X.readChunk(time_chunk, 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
        PR.addBytesRead(time_chunk.totalSize() * sizeof(float));
        FSC.processTimeChunk(TCI.t1,time_chunk, TCI.t_padding, TCI.t_padding);
        PR.addBytesProcessed(time_chunk.totalSize() * sizeof(float));
    }
    FSC.finalize();
    QVector<bigint> event_inds_to_use=FSC.eventIndicesToUse();
    qDebug().noquote() << QString("Using %1 of %2 events (%3%) after fit stage").arg(event_inds_to_use.count()).arg(times.count()).arg(event_inds_to_use.count()*1.0/times.count()*100);
    times=get_subarray(times,event_inds_to_use);
    labels=get_subarray(labels,event_inds_to_use);
    central_channels=get_subarray(central_channels,event_inds_to_use);
    PR.endProcessingStep();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create and write firings output
    PR.startProcessingStep("Create and write firings output");
    {
        bigint L = times.count();
        Mda firings(3, L);
        for (bigint i = 0; i < L; i++) {
            firings.set(central_channels[i], 0, i);
            firings.set(times[i], 1, i);
            firings.set(labels[i], 2, i);
        }

        firings.write64(firings_out);
    }
    PR.endProcessingStep();

    return true;
}

namespace MountainSort3 {
QList<int> get_channels_from_geom(const Mda& geom, bigint m, double adjacency_radius)
{
    bigint M = geom.N2();
    QList<int> ret;
    ret << m;
    for (int m2 = 1; m2 <= M; m2++) {
        if (m2 != m) {
            double sumsqr = 0;
            for (int i = 0; i < geom.N1(); i++) {
                double tmp = geom.value(i, m - 1) - geom.value(i, m2 - 1);
                sumsqr += tmp * tmp;
            }
            double dist = sqrt(sumsqr);
            if (dist <= adjacency_radius)
                ret << m2;
        }
    }
    return ret;
}

void extract_channels(Mda32& ret, const Mda32& X, const QList<int>& channels)
{
    bigint M2 = channels.count();
    bigint N = X.N2();
    ret.allocate(M2, N);
    for (bigint t = 0; t < N; t++) {
        for (bigint ii = 0; ii < M2; ii++) {
            ret.set(X.value(channels[ii] - 1, t), ii, t);
        }
    }
}

QList<TimeChunkInfo> get_time_chunk_infos(bigint M, bigint N, int num_simultaneous)
{
    bigint RAM_available_for_chunks_bytes = 10e9;
    bigint chunk_size = RAM_available_for_chunks_bytes / (M * num_simultaneous * 4);
    if (chunk_size < 1000)
        chunk_size = 1000;
    if (chunk_size > N)
        chunk_size = N;
    bigint chunk_overlap_size = 1000;

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

QVector<double> reorder(const QVector<double>& X, const QList<bigint>& inds)
{
    QVector<double> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

QVector<int> reorder(const QVector<int>& X, const QList<bigint>& inds)
{
    QVector<int> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

QVector<double> get_subarray(const QVector<double>& X, const QVector<bigint>& inds)
{
    QVector<double> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

QVector<int> get_subarray(const QVector<int>& X, const QVector<bigint>& inds)
{
    QVector<int> ret(inds.count());
    for (bigint i = 0; i < inds.count(); i++) {
        ret[i] = X[inds[i]];
    }
    return ret;
}

}
