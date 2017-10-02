import sys
import numpy as np
import os

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# imports from mlpy
from mlpy import ProcessorManager

def extract_geom(*,geom,channels_array='',geom_out,channels=''):
    """
    Extract a subset of channels from a geom csv file

    Parameters
    ----------
    geom : INPUT
        Path of geom csv file having M rows and R columns where M is number of channels and R is the number of physical dimensions (1, 2, or 3)
    channels_array : INPUT 
        (optional) Path of array of channel numbers (positive integers). Either use this or the channels parameter, not both.
        
    geom_out : OUTPUT
        Path of output geom csv file containing a subset of the channels
        
    channels : string
        (Optional) Comma-separated list of channels to extract. Either use this or the channels_array input, not both.
    """    
    if channels:
        Channels=np.fromstring(channels,dtype=int,sep=',')
    elif channels_array:
        Channels=channels_array
    else:
        Channels.np.empty(0)        
    
    X=np.loadtxt(open(geom, "rb"), delimiter=",")
    X=X[(Channels-1).tolist(),:]
    np.savetxt(geom_out,X,delimiter=",",fmt="%g")
    return True
            
extract_geom.name='mlpython1.extract_geom'
extract_geom.version="0.1"

PM=ProcessorManager()
PM.registerProcessor(extract_geom)
if not PM.run(sys.argv):
    exit(-1)
