import sys
import numpy as np
import os

parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from mlpy import ProcessorManager,writemda64,writemda32,readmda,DiskReadMda
from common import TimeseriesChunkReader

def extract_clips_0(chunk,times,clip_size):
    M=chunk.shape[0]
    N=chunk.shape[1]
    T=clip_size
    L=len(times)
    clips=np.zeros((M,T,L))
    t1=-np.floor((T+1)/2)
    for t in range(T):
        inds=(times+t+t1)
        aa=np.where((clip_size<=inds)&(inds<N-clip_size))[0]
        clips[:,t,aa]=chunk[:,inds[aa].astype(int).tolist()]
    return clips

def extract_clips(*,timeseries,firings,clips_out,clip_size=100):
    """
    Extract clips corresponding to spike events

    Parameters
    ----------
    timeseries : INPUT
        Path of timeseries mda file (MxN) from which to draw the event clips (snippets)
    firings : INPUT
        Path of firings mda file (RxL) where R>=2 and L is the number of events. Second row are timestamps.
        
    clips_out : OUTPUT
        Path of clips mda file (MxTxL). T=clip_size
        
    clip_size : int
        (Optional) clip size, aka snippet size, aka number of timepoints in a single clip
    """    
    X=DiskReadMda(timeseries)
    M,N = X.N1(),X.N2()
    F=readmda(firings)
    times=F[1,:]
    L=F.shape[1]
    T=clip_size
    clips=np.zeros((M,T,L))
    def _kernel(chunk,info):
        inds=np.where((info.t1<=times)&(times<=info.t2))[0]
        times0=times[inds]-info.t1+info.i1
        clips0=extract_clips_0(chunk,times0,clip_size)
        clips[:,:,inds]=clips0
        return True
    TCR=TimeseriesChunkReader(chunk_size_mb=10, overlap_size=clip_size*2)
    if not TCR.run(timeseries,_kernel):
        return False
    writemda32(clips,clips_out)
    return True
extract_clips.name='mlpython1.extract_clips'
extract_clips.version="0.1"
def test_extract_clips(args):
    M,T,L,N = 5,100,100,1000
    X=np.random.rand(M,N).astype(np.float32)
    writemda32(X,'tmp.mda')
    F=np.zeros((2,L))
    F[1,:]=200+np.random.randint(N-400,size=(1,L))
    writemda64(F,'tmp2.mda')
    ret=extract_clips(timeseries='tmp.mda',firings='tmp2.mda',clips_out='tmp3.mda',clip_size=T)
    assert(ret)
    clips0=readmda('tmp3.mda')
    assert(clips0.shape==(M,T,L))
    t0=int(F[1,10])
    a=int(np.floor((T+1)/2))
    np.array_equal(clips0[:,:,10],X[:,t0-a:t0-a+T])
    #np.testing.assert_almost_equal(clips0[:,:,10],X[:,t0-a:t0-a+T],decimal=4)
    return True
extract_clips.test=test_extract_clips

def compute_templates(*,timeseries,firings,templates_out,clip_size=100):
    """
    Compute templates (average waveforms) for clusters defined by the labeled events in firings.

    Parameters
    ----------
    timeseries : INPUT
        Path of timeseries mda file (MxN) from which to draw the event clips (snippets) for computing the templates. M is number of channels, N is number of timepoints.
    firings : INPUT
        Path of firings mda file (RxL) where R>=3 and L is the number of events. Second row are timestamps, third row are integer labels.    
        
    templates_out : OUTPUT
        Path of output mda file (MxTxK). T=clip_size, K=maximum cluster label. Note that empty clusters will correspond to a template of all zeros. 
        
    clip_size : int
        (Optional) clip size, aka snippet size, number of timepoints in a single template
    """    
    X=DiskReadMda(timeseries)
    M,N = X.N1(),X.N2()
    N=N
    F=readmda(firings)
    L=F.shape[1]
    L=L
    T=clip_size
    times=F[1,:]
    labels=F[2,:].astype(int)
    K=np.max(labels)
    sums=np.zeros((M,T,K))
    counts=np.zeros(K)
    def _kernel(chunk,info):
        inds=np.where((info.t1<=times)&(times<=info.t2))[0]
        times0=times[inds]-info.t1+info.i1
        labels0=labels[inds]
        for k in range(1,K+1):
            inds_kk=np.where(labels0==k)
            clips0=extract_clips_0(chunk,times0[inds_kk],clip_size)
            sums[:,:,k-1]=sums[:,:,k-1]+np.sum(clips0,axis=2)
            counts[k-1]=counts[k-1]+len(inds_kk)
        return True
    TCR=TimeseriesChunkReader(chunk_size_mb=10, overlap_size=clip_size*2)
    if not TCR.run(timeseries,_kernel):
        return False
    templates=np.zeros((M,T,K))
    for k in range(1,K+1):
        if counts[k-1]:
            templates[:,:,k-1]=sums[:,:,k-1]/counts[k-1]
    writemda32(templates,templates_out)
    return True
compute_templates.name='mlpython1.compute_templates'
compute_templates.version="0.1"
def test_compute_templates(args):
    M,N,K,T,L = 5,1000,6,50,100
    X=np.random.rand(M,N)
    writemda32(X,'tmp.mda')
    F=np.zeros((3,L))
    F[1,:]=1+np.random.randint(N,size=(1,L))
    F[2,:]=1+np.random.randint(K,size=(1,L))
    writemda64(F,'tmp2.mda')
    ret=compute_templates(timeseries='tmp.mda',firings='tmp2.mda',templates_out='tmp3.mda',clip_size=T)
    assert(ret)
    templates0=readmda('tmp3.mda')
    assert(templates0.shape==(M,T,K))
    return True
compute_templates.test=test_compute_templates

if len(sys.argv)==1:
    sys.argv.append('test')
    #sys.argv.append('mlpython1.extract_clips')
    sys.argv.append('mlpython1.compute_templates')

PM=ProcessorManager()
PM.registerProcessor(extract_clips)
PM.registerProcessor(compute_templates)
if not PM.run(sys.argv):
    exit(-1)
