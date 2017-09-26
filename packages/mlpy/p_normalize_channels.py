#!/usr/bin/python3

from mda.mdaio import readmda, writemda32, writemda64
import numpy as np

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
		M=X.shape[0]
		N=X.shape[1]
		for m in range(0,M):
			print('Normalizing channel ',m)
			row=X[m,:]
			stdev0=np.std(row)
			row=row*(1/stdev0)
			X[m,:]=row

		writemda32(X,output_path) #write
		return True
