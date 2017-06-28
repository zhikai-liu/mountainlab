#include "neighborhoodsorter.h"
#include "p_multineighborhood_sort.h"
#include "get_sort_indices.h"

#include <QTime>
#include <diskreadmda32.h>
#include <diskwritemda.h>
#include <mda.h>
#include <mda32.h>
#include "omp.h"
#include "compute_templates_0.h"
#include "merge_across_channels.h"
#include "fit_stage.h"
#include "reorder_labels.h"
#include "detect_events.h"
#include "dimension_reduce_clips.h"
#include "sort_clips.h"
#include "consolidate_clusters.h"
#include "cachemanager.h"
#include <QCoreApplication>

QList<int> get_channels_from_geom(const Mda& geom, bigint m, double adjacency_radius);
void extract_channels(Mda32& ret, const Mda32& X, const QList<int>& channels);
void sort_events_by_time(QVector<double>& times, QVector<int>& labels, QVector<int>& central_channels);
QVector<bigint> get_inds_for_times_in_range(const QVector<double>& times, double t1, double t2);
void extract_clips(Mda32& clips_out, const Mda32& timeseries, const QVector<double>& times, int clip_size);
Mda32 compute_templates_from_clips(const Mda32& clips, const QVector<int>& labels);
QVector<double> get_subarray(const QVector<double>& X, const QVector<bigint>& inds);
QVector<int> get_subarray(const QVector<int>& X, const QVector<bigint>& inds);

struct NeighborhoodData {
    QList<int> channels;
    QVector<double> timepoints;
    DiskWriteMda clips_out;
    QString clips_path;
    DiskWriteMda reduced_clips_out;
    QString reduced_clips_path;
    QVector<int> labels;
    Mda32 templates;
};

struct TimeChunkInfo {
    bigint t_padding; //t1 corresponds to index t_padding (because we have padding/overlap)
    bigint t1; //starting timepoint (corresponding to index t_padding)
    bigint size; //number of timepoints (excluding padding on left and right)
};

void STEP_detect_events(const DiskReadMda32& X, const QMap<int, NeighborhoodData*>& neighborhoods, P_multineighborhood_sort_opts opts)
{
    int progress_msec = 2000;

    bigint M = X.N1();
    bigint N = X.N2();

    bigint RAM_available_for_chunks_bytes = 1e9;
    int num_time_threads = omp_get_max_threads();
    bigint chunk_size = RAM_available_for_chunks_bytes / (M * num_time_threads * 4);
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

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// DETECT
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Detecting events...";
        QTime timer;
        timer.start();
        QTime progress_timer;
        progress_timer.start();
        int num_time_chunks_read = 0;
        QTime timerA;
        timerA.start();
        for (int it = 0; it < time_chunk_infos.count(); it += num_time_threads) {
            QList<TimeChunkInfo> infos = time_chunk_infos.mid(it, num_time_threads);
            QVector<Mda32> time_chunks(infos.count());
            double num_bytes_read = 0;
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                X.readChunk(time_chunks[j], 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
                num_time_chunks_read++;
                num_bytes_read += time_chunks[j].totalSize() * 4;
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for reading: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
#pragma omp parallel for
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                Mda32 time_chunk = time_chunks[j];

                //#pragma omp parallel for
                for (bigint m = 1; m <= M; m++) {
                    NeighborhoodData* ND = neighborhoods[m];
                    QVector<double> vals(time_chunk.N2());
                    for (bigint tt = 0; tt < time_chunk.N2(); tt++) {
                        vals[tt] = time_chunk.value(m - 1, tt);
                    }

                    QVector<double> timepoints0 = detect_events(vals, opts.detect_threshold, opts.detect_interval, opts.detect_sign);
//abort();
#pragma omp critical(store_timepoints)
                    {
                        for (bigint tt = 0; tt < timepoints0.count(); tt++) {
                            double t0 = timepoints0[tt] - TCI.t_padding + TCI.t1;
                            if ((TCI.t1 <= t0) && (t0 < TCI.t1 + TCI.size)) {
                                ND->timepoints << t0;
                            }
                        }
                    }
                }
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for detecting: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
            if (progress_timer.elapsed() > progress_msec) {
                qDebug().noquote() << QString("Detect: Processed %1 time chunks of %2...").arg(num_time_chunks_read).arg(time_chunk_infos.count());
                progress_timer.restart();
            }
        }
        qDebug().noquote() << "Sorting the detected timepoints...";
        // sort the timepoints
        for (int m = 1; m <= M; m++) {
            qSort(neighborhoods[m]->timepoints);
        }
        qDebug().noquote() << "Elapsed (detect): " << timer.elapsed() * 1.0 / 1000;
    }
}

