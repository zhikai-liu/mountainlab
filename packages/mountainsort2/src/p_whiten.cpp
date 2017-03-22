#include "p_whiten.h"

#include <QTime>
#include <diskreadmda32.h>
#include <diskwritemda.h>
#include <mda.h>
#include "pca.h"
#include "omp.h"

namespace P_whiten {

double quantize(float X, double unit)
{
    return (floor(X / unit + 0.5)) * unit;
}

void quantize(bigint N, float* X, double unit)
{
    for (bigint i = 0; i < N; i++) {
        X[i] = quantize(X[i], unit);
    }
}
}

bool p_whiten(QString timeseries, QString timeseries_out, Whiten_opts opts)
{
    (void)opts;

    DiskReadMda32 X(timeseries);
    bigint M = X.N1();
    bigint N = X.N2();

    bigint processing_chunk_size = 1e7;

    Mda XXt(M, M);
    double* XXtptr = XXt.dataPtr();
    bigint chunk_size = processing_chunk_size;
    if (N < processing_chunk_size) {
        chunk_size = N;
    }

    {
        QTime timer;
        timer.start();
        bigint num_timepoints_handled = 0;
#pragma omp parallel for
        for (bigint timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda32 chunk;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            float* chunkptr = chunk.dataPtr();
            Mda XXt0(M, M);
            double* XXt0ptr = XXt0.dataPtr();
            for (bigint i = 0; i < chunk.N2(); i++) {
                bigint aa = M * i;
                bigint bb = 0;
                for (bigint m1 = 0; m1 < M; m1++) {
                    for (bigint m2 = 0; m2 < M; m2++) {
                        XXt0ptr[bb] += chunkptr[aa + m1] * chunkptr[aa + m2];
                        bb++;
                    }
                }
            }
#pragma omp critical(lock2)
            {
                bigint bb = 0;
                for (bigint m1 = 0; m1 < M; m1++) {
                    for (bigint m2 = 0; m2 < M; m2++) {
                        XXtptr[bb] += XXt0ptr[bb];
                        bb++;
                    }
                }
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%ld/%ld (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    if (N > 1) {
        for (bigint ii = 0; ii < M * M; ii++) {
            XXtptr[ii] /= (N - 1);
        }
    }

    //Mda AA = get_whitening_matrix(COV);
    Mda WW;
    whitening_matrix_from_XXt(WW, XXt); // the result is symmetric (assumed below)
    double* WWptr = WW.dataPtr();

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, timeseries_out, M, N);
    {
        QTime timer;
        timer.start();
        bigint num_timepoints_handled = 0;
#pragma omp parallel for
        for (bigint timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda32 chunk_in;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk_in, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            float* chunk_in_ptr = chunk_in.dataPtr();
            Mda32 chunk_out(M, chunk_in.N2());
            float* chunk_out_ptr = chunk_out.dataPtr();
            for (bigint i = 0; i < chunk_in.N2(); i++) { // explicitly do mat-mat mult ... TODO replace w/ BLAS3
                bigint aa = M * i;
                bigint bb = 0;
                for (bigint m1 = 0; m1 < M; m1++) {
                    for (bigint m2 = 0; m2 < M; m2++) {
                        chunk_out_ptr[aa + m1] += chunk_in_ptr[aa + m2] * WWptr[bb]; // actually this does dgemm w/ WW^T
                        bb++; // but since symmetric, doesn't matter.
                    }
                }
            }
#pragma omp critical(lock2)
            {
                // The following is needed to make the output deterministic, due to a very tricky floating-point problem that I honestly could not track down
                // It has something to do with multiplying by very small values of WWptr[bb]. But I truly could not pinpoint the exact problem.
                P_whiten::quantize(chunk_out.totalSize(), chunk_out.dataPtr(), 0.0001);
                Y.writeChunk(chunk_out, 0, timepoint);
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%ld/%ld (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    Y.close();

    return true;
}

bool p_compute_whitening_matrix(QStringList timeseries_list, QString whitening_matrix_out, Whiten_opts opts)
{
    (void)opts;

    DiskReadMda32 X0(2, timeseries_list);
    bigint M = X0.N1();
    bigint N = X0.N2();
    qDebug().noquote() << "Computing whitening matrix: M/N" << M << N;

    bigint processing_chunk_size = 1e7;

    Mda XXt(M, M);
    double* XXtptr = XXt.dataPtr();
    bigint chunk_size = processing_chunk_size;
    if (N < processing_chunk_size) {
        chunk_size = N;
    }

    bigint timepoint = 0;
    while (timepoint < N) {
        QList<Mda32> chunks;
        qDebug() << "---------------------" << omp_get_num_threads();
        while ((timepoint < N) && (chunks.count() < omp_get_max_threads())) {
            Mda32 chunk0;
            X0.readChunk(chunk0, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            chunks << chunk0;
            timepoint += chunk_size;
        }
        bigint num_chunks = chunks.count();
        qDebug() << QString("Processing %1 chunks. timepoint %2. (%3%)").arg(num_chunks).arg(timepoint).arg((int)(timepoint * 1.0 / N * 100));
#pragma omp parallel
        {
#pragma omp for
            for (bigint i = 0; i < num_chunks; i++) {
                Mda32 chunk0;
#pragma omp critical
                {
                    chunk0 = chunks[i];
                }
                Mda XXt0(M, M);
                double* XXt0ptr = XXt0.dataPtr();
                float* chunkptr = chunk0.dataPtr();
                for (bigint i = 0; i < chunk0.N2(); i++) {
                    bigint aa = M * i;
                    bigint bb = 0;
                    for (bigint m1 = 0; m1 < M; m1++) {
                        for (bigint m2 = 0; m2 < M; m2++) {
                            XXt0ptr[bb] += chunkptr[aa + m1] * chunkptr[aa + m2];
                            bb++;
                        }
                    }
                }
#pragma omp critical(lock2)
                {
                    bigint bb = 0;
                    for (bigint m1 = 0; m1 < M; m1++) {
                        for (bigint m2 = 0; m2 < M; m2++) {
                            XXtptr[bb] += XXt0ptr[bb];
                            bb++;
                        }
                    }
                }
            }
        }
    }

    if (N > 1) {
        for (bigint ii = 0; ii < M * M; ii++) {
            XXtptr[ii] /= (N - 1);
        }
    }

    //Mda AA = get_whitening_matrix(COV);
    Mda WW;
    whitening_matrix_from_XXt(WW, XXt); // the result is symmetric (assumed below)

    return WW.write64(whitening_matrix_out);
}

bool p_whiten_clips(QString clips_path, QString whitening_matrix, QString clips_out_path, Whiten_opts opts)
{
    (void)opts;
    Mda WW(whitening_matrix);
    DiskReadMda32 clips(clips_path);

    double* WWptr = WW.dataPtr();

    bigint M = clips.N1();
    bigint T = clips.N2();
    bigint L = clips.N3();

    qDebug().noquote() << "Whitening..." << M << T << L;

    DiskWriteMda clips_out(MDAIO_TYPE_FLOAT32, clips_out_path, M, T, L);

    for (bigint i = 0; i < L; i++) {
        Mda32 chunk;
        if (!clips.readChunk(chunk, 0, 0, i, M, T, 1)) {
            qWarning() << "Problem reading chunk" << i;
            return false;
        }
        float* chunk_ptr = chunk.dataPtr();

        Mda32 chunk_out(M, T);
        float* chunk_out_ptr = chunk_out.dataPtr();
        {

            for (bigint t = 0; t < T; t++) {
                bigint aa = 0;
                bigint bb = M * t;
                for (bigint m1 = 0; m1 < M; m1++) {
                    for (bigint m2 = 0; m2 < M; m2++) {
                        chunk_out_ptr[bb + m1] += chunk_ptr[bb + m2] * WWptr[aa]; // actually this does dgemm w/ WW^T
                        aa++; // but since symmetric, doesn't matter.
                    }
                }
            }
        }
        if (!clips_out.writeChunk(chunk_out, 0, 0, i)) {
            qWarning() << "Problem writing chunk";
            return false;
        }
    }

    /*
    {
        // The following is needed to make the output deterministic, due to a very tricky floating-point problem that I honestly could not track down
        // It has something to do with multiplying by very small values of WWptr[bb]. But I truly could not pinpoint the exact problem.
        P_whiten::quantize(clips_out.totalSize(), clips_out.dataPtr(), 0.0001);
    }
    */

    return true;
}

bool p_apply_whitening_matrix(QString timeseries, QString whitening_matrix, QString timeseries_out, Whiten_opts opts)
{
    (void)opts;

    DiskReadMda32 X(timeseries);
    bigint M = X.N1();
    bigint N = X.N2();

    bigint processing_chunk_size = 1e7;
    bigint chunk_size = processing_chunk_size;
    if (N < processing_chunk_size) {
        chunk_size = N;
    }

    Mda WW(whitening_matrix);

    double* WWptr = WW.dataPtr();

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, timeseries_out, M, N);
    {
        QTime timer;
        timer.start();
        bigint num_timepoints_handled = 0;
#pragma omp parallel for
        for (bigint timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda32 chunk_in;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk_in, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            float* chunk_in_ptr = chunk_in.dataPtr();
            Mda32 chunk_out(M, chunk_in.N2());
            float* chunk_out_ptr = chunk_out.dataPtr();
            for (bigint i = 0; i < chunk_in.N2(); i++) { // explicitly do mat-mat mult ... TODO replace w/ BLAS3
                bigint aa = M * i;
                bigint bb = 0;
                for (bigint m1 = 0; m1 < M; m1++) {
                    for (bigint m2 = 0; m2 < M; m2++) {
                        chunk_out_ptr[aa + m1] += chunk_in_ptr[aa + m2] * WWptr[bb]; // actually this does dgemm w/ WW^T
                        bb++; // but since symmetric, doesn't matter.
                    }
                }
            }
#pragma omp critical(lock2)
            {
                // The following is needed to make the output deterministic, due to a very tricky floating-point problem that I honestly could not track down
                // It has something to do with multiplying by very small values of WWptr[bb]. But I truly could not pinpoint the exact problem.
                P_whiten::quantize(chunk_out.totalSize(), chunk_out.dataPtr(), 0.0001);
                Y.writeChunk(chunk_out, 0, timepoint);
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%ld/%ld (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    Y.close();

    return true;
}
