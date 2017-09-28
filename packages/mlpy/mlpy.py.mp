#!/usr/bin/python3

import sys
from mlprocessorlibrary.mlprocessorlibrary import MLProcessorLibrary

from p_extract_geom import Processor as p_extract_geom
from p_normalize_channels.p_normalize_channels import Processor as p_normalize_channels
from p_test_chunks import Processor as p_test_chunks

PL=MLProcessorLibrary()
PL.addProcessor(p_extract_geom())
PL.addProcessor(p_normalize_channels())
PL.addProcessor(p_test_chunks())
if not PL.run(sys.argv):
	exit(-1)

