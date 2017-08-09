#ifndef P_COMBINE_FIRING_SEGMENTS_H
#define P_COMBINE_FIRING_SEGMENTS_H

#include <QStringList>

struct P_combine_firing_segments_opts {
    int clip_size = 60;
    double template_correlation_threshold = 0.7;
    double match_score_threshold = 0.2;
    int offset_search_radius = 10;
    int match_tolerance = 3;
};

bool p_combine_firing_segments(QString timeseries, QStringList firings_list, QString firings_out, P_combine_firing_segments_opts opts);

/*
Each segment S is compared with the previous segment Sprev. For each cluster k2 in S, we find if there is a matching cluster k1 in Sprev. The criteria for a matching cluster is as follows:

First the templates (avg waveforms) needs to have large enough correlation. But this is computed as a sliding correlation -- optimal time offset -- to account for time alignment issues.

The amount of sliding is the offset_search_radius=10. The threshold is the template_correlation_threshold=0.7.

Once we have a candidate pair k1/k2, we compute the match_score which is a fraction between 0 and 1. This must be greater than or equal to the match_score_threshold=0.2 in order to consider as a further candiate.

The match score is computed as numer/denom. The numerator is the number of matching events in the overlapping time window. A matching event is a coincident time with a tolerance of match_tolerance=3 timepoints. Important: the optimal time offset from the templates is used here, so we search between optimal_offset-match_tolerance to optimal_offset+match_tolerance.

The denom is the num1+num2-numer where num1 is the number of events in cluster k1 (in segment Sprev) in the overlapping window, and num2 is the number of events in cluster k2 (in segment S).

If there is more than one match, we use the one with greatest match_score.

Then all of the matching events (redundant) in segment S are deleted, and the remaining ones are mapped to cluster k1 in segment Sprev. (edited)

In a second pass, we go through and map to a new set of labels, responding the matches between adjacent (overlapping) segments.

Finally, we do another fit_stage because there will be other redundant events in this procedure.
*/

#endif // P_COMBINE_FIRING_SEGMENTS_H
