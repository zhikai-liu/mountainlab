#include "isocut_w.h"
#include <math.h>
#include "jisotonic.h"
#include <stdio.h>
#include <stdlib.h>
#include "isocut.h"
#include <algorithm>

namespace Isocut_w {

void sort(int N, double* out, const double* in)
{
    std::copy(in, in + N, out);
    std::sort(out, out + N);
}

double compute_ks_w(int N1, int N2, double* samples1, double* samples2, const double* weights1, const double* weights2)
{
    double sum_weights1 = 0;
    for (int i = 0; i < N1; i++)
        sum_weights1 += weights1[i];
    double sum_weights2 = 0;
    for (int i = 0; i < N2; i++)
        sum_weights2 += weights2[i];
    double max_dist = 0;
    double count1 = 0;
    double count2 = 0;
    int i1 = 0;
    for (int i2 = 0; i2 < N2; i2++) {
        while ((i1 + 1 < N1) && (samples1[i1 + 1] <= samples2[i2])) {
            count1 += weights1[i1];
            i1++;
        }
        const double dist = fabs(count2 / sum_weights2 - count1 / sum_weights1);
        max_dist = std::max(max_dist, dist);
        count2 += weights2[i2];
    }
    return max_dist;
}
}

std::vector<size_t> sort_indices(int N, const double* X)
{

    std::vector<double> v(N);
    for (long i = 0; i < N; i++)
        v[i] = X[i];

    // initialize original index locations
    std::vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

    return idx;
}

bool isocut_w(int N, double* cutpoint, const double* samples_in, const double* weights_in, double threshold, int minsize)
{
    *cutpoint = 0;
    std::vector<size_t> indices = sort_indices(N, samples_in);
    double* samples = (double*)malloc(sizeof(double) * N);
    double* weights = (double*)malloc(sizeof(double) * N);
    for (long i = 0; i < N; i++) {
        samples[i] = samples_in[indices[i]];
        weights[i] = weights_in[indices[i]];
    }

    int* N0s = (int*)malloc(sizeof(int) * (N * 2 + 2)); //conservative malloc
    int num_N0s = 0;
    for (int ii = 2; ii <= floor(log2(N / 2 * 1.0)); ii++) {
        N0s[num_N0s] = pow(2, ii);
        num_N0s++;
        N0s[num_N0s] = -pow(2, ii);
        num_N0s++;
    }
    N0s[num_N0s] = N;
    num_N0s++;

    bool found = false;
    for (int jj = 0; (jj < num_N0s) && (!found); jj++) {
        int N0 = N0s[jj];
        int NN0 = N0;
        if (N0 < 0)
            NN0 = -N0;
        double* samples0 = (double*)malloc(sizeof(double) * NN0);
        double* spacings0 = (double*)malloc(sizeof(double) * (NN0 - 1));
        double* spacings0_fit = (double*)malloc(sizeof(double) * (NN0 - 1));
        double* samples0_fit = (double*)malloc(sizeof(double) * NN0);
        double* spacings0_weights = (double*)malloc(sizeof(double) * (NN0 - 1));

        if (N0 > 0) {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[ii] = samples[ii];
            }
        }
        else {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[NN0 - 1 - ii] = samples[N - 1 - ii];
            }
        }
        for (int ii = 0; ii < NN0 - 1; ii++) {
            double w0 = (weights[ii] + weights[ii + 1]) / 2;
            spacings0_weights[ii] = w0;
        }
        double sum_of_weights = 0;
        for (int ii = 0; ii < NN0; ii++) {
            sum_of_weights += weights[ii];
        }
        for (int ii = 0; ii < NN0 - 1; ii++) {
            spacings0[ii] = (samples0[ii + 1] - samples0[ii]) / spacings0_weights[ii];
        }
        jisotonic_downup(NN0 - 1, spacings0_fit, spacings0, weights);
        samples0_fit[0] = samples0[0];
        for (int ii = 1; ii < NN0; ii++) {
            samples0_fit[ii] = samples0_fit[ii - 1] + spacings0_fit[ii - 1] * spacings0_weights[ii - 1];
        }
        double ks0 = Isocut_w::compute_ks_w(NN0, NN0, samples0, samples0_fit, weights, weights);
        ks0 *= sqrt(NN0 * 1.0); //do we use NN0 or sum(weights) here?
        //ks0 *= sqrt(sum_of_weights * 1.0);
        if (ks0 >= threshold) {
            for (int ii = 0; ii < NN0 - 1; ii++) {
                if (spacings0_fit[ii])
                    spacings0[ii] = spacings0[ii] / spacings0_fit[ii];
            }
            jisotonic_updown(NN0 - 1, spacings0_fit, spacings0, 0);
            if (NN0 >= minsize * 2) {
                int max_ind = minsize - 1;
                double maxval = 0;
                for (int ii = minsize - 1; ii <= NN0 - 1 - minsize; ii++) {
                    if (spacings0_fit[ii] > maxval) {
                        maxval = spacings0_fit[ii];
                        max_ind = ii;
                    }
                }
                *cutpoint = (samples0[max_ind] + samples0[max_ind + 1]) / 2;
                found = true;
            }
        }

        free(samples0);
        free(spacings0);
        free(spacings0_fit);
        free(samples0_fit);
        free(spacings0_weights);
    }

    free(N0s);
    free(samples);

    return found;
}