void STEP_extract_clips(const DiskReadMda32& X, const QMap<int, NeighborhoodData*>& neighborhoods, P_multineighborhood_sort_opts opts, QString temp_path)
{
    int progress_msec = 2000;

    bigint M = X.N1();
    bigint N = X.N2();

    bigint RAM_available_for_chunks_bytes = 1e9;
    int num_time_threads = omp_get_max_threads();
    bigint chunk_size = RAM_available_for_chunks_bytes / (M * num_time_threads * 4);
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

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// EXTRACT CLIPS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Extract clips...";
        QTime timer;
        timer.start();
        QTime progress_timer;
        progress_timer.start();
        int num_time_chunks_read = 0;

        // allocate the clips
        for (int m = 1; m <= M; m++) {
            NeighborhoodData* ND = neighborhoods[m];
            int M2 = ND->channels.count();
            int L = ND->timepoints.count();
            ND->clips_path = temp_path + "/" + QString("clips_%1.mda").arg(m);
            ND->reduced_clips_path = temp_path + "/" + QString("reduced_clips_%1.mda").arg(m);
            CacheManager::globalInstance()->setTemporaryFileExpirePid(ND->clips_path, QCoreApplication::applicationPid());
            CacheManager::globalInstance()->setTemporaryFileExpirePid(ND->reduced_clips_path, QCoreApplication::applicationPid());
            ND->clips_out.open(MDAIO_TYPE_FLOAT32, ND->clips_path, M2, opts.clip_size, L);
        }

        QTime timerA;
        timerA.start();
        for (int it = 0; it < time_chunk_infos.count(); it += num_time_threads) {
            QList<TimeChunkInfo> infos = time_chunk_infos.mid(it, num_time_threads);
            QVector<Mda32> time_chunks(infos.count());
            double num_bytes_read = 0;
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                X.readChunk(time_chunks[j], 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
                num_time_chunks_read++;
                num_bytes_read += time_chunks[j].totalSize() * 4;
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for reading: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
#pragma omp parallel for
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                Mda32 time_chunk = time_chunks[j];

                for (bigint m = 1; m <= M; m++) {
                    NeighborhoodData* ND = neighborhoods[m];
                    Mda32 neighborhood_time_chunk;
                    int M2 = ND->channels.count();
                    extract_channels(neighborhood_time_chunk, time_chunk, ND->channels);
                    QVector<bigint> times_inds = get_inds_for_times_in_range(ND->timepoints, TCI.t1, TCI.t1 + TCI.size);
                    QVector<double> times0(times_inds.count());
                    for (bigint j = 0; j < times_inds.count(); j++) {
                        times0[j] = ND->timepoints[times_inds[j]] - TCI.t1 + TCI.t_padding;
                    }
                    Mda32 clips0;
                    extract_clips(clips0, neighborhood_time_chunk, times0, opts.clip_size);
#pragma omp critical(set_extracted_clips)
                    {
                        for (bigint a = 0; a < times_inds.count(); a++) {
                            Mda32 clip0;
                            clips0.getChunk(clip0, 0, 0, a, M2, opts.clip_size, 1);
                            ND->clips_out.writeChunk(clip0, 0, 0, times_inds[a]);
                        }
                    }
                }
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for extracting clips: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
            if (progress_timer.elapsed() > progress_msec) {
                qDebug().noquote() << QString("Extract clips: Processed %1 time chunks of %2...").arg(num_time_chunks_read).arg(time_chunk_infos.count());
                progress_timer.restart();
            }
        }

        // close the clips files
        for (int m = 1; m <= M; m++) {
            neighborhoods[m]->clips_out.close();
        }

        qDebug().noquote() << "Elapsed (extract clips): " << timer.elapsed() * 1.0 / 1000;
    }
}

