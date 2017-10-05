import numpy as np

import sys,os
parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from mlpy import readmda,writemda32

processor_name='pyms.synthesize_timeseries'
processor_version='0.1'
def synthesize_timeseries(*,firings='',waveforms='',timeseries_out,noise_level=1,num_timepoints=0,waveform_upsamplefac=1):
    """
    Synthesize a electrophysiology timeseries from a set of ground-truth firing events and waveforms

    Parameters
    ----------
    firings : INPUT
        (Optional) The path of firing events file in .mda format. RxL where R>=3 and L is the number of events. Second row is the timestamps, third row is the integer labels/
    waveforms : INPUT
        (Optional) The path of (possibly upsampled) waveforms file in .mda format. Mx(T*waveform_upsample_factor)*K, where M is the number of channels, T is the clip size, and K is the number of units.
    
    timeseries_out : OUTPUT
        The output path for the new timeseries. MxN, where N=num_timepoints (if num_timepoints>0)

    noise_level : double
        (Optional) Standard deviation of the simulated background noise added to the timeseries
    num_timepoints : int
        (Optional) Number of timepoints (N) in the dataset. If set to zero, will use N=max(times)+clip_size where times are the event times from the firings file and clip_size is the size of the second dimension of the waveforms
    waveform_upsamplefac : int
        (Optional) The upsampling factor corresponding to the input waveforms. (avoids digitization artifacts)
    """
    noise_level=np.float64(noise_level)
    num_timepoints=np.int64(num_timepoints)
    waveform_upsamplefac=int(waveform_upsamplefac)
    
    if waveforms:
        W=readmda(waveforms)
    else:
        W=np.zeros((1,100,1))
    
    if firings:
        F=readmda(firings)
    else:
        F=np.zeros((3,0))
        
    times=F[1,:]
    
    M,TT,K = W.shape[0],W.shape[1],W.shape[2]
    T=TT/waveform_upsample_factor
    
    N=num_timepoints
    if (N==0):
        if times.size==0:
            N=T
        else:
            N=max(times)+T
            
    X=np.random.randn(M,N)*noise_level
    
        
    
    return writemda32(X,timeseries_out)

def test_synthesize_timeseries():
    N=100000
    ret=synthesize_timeseries(timeseries_out='tmp.mda',num_timepoints=N)
    assert(ret)
    A=readmda('tmp.mda')
    assert(A.shape==(1,N))
    return True

synthesize_timeseries.test=test_synthesize_timeseries
synthesize_timeseries.name = processor_name
synthesize_timeseries.version = processor_version

if __name__ == '__main__':
    print ('Running test')
    test_synthesize_timeseries()