import sys
import numpy as np
import os, inspect

# append the parent path to search directory
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir) 

# imports from mlpy
from mlpy import ProcessorManager

class p_extract_geom:
	name='mlpython1.extract_geom'
	description='Extract a subset of channels from a geom.csv file'
	version='0.1'
	inputs=[{"name":"geom","description":""}]
	outputs=[{"name":"geom_out","description":""}]
	parameters=[{"name":"channels","description":"","optional":False}]
	def run(self,args):
		geom_path=args['geom']
		geom_out_path=args['geom_out']
		channels_str=args['channels']
		channels=np.fromstring(channels_str,dtype=int,sep=',')
		X=np.loadtxt(open(geom_path, "rb"), delimiter=",")
		X=X[channels-1,:]
		np.savetxt(geom_out_path,X,delimiter=",",fmt="%g")
		return True

PM=ProcessorManager()
PM.registerProcessor(p_extract_geom())
if not PM.run(sys.argv):
	exit(-1)
