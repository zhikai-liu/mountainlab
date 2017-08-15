#!/usr/bin/env python

import h5py
import numpy as np
import os
import argparse

def pack_group(path,top_file,relpath):
	print('Packing:: '+path)
	if os.path.exists(path+'/data.nwb'):
		g=h5py.File(path+'/data.nwb','r')
		dd=dict(g)
		for key in dd:
			h5py.h5o.copy(g.id,key,top_file.id,relpath+'/'+key)
		g.close()
	files=os.listdir(path)
	for file in files:
		if os.path.isdir(path+'/'+file):
			top_file.create_group(relpath+'/'+file)
			pack_group(path+'/'+file,top_file,relpath+'/'+file)

	
def main():
	parser = argparse.ArgumentParser(usage='%(prog)s [-h] <src_directory> <nwb_filename>',description='Pack a directory into a .nwb file. See also unpack_nwb')
	parser.add_argument('src_directory', nargs='+', help='source directory')
	parser.add_argument('nwb_filename', nargs='+', help='destination .nwb file')
	
	args = parser.parse_args()

	src_directory = args.src_directory[0]
	nwb_filename = args.nwb_filename[0]

	f=h5py.File(nwb_filename,'w')
	pack_group(src_directory,f,'')
	f.close()

main()
