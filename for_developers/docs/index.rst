MountainLab Documentation
=========================

MountainLab is a scientific data processing, sharing and visualization environment for scientists. It is built around the spike sorting software, MountainSort, but is designed to be widely applicable for many fields in computational data analysis.

MountainLab is built in layers in order to maintain flexibility and simplicity. The bottom layer allows users to run individual processing commands from a Linux terminal. The top layers allow cloud-based data processing and sharing of analysis pipelines and results through web browser interfaces.

The software comprises the following components:

* :doc:`processor_system` (mproc)

  - Install packages of processors on local workstation
  - Issue processing commands via linux terminal (mp-exec-process, mp-run-process, mp-queue-process)
  - Create custom plugin processors in any lanuage (C++, python, matlab/octave, etc.)

..

* Prv system (prv)

  - Represent large data files using tiny .prv text files that serve as universal handles
  - Enables separation of huge data files from analysis workspace
  - Facilitates sharing of processing results using universal file pointers

..

* Pipelines (mlp)

  - Create and execute processing pipelines

..

* MountainView

  - Desktop visualization of processing results
  - (Currently specific to spike sorting)

..

* Web framework
 
  - Create, exececute and share analysis pipelines via web browser (mlpipeline)
  - Register custom processing servers (cordion, larinet, kulele)

..

.. toctree::
   :maxdepth: 2

   overview

.. :ref:`overview_of_mountainlab`


.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`

