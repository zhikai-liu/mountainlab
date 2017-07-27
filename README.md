# MountainLab Spike Sorting Software

Notice: We recently moved everything in the master branch into the old/ directory. You should switch to one of the working branches (for example 'git checkout ms3' or 'git checkout 2017_06'). See installation instructions. The 2017_06 branch is a snapshot of the June 30th version. The ms3 branch is under development. If you can, use ms3, or think about switching in the relatively near future.

## About

MountainSort (a component of MountainLab) is spike sorting software developed by Jeremy Magland, Alex Barnett, and Leslie Greengard at the Center for Computational Biology, Flatiron Institute in close collaboration with Jason Chung and Loren Frank at UCSF department of Physiology. It is part of MountainLab, a general framework for data analysis and visualization.

MountainLab software is being developed by Jeremy Magland and Witold Wysota.

The software comprises tools for processing electrophysiological recordings and for visualizing and validating the results.

Contact the authors for information on the slack team for users and developers.

## Working branches

* [2017_06 branch](https://github.com/magland/mountainlab/tree/2017_06) - snapshot with only critical bug fix updates

* [ms3](https://github.com/magland/mountainlab/tree/ms3) - development branch with the ms3 processing pipeline

## Installation

[Installation instructions](old/doc/installation.md)

## How to run spike sorting

[The first sort](old/doc/the_first_sort.md)

## Some demo videos

* [Demo of additional WIP GUI for viewing very large datasets -- spikeview](https://www.youtube.com/watch?v=z1V1di8sQOI)

## Data formats used in MountainLab

[The .mda file format](old/doc/mda_format.md)

## Data management

[The .prv data management system](old/doc/prv_system.md)

## Automated curation

[Cluster metrics and automated curation](old/doc/metrics_automated_curation.md)

## About the master branch ##

The (default) master branch will ultimately not contain any code, but will rather be a table of contents to the various software versions (branches). This will allow you to stay on your preferred version until you are ready to switch to the newer stable version.

## Acknowledgements

Thanks to all the users on the slack team for ongoing testing and feedback.

## References

[Barnett, Alex H., Jeremy F. Magland, and Leslie F. Greengard. "Validation of Neural Spike Sorting Algorithms without Ground-truth Information." Journal of Neuroscience Methods 264 (2016): 65-77.](http://www.ncbi.nlm.nih.gov/pubmed/26930629) [Link to arXiv](http://arxiv.org/abs/1508.06936)

Magland, Jeremy F., and Alex H. Barnett. Unimodal clustering using isotonic regression: ISO-SPLIT. [Link to arXiv](http://arxiv.org/abs/1508.04841)

