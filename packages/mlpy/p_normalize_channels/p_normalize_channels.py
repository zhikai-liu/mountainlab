#!/usr/bin/python3

from mda.mdaio import readmda, writemda32, writemda64
import numpy as np
from p_normalize_channels.normalize_channels_cpp import nc_channel_variances, nc_scale_channels

class Processor:
	name='mlpy.normalize_channels'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[{"name":"output","description":"output .mda file"}]
	parameters=[]
	def run(self,args):
		### The processing
		input_path=args['input']
		output_path=args['output']
		X=readmda(input_path) #read
		print(X.ndim)
		variances=nc_channel_variances(X)
		nc_scale_channels(X,np.sqrt(variances));

		writemda32(X,output_path) #write
		return True