/*
bool isocut_w_new(int N, double* cutpoint, const double* samples_in, const double* weights, double threshold, int minsize)
{
    //return isocut(N,cutpoint,samples_in,threshold,minsize);
    *cutpoint = 0;
    double* samples = (double*)malloc(sizeof(double) * N);
    Isocut_w::sort(N, samples, samples_in);

    int* N0s = (int*)malloc(sizeof(int) * (N * 2 + 2)); //conservative malloc
    int num_N0s = 0;
    for (int ii = 2; ii <= floor(log2(N / 2 * 1.0)); ii++) {
        N0s[num_N0s] = pow(2, ii);
        num_N0s++;
        N0s[num_N0s] = -pow(2, ii);
        num_N0s++;
    }
    N0s[num_N0s] = N;
    num_N0s++;

    bool found = false;
    for (int jj = 0; (jj < num_N0s) && (!found); jj++) {
        int N0 = N0s[jj];
        int NN0 = N0;
        if (N0 < 0)
            NN0 = -N0;
        double* samples0 = (double*)malloc(sizeof(double) * NN0);
        double* weighted_spacings0 = (double*)malloc(sizeof(double) * NN0);
        double* weighted_spacings0_fit = (double*)malloc(sizeof(double) * NN0);
        double* samples0_fit = (double*)malloc(sizeof(double) * NN0);

        if (N0 > 0) {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[ii] = samples[ii];
            }
        }
        else {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[NN0 - 1 - ii] = samples[N - 1 - ii];
            }
        }
        for (int ii = 0; ii < NN0; ii++) {
            double val=0;
            if (ii==0) {
                if (NN0>1)
                    val=samples0[1]-samples0[0];
            }
            else if (ii==NN0-1) {
                if (NN0>1)
                    val=samples0[NN0-1]-samples0[NN0-2];
            }
            else {
                val=(samples0[ii+1]-samples0[ii-1])/2;
            }
            weighted_spacings0[ii] = val/weights[ii];
        }
        jisotonic_downup(NN0, weighted_spacings0_fit, weighted_spacings0, weights);
        samples0_fit[0] = samples0[0];
        for (int ii = 1; ii < NN0; ii++) {
            samples0_fit[ii] = samples0_fit[ii - 1] + weighted_spacings0_fit[ii-1] * weights[ii-1];
        }
        double ks0 = Isocut_w::compute_ks(NN0, NN0, samples0, samples0_fit);
        ks0 *= sqrt(NN0 * 1.0); //should the weight come into play here? not sure, but don't think so
        //ks0 *= sqrt(sum_of_weights*1.0); //should this be sum_of_weights or NN0?
        if (ks0 >= threshold) {
            for (int ii = 0; ii < NN0; ii++) {
                if (weighted_spacings0_fit[ii])
                    weighted_spacings0[ii] = weighted_spacings0[ii] / weighted_spacings0_fit[ii];
            }
            jisotonic_updown(NN0, weighted_spacings0_fit, weighted_spacings0, weights);
            if (NN0 >= minsize * 2) {
                int max_ind = minsize - 1;
                double maxval = 0;
                for (int ii = minsize - 1; ii <= NN0 - 1 - minsize; ii++) {
                    if (weighted_spacings0_fit[ii] > maxval) {
                        maxval = weighted_spacings0_fit[ii];
                        max_ind = ii;
                    }
                }
                *cutpoint = (samples0[max_ind] + samples0[max_ind + 1]) / 2;
                found = true;
            }
        }

        free(samples0);
        free(weighted_spacings0);
        free(weighted_spacings0_fit);
        free(samples0_fit);
    }

    free(N0s);
    free(samples);

    return found;
}
*/

bool isocut_w(int N, double* cutpoint, double* X, double* weights, double threshold)
{
    return isocut_w(N, cutpoint, X, weights, threshold, 4);
}
