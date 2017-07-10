# MountainLab

**See important notice below**

MountainSort is spike sorting software developed by Jeremy Magland, Alex Barnett, and Leslie Greengard at the Center for Computational Biology, Flatiron Institute in close collaboration with Jason Chung and Loren Frank at UCSF department of Physiology. It is part of MountainLab, a general framework for data analysis and visualization.

MountainLab software is being developed by Jeremy Magland and Witold Wysota.

The software comprises tools for processing electrophysiological recordings and for visualizing and validating the results.

Contact the authors for information on the slack team for users and developers.

## Important notice ##
I believe in continually simplifying the code base. That sometimes involves deleting a lot of files. Of course, when users start to depend on the software it is not right to pull the rug out from under them. It is challenging to decide which new version to incorporate into the (default) master branch.

We will therefore be migrating into the following system. The (default) master branch will ultimately not contain any code, but will rather be a table of contents to the various software versions (branches). This will allow you to stay on your preferred version until you are ready to switch to the newer stable version. In the short term, the master branch will still function. But I would recommend that you switch to the 2017_06 branch (below) which will be a snapshot (of master right now) with only critical bug fixes.

* [2017_06 branch](https://github.com/magland/mountainlab/tree/2017_06) - snapshot with only critical bug fix updates

* [ms3](https://github.com/magland/mountainlab/tree/ms3) - development branch with the ms3 processing pipeline


## Installation

[Installation instructions](doc/installation.md)

## How to run spike sorting

[The first sort](doc/the_first_sort.md)

## Data formats used in MountainLab

[The .mda file format](doc/mda_format.md)

## Data management

[The .prv data management system](doc/prv_system.md)

## Automated curation

[Cluster metrics and automated curation](doc/metrics_automated_curation.md)

## References

[Barnett, Alex H., Jeremy F. Magland, and Leslie F. Greengard. "Validation of Neural Spike Sorting Algorithms without Ground-truth Information." Journal of Neuroscience Methods 264 (2016): 65-77.](http://www.ncbi.nlm.nih.gov/pubmed/26930629) [Link to arXiv](http://arxiv.org/abs/1508.06936)

Magland, Jeremy F., and Alex H. Barnett. Unimodal clustering using isotonic regression: ISO-SPLIT. [Link to arXiv](http://arxiv.org/abs/1508.04841)

