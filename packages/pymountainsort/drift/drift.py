import os
import sys

parent_path=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_path)

from mlpy import ProcessorManager

import p_concatenate_firings

PM=ProcessorManager()

PM.registerProcessor(p_concatenate_firings.concatenate_firings)

if not PM.run(sys.argv):
    exit(-1)
