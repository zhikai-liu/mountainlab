import os
import sys

parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from mlpy import ProcessorManager

import p_extract_geom
import p_extract_clips
import p_compute_templates
import p_extract_timeseries

PM=ProcessorManager()

PM.registerProcessor(p_extract_geom.extract_geom)
PM.registerProcessor(p_extract_clips.extract_clips)
PM.registerProcessor(p_compute_templates.compute_templates)
PM.registerProcessor(p_extract_timeseries.extract_timeseries)

if not PM.run(sys.argv):
    exit(-1)
