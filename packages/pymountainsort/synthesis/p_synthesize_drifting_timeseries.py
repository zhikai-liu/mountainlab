import numpy as np

import sys,os
parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from p_synthesize_timeseries import synthesize_timeseries

from mlpy import readmda,writemda32

processor_name='pyms.synthesize_drifting_timeseries'
processor_version='0.1'
def synthesize_drifting_timeseries(*,firings,waveforms,timeseries_out=None,noise_level=1,samplerate=30000,duration=60,waveform_upsamplefac=1,amplitudes_row=0):
    """
    Synthesize a electrophysiology timeseries from a set of ground-truth firing events and waveforms, and simulating drift (linear for now)

    Parameters
    ----------
    firings : INPUT
        (Optional) The path of firing events file in .mda format. RxL where 
        R>=3 and L is the number of events. Second row is the timestamps, 
        third row is the integer labels
    waveforms : INPUT
        (Optional) The path of (possibly upsampled) waveforms file in .mda
        format. Mx(T*waveform_upsample_factor)*(2K), where M is the number of
        channels, T is the clip size, and K is the number of units. Each unit
        has a contiguous pair of waveforms (interpolates from first to second
        across the timeseries)
    
    timeseries_out : OUTPUT
        The output path for the new timeseries. MxN

    noise_level : double
        (Optional) Standard deviation of the simulated background noise added to the timeseries
    samplerate : double
        (Optional) Sample rate for the synthetic dataset in Hz
    duration : double
        (Optional) Duration of the synthetic dataset in seconds. The number of timepoints will be duration*samplerate
    waveform_upsamplefac : int
        (Optional) The upsampling factor corresponding to the input waveforms. (avoids digitization artifacts)
    amplitudes_row : int
        (Optional) If positive, this is the row in the firings arrays where the amplitude scale factors are found. Otherwise, use all 1's
    """
    
    if type(firings)==str:
        F=readmda(firings)
    else:
        F=firings

    print(F.shape)        
    if amplitudes_row==0:
        F=np.concatenate((F,np.ones((1,F.shape[1]))))
        amplitudes_row=F.shape[0]
        
    times=F[1,:]
    times_normalized=times/(duration*samplerate) #normalized between 0 and 1
    labels=F[2,:]
    amps=F[amplitudes_row-1,:]
            
    F=np.kron(F,[1,1]) #duplicate every event!
    
    F[amplitudes_row-1,::2]=amps*times_normalized
    F[amplitudes_row-1,1::2]=amps*(1-times_normalized)
    # adjust the labels
    F[2,::2]=labels*2-1 #remember that labels are 1-indexed
    F[2,1::2]=labels*2
    print(F)
    return synthesize_timeseries(firings=F,waveforms=waveforms,timeseries_out=timeseries_out,noise_level=noise_level,samplerate=samplerate,duration=duration,waveform_upsamplefac=waveform_upsamplefac,amplitudes_row=amplitudes_row)

def test_synthesize_drifting_timeseries():
    waveform_upsamplefac=5
    M=4 #num channels
    T=800 #num timepoints per clip
    K=2 # num units
    from p_synthesize_random_waveforms import synthesize_random_waveforms
    #W=np.random.rand(M,T*waveform_upsamplefac,K*2)
    (W,geom)=synthesize_random_waveforms(M=M,T=T,K=K*2,upsamplefac=waveform_upsamplefac)
    times=np.arange(2500,7500,500)
    F=np.array([[0,0,0,0,0,0,0,0,0,0],times,[1,2,1,2,1,2,1,2,1,2]])
    print(F)
    print(F.shape)
    #return;
    X=synthesize_drifting_timeseries(waveforms=W,firings=F,duration=1,samplerate=10000,noise_level=0)
    import matplotlib.pyplot as pp
    pp.imshow(X,extent=[0, 1, 0, 1]);
    #pp.plot(X[0,:]); pp.show()
    writemda32(X,'tmp.mda');
    return True

synthesize_drifting_timeseries.test=test_synthesize_drifting_timeseries
synthesize_drifting_timeseries.name = processor_name
synthesize_drifting_timeseries.version = processor_version

if __name__ == '__main__':
    print ('Running test')
    test_synthesize_drifting_timeseries()