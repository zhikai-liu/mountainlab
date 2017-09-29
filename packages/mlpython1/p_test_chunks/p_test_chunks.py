#!/usr/bin/python3

import sys
import numpy as np
import os, inspect

# append the parent path to search directory
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir) 

# imports from mlpy
from mlpy import ProcessorManager
from mlpy import DiskReadMda

from timeserieschunkreader import TimeseriesChunkReader

class p_test_chunks:
	name='mlpy.test_chunks'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[]
	parameters=[]

	_chunk_size=100000
	_overlap_size=0
	_reader=TimeseriesChunkReader(_chunk_size, _overlap_size)
	_sum=0
	_num_chunks=0
	def run(self,args):
		input_path=args['input']
		self._sum=0
		if not self._reader.run(input_path, self._kernel):
			return False
		print("sum={}".format(self._sum))
		M=DiskReadMda(input_path).N1()
		N=DiskReadMda(input_path).N2()
		print("M={}, N={}, num.chunks={}".format(M,N,self._num_chunks))
		print("sum2={}".format(np.sum(DiskReadMda(input_path).readChunk(i1=0,i2=0,N1=M,N2=N).ravel())))
		return True

	def _kernel(self, X, info):
		self._num_chunks=self._num_chunks+1
		M=X.shape[0]
		N=X.shape[1]
		for m in range(0,M):
			row=X[m,info.i1:info.i2+1]
			self._sum=self._sum+np.sum(row)
		return True

PM=ProcessorManager()
PM.registerProcessor(p_test_chunks())
if not PM.run(sys.argv):
	exit(-1)