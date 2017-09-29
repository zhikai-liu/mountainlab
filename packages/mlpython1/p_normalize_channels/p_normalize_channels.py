import sys,inspect,os

# append the parent path to search directory
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir) 

# imports from mlpy
from mlpy import readmda, writemda32
from mlpy import ProcessorManager

# numpy
import numpy as np

# import the C++ code
import cppimport
cpp=cppimport.imp('normalize_channels_cpp')

class p_normalize_channels:
	name='mlpy.normalize_channels'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[{"name":"output","description":"output .mda file"}]
	parameters=[]
	def run(self,args):
		input_path=args['input']
		output_path=args['output']
		X=readmda(input_path)
		print(X.ndim)
		variances=cpp.channel_variances(X)
		cpp.scale_channels(X,np.sqrt(variances));

		writemda32(X,output_path)
		return True

PM=ProcessorManager()
PM.registerProcessor(p_normalize_channels())
if not PM.run(sys.argv):
	exit(-1)
