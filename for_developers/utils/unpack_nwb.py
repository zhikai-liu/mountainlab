#!/usr/bin/env python

import h5py
import numpy as np
import os
import argparse

def unpack_group(item,path,top_file,relpath):
	print('Unpacking:: '+path)
	os.mkdir(path)
	keys=item.keys()
	g=h5py.File(path+'/data.nwb','w')
	for key in keys:
		try:
			item2=item[key]
			if isinstance(item2,h5py.Group):
				unpack_group(item2,path+'/'+key,top_file,relpath+'/'+key)
			else:
				h5py.h5o.copy(top_file.id,relpath+'/'+key,g.id,key)
		except Exception as err:
			print('Error unpacking '+relpath+'/'+key+': ',err)
	g.close()
	
def main():
	parser = argparse.ArgumentParser(usage='%(prog)s [-h] <nwb_filename> <dest_directory>',description='Unpack a .nwb file into a directory, expanding out the groups into subdirectories. See also pack_nwb')
	parser.add_argument('nwb_filename', nargs='+', help='.nwb file to be unpacked')
	parser.add_argument('dest_directory', nargs='+', help='destination directory')
	args = parser.parse_args()

	nwb_filename = args.nwb_filename[0]
	dest_directory = args.dest_directory[0]

	f=h5py.File(nwb_filename,'r')
	unpack_group(f,dest_directory,f,'')
	f.close()

main()