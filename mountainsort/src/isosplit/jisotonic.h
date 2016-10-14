/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef jisotonic_h
#define jisotonic_h

void jisotonic(int N, double* BB, double* MSE, const double* AA, const double* WW);
void jisotonic_updown(int N, double* out, const double* in, const double* weights);
void jisotonic_downup(int N, double* out, const double* in, const double* weights);

#endif
