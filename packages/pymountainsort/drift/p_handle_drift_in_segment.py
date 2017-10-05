import numpy as np
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import mlpy

processor_name='pyms.handle_drift_in_segment'
processor_version='0.1'

def handle_drift_in_segment(*,timeseries,firings,firings_out):
    """
    Link clusters split due to drift
    Parameters
    ----------
    timeseries : INPUT
        Timeseries from which the events are extracted from
    firings : INPUT
        Path of firing mda file to be evaluated for drift-related splitting
    firings_out : OUTPUT
        A path for the resulting drift-adjusted firings.mda

    """
    num_pairs = 1000
    #TODO: Convert to disk read/write, large file handling
    #TODO: Possible Geom layer
    timeseries_tmp=mlpy.readmda(timeseries)
    num_chan=timeseries_tmp.shape[0]
    for idx, firings in enumerate(firings_list):
        #calculate templates for all labels
        templates=np.zeros((np.amax(firings[2,:]),num_chan,clipsize))
        for label in labels:
            label_times=firings[1,np.where(firings[2,:]==label)]
            peak_indices=np.searchsorted(timeseries_tmp,label_times)
            clips=np.zeros((len(peak_indices),num_chan,clipsize))
            for idx, event in enumerate(peak_indices):
                clips[idx,:,:]=timeseries[:,event-clipsize:event+clipsize]
            template=np.mean(clips,axis=0)
            templates[label,:,:]=template
        #TODO: compare templates, and decide on which pairs for evaluation
        for pairs in closest_templates:
            firings_subset=firings[:,np.isin(firings[2,:],pair_to_test)
            FirstEventInPair_idx=np.flatnonzero(np.diff(labels))
            time_differences=eventtimes[FirstEventInPair_idx+1]-eventtimes[FirstEventInPair_idx]
            idx_for_eval=FirstEventInPair_idx[np.argsort(time_differences)[0:num_pairs]]
            idx_for_eval = np.unique(np.append(idx_for_eval,idx_for_eval+1))
            #event_labels_for_comp=labels[idx_for_eval]
            event_times_for_comp=np.searchsorted(timeseries_tmp,eventtimes[idx_for_eval])
            clips=np.zeros((len(event_times_for_comp),num_chan,clipsize))
            for idx, event in enumerate(event_times_for_comp):
                clips[idx,:,:]=timeseries[:,event-clipsize:event+clipsize]
            #TODO: TEST if the pairs should be linked / link or not
        #TODO: Iteration layer? recompute template distances after linked
        #TODO: create now drift linked firings
        #TODO: match across firings

def test_handle_drift_in_segment(args)
    test_clips=np.reshape(np.array([i for i in range(18)]),(2,3,3))

handle_drift_in_segment.test=test_handle_drift_in_segment
handle_drift_in_segment.name = processor_name
handle_drift_in_segment.version = processor_version
handle_drift_in_segment.author = 'J Chung and J Magland'
handle_drift_in_segment.last_modified = '...'
"""
PM=mlpy.ProcessorManager()
PM.registerProcessor(handle_drift_in_segments)
if not PM.run(sys.argv):
    exit(-1)
"""

if __name__ == '__main__':
    print('Running test')
    test_handle_drift_in_segment()