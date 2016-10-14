#include "isosplit2_w.h"
#include "isosplit2.h"
#include "isocut_w.h"
#include "pca.h"

namespace Isosplit2_w {

double distance_between_vectors(int M, float* v1, float* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

double distance_between_vectors(int M, const double* v1, const double* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

double distance_between_vectors(int M, const double* v1, const float* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

bool was_already_attempted_w(int M, const AttemptedComparisons_w& attempted_comparisons, float* center1, float* center2, double count1, double count2, double repeat_tolerance)
{
    double tol = repeat_tolerance;
    for (int i = 0; i < attempted_comparisons.counts1.count(); i++) {
        double diff_count1 = fabs(attempted_comparisons.counts1[i] - count1);
        double avg_count1 = (attempted_comparisons.counts1[i] + count1) / 2;
        if (diff_count1 <= tol * avg_count1) {
            double diff_count2 = fabs(attempted_comparisons.counts2[i] - count2);
            double avg_count2 = (attempted_comparisons.counts2[i] + count2) / 2;
            if (diff_count2 <= tol * avg_count2) {
#if 0
                float C1[M];
                for (int m = 0; m < M; m++)
                    C1[m] = attempted_comparisons.centers1[i * M + m];
                float C2[M];
                for (int m = 0; m < M; m++)
                    C2[m] = attempted_comparisons.centers2[i * M + m];
#else
                // We don't have to copy into C1 and C2
                // attempted_comparisons.centers{1,2} already contain all the data we need
                // we just need to point to the proper section of the data
                // the only problem is these are doubles and not floats
                // but this shouldn't affect speed that much
                const double* C1 = attempted_comparisons.centers1.constData() + (i * M);
                const double* C2 = attempted_comparisons.centers2.constData() + (i * M);
#endif
                const double dist0 = distance_between_vectors(M, C1, C2);
                if (dist0 > 0) {
                    double dist1 = distance_between_vectors(M, C1, center1);
                    double dist2 = distance_between_vectors(M, C2, center2);
                    double frac1 = dist1 / dist0;
                    double frac2 = dist2 / dist0;
                    if ((frac1 <= tol * 1 / sqrt(count1)) && (frac2 <= tol * 1 / sqrt(count2))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void find_next_comparison_w(int M, int K, int& k1, int& k2, const bool* active_labels, float* Cptr, double* counts, const AttemptedComparisons_w& attempted_comparisons, double repeat_tolerance)
{
    QVector<int> active_inds;
    active_inds.reserve(1 + K / 2); // just in case => going to realloc at most once
    for (int k = 0; k < K; k++)
        if (active_labels[k])
            active_inds << k;
    if (active_inds.isEmpty()) {
        k1 = -1;
        k2 = -1;
        return;
    }
    int Nactive = active_inds.count();
    double dists[Nactive][Nactive];
    for (int a = 0; a < Nactive; a++) {
        for (int b = 0; b < Nactive; b++) {
            dists[a][b] = distance_between_vectors(M, &Cptr[active_inds[a] * M], &Cptr[active_inds[b] * M]);
        }
        dists[a][a] = -1; //don't use it
    }
    while (true) {
        int best_a = -1, best_b = -1;
        double best_dist = -1;
        for (int a = 0; a < Nactive; a++) {
            for (int b = 0; b < Nactive; b++) {
                double val = dists[a][b];
                if (val >= 0) {
                    if ((best_dist < 0) || (val < best_dist)) {
                        best_a = a;
                        best_b = b;
                        best_dist = val;
                    }
                }
            }
        }
        if (best_a < 0) {
            k1 = -1;
            k2 = -1;
            return;
        }
        k1 = active_inds[best_a];
        k2 = active_inds[best_b];
        if ((counts[k1] > 0) && (counts[k2] > 0)) { //just to make sure (zero was actually happening some times, but not sure why)
            if (!was_already_attempted_w(M, attempted_comparisons, &Cptr[k1 * M], &Cptr[k2 * M], counts[k1], counts[k2], repeat_tolerance)) {
                //hurray!
                return;
            }
        }
        dists[best_a][best_b] = -1;
        dists[best_b][best_a] = -1;
    }
    k1 = -1;
    k2 = -1;
}

double sum_of_weights_at_inds(const QVector<float>& weights, const QList<long>& inds)
{
    double sum = 0;
    for (int i = 0; i < inds.count(); i++) {
        sum += weights[inds[i]];
    }
    return sum;
}

Mda32 matrix_multiply_transposed_isosplit_w(const Mda32& A, const QVector<float>& weights)
{
    int N1 = A.N1();
    int N2 = A.N2();

    Mda32 ret(N1, N1);
    for (int j = 0; j < N1; j++) {
        for (int i = 0; i < j + 1; i++) {
            double val = 0;
            for (int k = 0; k < N2; k++) {
                val += A.get(i, k) * A.get(j, k) * weights[k];
            }
            ret.set(val, i, j);
            ret.set(val, j, i); // resulting matrix is symmetric
        }
    }
    return ret;
}

QVector<double> compute_centroid_w(Mda32& X, const QVector<float>& weights)
{
    int M = X.N1();
    int N = X.N2();
    QVector<double> ret(M, 0);
    double denom = 0;
    for (int n = 0; n < N; n++) {
        denom += weights[n];
        for (int m = 0; m < M; m++) {
            ret[m] += X.value(m, n) * weights[n];
        }
    }
    if (denom) {
        for (int i = 0; i < M; i++)
            ret[i] /= denom;
    }
    return ret;
}

void whiten_two_clusters_w(double* V, Mda32& X1, const QVector<float>& weights1, Mda32& X2, const QVector<float>& weights2)
{
    int M = X1.N1();
    int N1 = X1.N2();
    int N2 = X2.N2();
    int N = N1 + N2;

    QVector<double> center1 = compute_centroid_w(X1, weights1);
    QVector<double> center2 = compute_centroid_w(X2, weights2);

    if (M < N) { //otherwise there are too few points to whiten

        Mda32 XX(M, N);
        QVector<float> WW;
        WW.reserve(N);
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < M; j++) {
                XX.set(X1.get(j, i) - center1[j], j, i);
            }
            WW << weights1[i];
        }
        for (int i = 0; i < N2; i++) {
            for (int j = 0; j < M; j++) {
                XX.set(X2.get(j, i) - center2[j], j, i + N1);
            }
            WW << weights2[i];
        }

        Mda32 XXt = matrix_multiply_transposed_isosplit_w(XX, WW);

        //Mda32 W = get_whitening_matrix_isosplit(COV);
        Mda32 W;
        whitening_matrix_from_XXt(W, XXt);
        X1 = matrix_multiply_isosplit(W, X1);
        X2 = matrix_multiply_isosplit(W, X2);
    }

    //compute the vector
    center1 = compute_centroid_w(X1, weights1);
    center2 = compute_centroid_w(X2, weights2);
    for (int m = 0; m < M; m++) {
        V[m] = center2[m] - center1[m];
    }
}

//#include "jsvm.h"
QVector<int> test_redistribute_w(bool& do_merge, Mda32& Y1, const QVector<float>& weights1, Mda32& Y2, const QVector<float>& weights2, double isocut_threshold)
{
    Mda32 X1;
    X1 = Y1;
    Mda32 X2;
    X2 = Y2;
    int M = X1.N1();
    int N1 = X1.N2();
    int N2 = X2.N2();
    QVector<int> ret(N1 + N2, 1);
    do_merge = true;
    double V[M];
    /*
    bool use_svm = false;
    if (use_svm) {
        Mda32 X_00(X1.N1(), X1.N2() + X2.N2());
        QVector<int> labels_00;
        for (long ii = 0; ii < X1.N2(); ii++) {
            for (int jj = 0; jj < X1.N1(); jj++) {
                X_00.setValue(X1.value(jj, ii), jj, ii);
            }
            labels_00 << 1;
        }
        for (long ii = 0; ii < X2.N2(); ii++) {
            for (int jj = 0; jj < X1.N1(); jj++) {
                X_00.setValue(X2.value(jj, ii), jj, ii + X1.N2());
            }
            labels_00 << 2;
        }
        double cutoff_00;
        QVector<double> direction_00;
        get_svm_discrim_direction(cutoff_00, direction_00, X_00, labels_00);
        for (int mm = 0; mm < M; mm++) {
            V[mm] = direction_00[mm];
        }
    }
    else {
    */
    whiten_two_clusters_w(V, X1, weights1, X2, weights2);
    //}
    double normv = 0;
    {
        double sumsqr = 0;
        for (int m = 0; m < M; m++)
            sumsqr += V[m] * V[m];
        normv = sqrt(sumsqr);
    }
    if (!normv) {
        printf("Warning: isosplit2: vector V is null.\n");
        return ret;
    }
    if (N1 + N2 <= 5) {
        //avoid a crash?
        return ret;
    }

    //project onto line connecting the centers
    QVector<double> XX;
    QVector<double> WW;
    XX.reserve(N1 + N2);
    WW.reserve(N1 + N2);
    for (int i = 0; i < N1; i++) {
        double val = 0;
        for (int m = 0; m < M; m++)
            val += X1.value(m, i) * V[m];
        XX << val;
        WW << weights1[i];
    }
    for (int i = 0; i < N2; i++) {
        double val = 0;
        for (int m = 0; m < M; m++)
            val += X2.value(m, i) * V[m];
        XX << val;
        WW << weights2[i];
    }
    double cutpoint;
    bool do_cut = isocut_w(N1 + N2, &cutpoint, XX.constData(), WW.constData(), isocut_threshold, 5);

    if (do_cut) {
        do_merge = false;
        for (int i = 0; i < N1 + N2; i++) {
            if (XX[i] <= cutpoint)
                ret[i] = 1;
            else
                ret[i] = 2;
        }
    }
    return ret;
}

QVector<int> test_redistribute_w(bool& do_merge, Mda32& X, const QVector<float>& weights, const QList<long>& inds1, const QList<long>& inds2, double isocut_threshold)
{
    int M = X.N1();
    Mda32 X1(M, inds1.count());
    QVector<float> weights1(inds1.count());
    for (int i = 0; i < inds1.count(); i++) {
        weights1[i] = weights[inds1[i]];
        for (int m = 0; m < M; m++) {
            X1.setValue(X.value(m, inds1[i]), m, i);
        }
    }
    Mda32 X2(M, inds2.count());
    QVector<float> weights2(inds2.count());
    for (int i = 0; i < inds2.count(); i++) {
        weights2[i] = weights[inds2[i]];
        for (int m = 0; m < M; m++) {
            X2.setValue(X.value(m, inds2[i]), m, i);
        }
    }
    return test_redistribute_w(do_merge, X1, weights1, X2, weights2, isocut_threshold);
}

void geometric_median_w(int M, int N, double* ret, const double* X, const QVector<float>& WW)
{
    int num_iterations = 10;
    if (N == 0)
        return;
    if (N == 1) {
        std::copy(X, X + M, ret);
        return;
    }
    std::vector<double> weights(N, 1);
    for (int it = 1; it <= num_iterations; it++) {
        for (int m = 0; m < M; m++)
            ret[m] = 0;
        int aa = 0;
        double denom = 0;
        for (int n = 0; n < N; n++) {
            double w0 = weights[n];
            if (!WW.isEmpty())
                w0 *= WW[n];
            denom += w0;
            for (int m = 0; m < M; m++) {
                ret[m] += X[aa] * w0;
                aa++;
            }
        }
        if (denom) {
            for (int m = 0; m < M; m++) {
                ret[m] /= denom;
            }
        }
        aa = 0;
        for (int n = 0; n < N; n++) {
            double sumsqr_diff = 0;
            for (int m = 0; m < M; m++) {
                double val = X[aa] - ret[m];
                sumsqr_diff = val * val;
                aa++;
            }
            if (sumsqr_diff != 0) {
                weights[n] = 1 / sqrt(sumsqr_diff);
            }
            else
                weights[n] = 0;
        }
    }
}

QVector<double> compute_center_w(Mda32& X, const QList<long>& inds, const QVector<float>& weights)
{
    int M = X.N1();
    int NN = inds.count();
    if (NN == 0) {
        return QVector<double>(M, 0);
    }
    QVector<double> XX(M * NN);
    int aa = 0;
    for (int n = 0; n < NN; n++) {
        for (int m = 0; m < M; m++) {
            XX[aa] = X.value(m, inds[n]);
            aa++;
        }
    }
    if (NN == 1) {
        // for NN = 1 geometric_median() returns the original data
        // but it copies it to the target vector.
        // We can avoid it by returning the original vector
        XX.resize(M); // it's already M but let's explicitly make sure of that
        return XX;
    }
    QVector<double> result(M);
    geometric_median_w(M, NN, result.data(), XX.constData(), weights);
    return result;
}

Mda32 compute_centers_w(Mda32& X, const QVector<int>& labels, int K, const QVector<float>& weights)
{
    int M = X.N1();
    //int N=X.N2();
    Mda32 ret(M, K);
    for (int k = 0; k < K; k++) {
        QList<long> inds = find_inds(labels, k);
        QVector<double> ctr = compute_center_w(X, inds, weights);
        for (int m = 0; m < M; m++)
            ret.set(ctr[m], m, k);
    }
    return ret;
}
}

QVector<int> isosplit2_w(Mda32& X, const QVector<float>& weights, float isocut_threshold, int K_init, bool verbose)
{
    double repeat_tolerance = 0.2;

    int M = X.N1();
    int N = X.N2();
    QVector<int> labels = do_kmeans(X, K_init); //i believe there is no need for weights here. This is the initialization. Maybe it's even better not to use weights, so that the dense regions don't dominate.

    QVector<bool> active_labels(K_init, true);
    Mda32 centers = Isosplit2_w::compute_centers_w(X, labels, K_init, weights); //M x K_init
    double counts[K_init];
    for (int ii = 0; ii < K_init; ii++)
        counts[ii] = 0;
    for (int i = 0; i < N; i++)
        counts[labels[i]] += weights[i];
    dtype32* Cptr = centers.dataPtr();

    AttemptedComparisons_w attempted_comparisons;

    int num_iterations = 0;
    int max_iterations = 1000;
    while ((true) && (num_iterations < max_iterations)) {
        num_iterations++;
        if (verbose)
            printf("isosplit2_w: iteration %d\n", num_iterations);
        int k1, k2;
        Isosplit2_w::find_next_comparison_w(M, K_init, k1, k2, active_labels.constData(), Cptr, counts, attempted_comparisons, repeat_tolerance);
        if (k1 < 0)
            break;
        if (verbose)
            printf("compare %d(%g),%d(%g) --- ", k1, counts[k1], k2, counts[k2]);

        const QList<long> inds1 = find_inds(labels, k1);
        const QList<long> inds2 = find_inds(labels, k2);
        QList<long> inds12 = inds1 + inds2;
        for (int m = 0; m < M; m++) {
            attempted_comparisons.centers1 << Cptr[m + k1 * M];
            attempted_comparisons.centers2 << Cptr[m + k2 * M];
        }
        attempted_comparisons.counts1 << Isosplit2_w::sum_of_weights_at_inds(weights, inds1);
        attempted_comparisons.counts2 << Isosplit2_w::sum_of_weights_at_inds(weights, inds2);
        for (int m = 0; m < M; m++) {
            attempted_comparisons.centers2 << Cptr[m + k1 * M];
            attempted_comparisons.centers1 << Cptr[m + k2 * M];
        }
        attempted_comparisons.counts2 << Isosplit2_w::sum_of_weights_at_inds(weights, inds1);
        attempted_comparisons.counts1 << Isosplit2_w::sum_of_weights_at_inds(weights, inds2);

        bool do_merge;

        QVector<int> labels0 = Isosplit2_w::test_redistribute_w(do_merge, X, weights, inds1, inds2, isocut_threshold);
        int max_label = *std::max_element(labels0.constBegin(), labels0.constEnd());
        if ((do_merge) || (max_label == 1)) {
            if (verbose)
                printf("merging size=%d.\n", inds12.count());
            for (int i = 0; i < N; i++) {
                if (labels[i] == k2)
                    labels[i] = k1;
            }
            QVector<double> ctr = Isosplit2_w::compute_center_w(X, inds12, weights);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k1);
            }
            counts[k1] = Isosplit2_w::sum_of_weights_at_inds(weights, inds12);
            counts[k2] = 0;
            active_labels[k2] = false;
        }
        else {

            QList<long> indsA0 = find_inds(labels0, 1);
            QList<long> indsB0 = find_inds(labels0, 2);
            QList<long> indsA, indsB;
            indsA.reserve(indsA0.count());
            indsB.reserve(indsB0.count());
            for (int i = 0; i < indsA0.count(); i++)
                indsA << inds12[indsA0[i]];
            for (int i = 0; i < indsB0.count(); i++)
                indsB << inds12[indsB0[i]];
            for (int i = 0; i < indsA.count(); i++) {
                labels[indsA[i]] = k1;
            }
            for (int i = 0; i < indsB.count(); i++) {
                labels[indsB[i]] = k2;
            }
            if (verbose)
                printf("redistributing sizes=(%d,%d).\n", indsA.count(), indsB.count());
            QVector<double> ctr = Isosplit2_w::compute_center_w(X, indsA, weights);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k1);
            }
            ctr = Isosplit2_w::compute_center_w(X, indsB, weights);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k2);
            }
            counts[k1] = Isosplit2_w::sum_of_weights_at_inds(weights, indsA);
            counts[k2] = Isosplit2_w::sum_of_weights_at_inds(weights, indsB);
        }
    }

    int labels_map[K_init];
    for (int k = 0; k < K_init; k++)
        labels_map[k] = 0;
    int kk = 1;
    for (int j = 0; j < K_init; j++) {
        if ((active_labels[j]) && (counts[j] > 0)) {
            labels_map[j] = kk;
            kk++;
        }
    }
    QVector<int> ret;
    ret.reserve(N);
    for (int i = 0; i < N; i++) {
        ret << labels_map[labels[i]];
    }
    return ret;
}