bool p_multineighborhood_sort(QString timeseries, QString geom, QString firings_out, QString temp_path, const P_multineighborhood_sort_opts& opts)
{
    if (temp_path.isEmpty()) {
        qWarning() << "temporary path is empty.";
        return false;
    }

    //important so we can parallelize in both time and space
    omp_set_nested(1);
    int progress_msec = 2000;

    // The timeseries array (preprocessed - M x N)
    DiskReadMda32 X(timeseries);
    int M = X.N1(); // # channels
    bigint N = X.N2(); // # timepoints

    // The geometry file (dxM - where d is usually 2)
    Mda Geom(geom);

    // Allocate the neighborhood objects
    QMap<int, NeighborhoodData*> neighborhoods;
    for (int m = 1; m <= M; m++) {
        NeighborhoodData* nbhd = new NeighborhoodData;
        nbhd->channels = get_channels_from_geom(Geom, m, opts.adjacency_radius);
        neighborhoods[m] = nbhd;
    }

    int num_neighborhood_threads = qMin(M, omp_get_max_threads());

    bigint RAM_available_for_chunks_bytes = 1e9;
    int num_time_threads = omp_get_max_threads();
    bigint chunk_size = RAM_available_for_chunks_bytes / (M * num_time_threads * 4);
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

    STEP_detect_events(X, neighborhoods, opts);
    STEP_extract_clips(X, neighborhoods, opts, temp_path);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// REDUCE CLIPS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Reduce clips...";
        QTime timer;
        timer.start();
#pragma omp parallel for num_threads(num_neighborhood_threads)
        for (bigint m = 1; m <= M; m++) {
            dimension_reduce_clips(neighborhoods[m]->clips_path, neighborhoods[m]->reduced_clips_path, opts.num_features_per_channel, opts.max_pca_samples);
        }

        qDebug().noquote() << "Elapsed (reduce clips): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// SORT CLIPS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Sort reduced clips...";
        QTime timer;
        timer.start();
        QTime progress_timer;
        progress_timer.start();
        int num_neighborhoods_sorted = 0;

#pragma omp parallel for num_threads(num_neighborhood_threads)
        for (bigint m = 1; m <= M; m++) {
            Sort_clips_opts oo;
            oo.max_samples = opts.max_pca_samples;
            oo.num_features = opts.num_features;
            //neighborhoods[m]->reduced_clips.write32(QString("/home/magland/tmp/reduced_clips_%1.mda").arg(m));
            //neighborhoods[m]->clips.write32(QString("/home/magland/tmp/clips_%1.mda").arg(m));
            Mda32 reduced_clips(neighborhoods[m]->reduced_clips_path);
            neighborhoods[m]->labels = sort_clips(reduced_clips, oo);
#pragma omp critical(sort_progress)
            {
                num_neighborhoods_sorted++;
                if (progress_timer.elapsed() > progress_msec) {
                    qDebug().noquote() << QString("Sorted %1 neighborhoods of %2...").arg(num_neighborhoods_sorted).arg(M);
                    progress_timer.restart();
                }
            }
        }

        qDebug().noquote() << "Elapsed (Sort reduced clips): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// COMPUTE TEMPLATES IN NEIGHBORHOODS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Compute templates in neighborhoods...";
        QTime timer;
        timer.start();
#pragma omp parallel for num_threads(num_neighborhood_threads)
        for (bigint m = 1; m <= M; m++) {
            Mda32 clips0(neighborhoods[m]->clips_path);
            neighborhoods[m]->templates = compute_templates_from_clips(clips0, neighborhoods[m]->labels);
        }

        qDebug().noquote() << "Elapsed (Compute templates in neighborhoods): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// CONSOLIDATE CLUSTERS IN NEIGHBORHOODS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.consolidate_clusters) {
        qDebug().noquote() << "******* Consolidate clusters in neighborhoods...";
        QTime timer;
        timer.start();
#pragma omp parallel for num_threads(num_neighborhood_threads)
        for (bigint m = 1; m <= M; m++) {
            Consolidate_clusters_opts oo;
            QMap<int, int> label_map = consolidate_clusters(neighborhoods[m]->templates, oo);
            QVector<int>* labels = &neighborhoods[m]->labels;
            QVector<bigint> inds_to_keep;
            for (bigint i = 0; i < labels->count(); i++) {
                int k0 = (*labels)[i];
                if (label_map[k0] > 0)
                    inds_to_keep << i;
            }
            neighborhoods[m]->timepoints = get_subarray(neighborhoods[m]->timepoints, inds_to_keep);
            neighborhoods[m]->labels = get_subarray(neighborhoods[m]->labels, inds_to_keep);
            for (bigint i = 0; i < neighborhoods[m]->labels.count(); i++) {
                int k0 = label_map[neighborhoods[m]->labels[i]];
                neighborhoods[m]->labels[i] = k0;
            }
        }

        qDebug().noquote() << "Elapsed (Consolidate clusters in neighborhoods): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// COLLECT EVENTS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QVector<double> times;
    QVector<int> labels;
    QVector<int> central_channels;
    {
        qDebug().noquote() << "******* Collect events...";
        QTime timer;
        timer.start();
        int label_offset = 0;
        for (bigint m = 1; m <= M; m++) {
            QVector<double>* times0 = &neighborhoods[m]->timepoints;
            QVector<int>* labels0 = &neighborhoods[m]->labels;
            for (bigint i = 0; i < times0->count(); i++) {
                if ((*labels0)[i] > 0) {
                    times << (*times0)[i];
                    labels << (*labels0)[i] + label_offset;
                    central_channels << m;
                }
            }
            label_offset += MLCompute::max(*labels0);
        }
        sort_events_by_time(times, labels, central_channels);
        qDebug().noquote() << "Elapsed (Collect events): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// COMPUTE GLOBAL TEMPLATES
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mda32 templates;
    {
        qDebug().noquote() << "******* Compute global templates...";
        QTime timer;
        timer.start();
        templates = compute_templates_in_parallel(X, times, labels, opts.clip_size);

        qDebug().noquote() << "Elapsed (Compute global templates): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// MERGE ACROSS CHANNELS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.merge_across_channels) {
        qDebug().noquote() << "******* Merge across channels...";
        QTime timer;
        timer.start();
        Merge_across_channels_opts oo;
        oo.clip_size = opts.clip_size;
        merge_across_channels(times, labels, central_channels, templates, oo);
        templates = compute_templates_in_parallel(X, times, labels, opts.clip_size);

        qDebug().noquote() << "Elapsed (Merge across channels): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// FIT STAGE
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Fit stage...";
        QTime timer;
        timer.start();
        QTime progress_timer;
        progress_timer.start();
        int num_time_chunks_read = 0;

        QVector<bigint> inds_to_use;

        QTime timerA;
        timerA.start();
        for (int it = 0; it < time_chunk_infos.count(); it += num_time_threads) {
            QList<TimeChunkInfo> infos = time_chunk_infos.mid(it, num_time_threads);
            QVector<Mda32> time_chunks(infos.count());
            double num_bytes_read = 0;
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                X.readChunk(time_chunks[j], 0, TCI.t1 - TCI.t_padding, M, TCI.size + 2 * TCI.t_padding);
                num_time_chunks_read++;
                num_bytes_read += time_chunks[j].totalSize() * 4;
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for reading: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
#pragma omp parallel for
            for (int j = 0; j < infos.count(); j++) {
                TimeChunkInfo TCI = infos[j];
                Mda32 time_chunk = time_chunks[j];

                QVector<bigint> local_inds;
                QVector<double> local_times;
                QVector<int> local_labels;

                bigint ii = 0;
                while ((ii < times.count()) && (times[ii] < TCI.t1))
                    ii++;
                while ((ii < times.count()) && (times[ii] < TCI.t1 + TCI.size)) {
                    local_inds << ii;
                    local_times << times[ii] - TCI.t1 + chunk_overlap_size;
                    local_labels << labels[ii];
                    ii++;
                }
                Fit_stage_opts oo;
                QVector<bigint> local_inds_to_use = fit_stage(time_chunk, local_times, local_labels, templates, oo);
#pragma omp critical(fit_stage_set_inds_to_use1)
                {
                    for (bigint a = 0; a < local_inds_to_use.count(); a++) {
                        inds_to_use << local_inds[local_inds_to_use[a]];
                    }
                }
            }
            {
                double elapsed_sec = timerA.elapsed() * 1.0 / 1000;
                qDebug().noquote() << QString("Elapsed for fit stage: %1 (%2 MB/sec)").arg(timerA.restart()).arg(num_bytes_read / 1e6 / (elapsed_sec));
            }
            if (progress_timer.elapsed() > progress_msec) {
                qDebug().noquote() << QString("Fit stage: Processed %1 time chunks of %2...").arg(num_time_chunks_read).arg(time_chunk_infos.count());
                progress_timer.restart();
            }
        }

        qSort(inds_to_use);
        qDebug().noquote() << QString("Fit stage: Using %1 of %2 events").arg(inds_to_use.count()).arg(times.count());

        QVector<double> times2(inds_to_use.count());
        QVector<int> labels2(inds_to_use.count());
        QVector<int> central_channels2(inds_to_use.count());
        for (bigint a = 0; a < inds_to_use.count(); a++) {
            times2[a] = times[inds_to_use[a]];
            labels2[a] = labels[inds_to_use[a]];
            central_channels2[a] = central_channels[inds_to_use[a]];
        }
        times = times2;
        labels = labels2;
        central_channels = central_channels2;

        qDebug().noquote() << "Computing global templates...";
        templates = compute_templates_in_parallel(X, times, labels, opts.clip_size);

        qDebug().noquote() << "Elapsed (Fit stage): " << timer.elapsed() * 1.0 / 1000;
    }

    /*
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// FIT STAGE
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (opts.fit_stage) {
        qDebug().noquote() << "******* Fit stage...";
        QTime timer; timer.start();
        QTime progress_timer; progress_timer.start();
        int num_time_chunks_read=0;

        QVector<bigint> inds_to_use;

#pragma omp parallel for num_threads(num_time_threads) //parallel in time
        for (int it=0; it<time_chunk_infos.count(); it++) {
            TimeChunkInfo TCI=time_chunk_infos[it];
            double t=TCI.t1;
            Mda32 time_chunk;
#pragma omp critical(read_time_chunk)
            {
                X.readChunk(time_chunk,0,TCI.t1-TCI.t_padding,M,TCI.size+2*TCI.t_padding);
                num_time_chunks_read++;
                if (progress_timer.elapsed()>progress_msec) {
                    qDebug().noquote() << QString("Fit stage: Read %1 time chunks of %2...").arg(num_time_chunks_read).arg(time_chunk_infos.count());
                    progress_timer.restart();
                }
            }

            QVector<bigint> local_inds;
            QVector<double> local_times;
            QVector<int> local_labels;

            bigint ii=0;
            while ((ii<times.count())&&(times[ii]<t)) ii++;
            while ((ii<times.count())&&(times[ii]<t+TCI.size)) {
                local_inds << ii;
                local_times << times[ii]-t+chunk_overlap_size;
                local_labels << labels[ii];
                ii++;
            }
            Fit_stage_opts oo;
            oo.clip_size=opts.clip_size;
            QVector<bigint> local_inds_to_use=fit_stage(time_chunk,local_times,local_labels,templates,oo);
#pragma omp critical(fit_stage_set_inds_to_use1)
            {
                for (bigint a=0; a<local_inds_to_use.count(); a++) {
                    inds_to_use << local_inds[local_inds_to_use[a]];
                }
            }
        } // done with parallel in time

        qSort(inds_to_use);
        qDebug().noquote() << QString("Fit stage: Using %1 of %2 events").arg(inds_to_use.count()).arg(times.count());

        QVector<double> times2(inds_to_use.count());
        QVector<int> labels2(inds_to_use.count());
        QVector<int> central_channels2(inds_to_use.count());
        for (bigint a=0; a<inds_to_use.count(); a++) {
            times2[a]=times[inds_to_use[a]];
            labels2[a]=labels[inds_to_use[a]];
            central_channels2[a]=central_channels[inds_to_use[a]];
        }
        times=times2;
        labels=labels2;
        central_channels=central_channels2;

        qDebug().noquote() << "Computing global templates...";
        templates=compute_templates_in_parallel(X,times,labels,opts.clip_size);

        qDebug().noquote() << "Elapsed (Fit stage): " << timer.elapsed()*1.0/1000;
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// REORDER LABELS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Reorder labels...";
        QTime timer;
        timer.start();
        QMap<int, int> label_map = reorder_labels(templates);
        QVector<double> times2;
        QVector<int> labels2;
        QVector<int> central_channels2;
        for (bigint i = 0; i < times.count(); i++) {
            int k0 = labels[i];
            int k1 = label_map.value(k0, 0);
            if (k1 > 0) {
                times2 << times[i];
                labels2 << k1;
                central_channels2 << central_channels[i];
            }
        }
        times = times2;
        labels = labels2;
        central_channels = central_channels2;

        qDebug().noquote() << "Elapsed (Reorder labels): " << timer.elapsed() * 1.0 / 1000;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// CREATE FIRINGS OUTPUT
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        qDebug().noquote() << "******* Create firings output...";
        QTime timer;
        timer.start();

        bigint L = times.count();
        Mda firings(3, L);
        for (bigint i = 0; i < L; i++) {
            firings.set(central_channels[i], 0, i);
            firings.set(times[i], 1, i);
            firings.set(labels[i], 2, i);
        }

        firings.write64(firings_out);

        qDebug().noquote() << "Elapsed (Create firings output): " << timer.elapsed() * 1.0 / 1000;
    }

    return true;
}

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

void sort_events_by_time(QVector<double>& times, QVector<int>& labels, QVector<int>& central_channels)
{
    QList<bigint> inds = get_sort_indices_bigint(times);
    QVector<double> times2(times.count());
    QVector<int> labels2(times.count());
    QVector<int> central_channels2(times.count());
    for (bigint i = 0; i < inds.count(); i++) {
        times2[i] = times[inds[i]];
        labels2[i] = labels[inds[i]];
        central_channels2[i] = central_channels[inds[i]];
    }
    times = times2;
    labels = labels2;
    central_channels = central_channels2;
}

QVector<bigint> get_inds_for_times_in_range(const QVector<double>& times, double t1, double t2)
{
    QVector<bigint> ret;
    bigint ii = 0;
    while ((ii < times.count()) && (times[ii] < t1)) {
        ii++;
    }
    while ((ii < times.count()) && (times[ii] < t2)) {
        ret << ii;
        ii++;
    }
    return ret;
}

void extract_clips(Mda32& clips_out, const Mda32& timeseries, const QVector<double>& times, int clip_size)
{
    int M = timeseries.N1();
    //bigint N=timeseries.N2();
    int T = clip_size;
    bigint L = times.count();
    bigint Tmid = (bigint)((T + 1) / 2) - 1;
    clips_out.allocate(M, T, L);
    for (bigint i = 0; i < L; i++) {
        bigint t1 = times[i] - Tmid;
        //bigint t2 = t1 + T - 1;
        Mda32 tmp;
        timeseries.getChunk(tmp, 0, t1, M, T);
        clips_out.setChunk(tmp, 0, 0, i);
    }
}

Mda32 compute_templates_from_clips(const Mda32& clips, const QVector<int>& labels)
{
    int M = clips.N1();
    int T = clips.N2();
    bigint L = clips.N3();
    int K = MLCompute::max(labels);
    Mda sums(M, T, K);
    QVector<bigint> counts(K, 0);
#pragma omp parallel
    {
        Mda local_sums(M, T, K);
        QVector<bigint> local_counts(K, 0);
#pragma omp for
        for (bigint i = 0; i < L; i++) {
            int k0 = labels[i];
            if (k0 > 0) {
                Mda32 tmp;
                clips.getChunk(tmp, 0, 0, i, M, T, 1);
                for (int t = 0; t < T; t++) {
                    for (int m = 0; m < M; m++) {
                        local_sums.set(local_sums.get(m, t, k0 - 1) + tmp.get(m, t), m, t, k0 - 1);
                    }
                }
                local_counts[k0 - 1]++;
            }
        }
#pragma omp critical(set_sums_and_counts)
        for (int kk = 1; kk <= K; kk++) {
            counts[kk - 1] += local_counts[kk - 1];
            Mda tmp;
            local_sums.getChunk(tmp, 0, 0, kk - 1, M, T, 1);
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    sums.set(sums.get(m, t, kk - 1) + tmp.get(m, t), m, t, kk - 1);
                }
            }
        }
    }
    Mda32 templates(M, T, K);
    for (int kk = 1; kk <= K; kk++) {
        if (counts[kk - 1]) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    templates.set(sums.get(m, t, kk - 1) / counts[kk - 1], m, t, kk - 1);
                }
            }
        }
    }
    return templates;
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
