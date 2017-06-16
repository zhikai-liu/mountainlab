#include "detect_events.h"

QVector<double> detect_events(const QVector<double>& X, double detect_threshold, double detect_interval, int sign)
{
    double threshold2 = detect_threshold;

    bigint N = X.count();
    QVector<bigint> to_use(N);
    to_use.fill(0);
    bigint last_best_ind = 0;
    double last_best_val = 0;
    for (bigint n = 0; n < N; n++) {
        double val = (X[n]);
        if (sign < 0)
            val = -val;
        else if (sign == 0)
            val = fabs(val);
        if (n - last_best_ind > detect_interval)
            last_best_val = 0;
        if (val >= threshold2) {
            if (last_best_val > 0) {
                if (val > last_best_val) {
                    to_use[n] = 1;
                    to_use[last_best_ind] = 0;
                    last_best_ind = n;
                    last_best_val = val;
                }
            }
            else {
                if (val > 0) {
                    to_use[n] = 1;
                    last_best_ind = n;
                    last_best_val = val;
                }
            }
        }
    }
    QVector<double> times;
    for (bigint n = 0; n < N; n++) {
        if (to_use[n]) {
            times << n;
        }
    }
    return times;
}

