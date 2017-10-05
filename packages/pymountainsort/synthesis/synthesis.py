import os
import sys

parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from mlpy import ProcessorManager

import p_synthesize_timeseries
import p_synthesize_random_waveforms
import p_synthesize_random_firings

PM=ProcessorManager()

PM.registerProcessor(p_synthesize_timeseries.synthesize_timeseries)
PM.registerProcessor(p_synthesize_random_waveforms.synthesize_random_waveforms)
PM.registerProcessor(p_synthesize_random_firings.synthesize_random_firings)

if not PM.run(sys.argv):
    exit(-1)
