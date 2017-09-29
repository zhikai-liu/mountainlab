#!/usr/bin/python3

import numpy as np

class Processor:
	name='mlpy.extract_geom'
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
