#ifndef ISOCUT_W_H
#define ISOCUT_W_H

/*
 * MCWRAP [ cutpoint[1,1] ] = isocut_w (X[1,N], weights[1,N], threshold)
 * SET_INPUT N = size(X,2)
 * SOURCES isocut_w.cpp jisotonic.cpp
 * HEADERS isocut_w.h
 */
bool isocut_w(int N, double* cutpoint, double* X, double* weights, double threshold);
bool isocut_w(int N, double* cutpoint, const double* X, const double* weights, double threshold, int minsize);

#endif // ISOCUT_W_H
