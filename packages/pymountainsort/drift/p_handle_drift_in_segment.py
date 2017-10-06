import numpy as np
import sys
import os
parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)
sys.path.append(parent_path+'/basic')

import mlpy
from p_compute_templates import compute_templates_helper

processor_name='pyms.handle_drift_in_segment'
processor_version='0.1'

def handle_drift_in_segment(*,timeseries,firings,firings_out):
    """
    Handle drift in segment. TODO: finish this brief description
    Parameters
    ----------
    timeseries : INPUT
        Path to preprocessed timeseries from which the events are extracted from (MxN)
    firings : INPUT
        Path of input firings mda file
    firings_out : OUTPUT
        Path of output drift-adjusted firings mda file

    """
    subcluster_size = 1000 # Size of subclusters for comparison of merge candidate pairs
    corr_comp_thresh = 0.5 # Minimum correlation in templates to consider as merge candidate
    clip_size=50
            
    # compute the templates
    templates=compute_templates_helper(timeseries=timeseries,firings=firings,clip_size=clip_size)        
        
    ### Determine the merge candidate pairs based on correlation
    subflat_templates=templates.reshape(templates.shape[0],templates.shape[1]*templates.shape[2])
    pairwise_idxs=list(it.chain.from_iterable(it.combinations(range(templates.shape[0]),2)))
    pairwise_idxs=pairwise_idxs.reshape(-1,2)
    pairwise_corrcoef=np.zeros(pairwise_idxs.shape[0])
    for row in range(pairwise_idxs.shape[0]):
        pairwise_corrcoef[row]=np.corrcoef(subflat_templates[pairwise_idxs[row,0],:],subflat_templates[pairwise_idxs[row,1],:])[1,0]
    pairs_for_eval=pairwise_idxs[pairwise_corrcoef>=corr_comp_thresh]
    for pair_to_test in range(pairs_for_eval.shape[0]):
        firings_subset=firings[:,np.isin(firings[2,:],pairs_for_eval[pair_to_test,:])]
        test_labels=firings_subset[2,:]
        test_eventtimes=firings_subset[1,:]
        FirstEventInPair_idx=np.flatnonzero(np.diff(test_labels))
        time_differences=test_eventtimes[FirstEventInPair_idx+1]-test_eventtimes[FirstEventInPair_idx]
        idx_for_eval=FirstEventInPair_idx[np.argsort(time_differences)[0:num_pairs]]
        idx_for_eval = np.unique(np.append(idx_for_eval,idx_for_eval+1))
        #event_labels_for_comp=labels[idx_for_eval]
        event_times_for_comp=np.searchsorted(timeseries_tmp,test_eventtimes[idx_for_eval])
        clips=np.zeros((len(event_times_for_comp),num_chan,half_clipsize*2))
        for idx, event in enumerate(event_times_for_comp):
            clips[idx,:,:]=timeseries[:,event-half_clipsize:event+half_clipsize]
        #TODO: TEST if the pairs should be linked / link or not
    #TODO: Iteration layer? recompute template distances after linked
    #TODO: create now drift linked firings
    #TODO: match across firings

def test_handle_drift_in_segment():
    test_clips=np.reshape(np.array([i for i in range(18)]),(2,3,3))

handle_drift_in_segment.test=test_handle_drift_in_segment
handle_drift_in_segment.name = processor_name
handle_drift_in_segment.version = processor_version
handle_drift_in_segment.author = 'J Chung and J Magland'
handle_drift_in_segment.last_modified = '...'

if __name__ == '__main__':
    print('Running test')
    test_handle_drift_in_segment()