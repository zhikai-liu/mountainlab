#include "detect_events.h"

QVector<double> detect_events(const QVector<double>& X, double detect_threshold, double detect_interval, int sign)
{
    //double mean = MLCompute::mean(X);
    //double stdev = MLCompute::stdev(X);
    //double threshold2 = detect_threshold * stdev;
    double threshold2=detect_threshold;

    QVector<bigint> timepoints_to_consider;
    QVector<double> vals_to_consider;
    for (bigint n=0; n<X.count(); n++) {
        double val=X[n];
        if (sign < 0)
            val = -val;
        else if (sign == 0)
            val = fabs(val);
        if (val>threshold2) {
            timepoints_to_consider << n;
            vals_to_consider << val;
        }
    }
    QVector<bool> to_use(timepoints_to_consider.count(),false);
    bigint last_best_ind=-1;
    for (bigint i=0; i<timepoints_to_consider.count(); i++) {
        if (last_best_ind>=0) {
            if (timepoints_to_consider[last_best_ind]<timepoints_to_consider[i]-detect_interval) {
                last_best_ind=-1;
                //last best ind is not within range. so update it
                if ((i>0)&&(timepoints_to_consider[i-1]>=timepoints_to_consider[i]-detect_interval)) {
                    last_best_ind=i-1;
                    bigint jj=last_best_ind;
                    while ((jj-1>=0)&&(timepoints_to_consider[jj-1]>=timepoints_to_consider[i]-detect_interval)) {
                        if (vals_to_consider[jj-1]>vals_to_consider[last_best_ind]) {
                            last_best_ind=jj-1;
                        }
                        jj--;
                    }
                }
                else {
                    last_best_ind=-1;
                }
            }
        }
        if (last_best_ind>=0) {
            if (vals_to_consider[i]>vals_to_consider[last_best_ind]) {
                to_use[i]=true;
                to_use[last_best_ind]=false;
                last_best_ind=i;
            }
            else {
                to_use[i]=false;
            }
        }
        else {
            to_use[i]=true;
            last_best_ind=i;
        }
    }

    QVector<double> times;
    for (bigint i=0; i<timepoints_to_consider.count(); i++) {
        if (to_use[i]) {
            times << timepoints_to_consider[i];
        }
    }
    return times;
}

