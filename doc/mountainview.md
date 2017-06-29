### MountainView GUI

After completing a sorting, you can visualize the sorting results using MountainView, the GUI.

### Launching MountainView

There are two ways to launch the GUI. The easiest and recommended way is to use kron:

> kron-view results pipeline dataset

Also, if you want to change or add a curation script, you can add that flag:

> kron-view results pipeline dataset --curation=path/to/curation.script

Kron uses the information used in the run so you do not have to specify all of the files that mountainview needs. To directly call mountainview, you can call the executable directly but need to specify all relevant files: 

> mountainview --raw=path/to/raw.mda --filt=path/to/filt.mda --pre=path/to/pre.mda --geom=path/to/geom.csv --cluster_metrics=path/to/cluster_metrics.json --firings=path/to/firings.mda  --samplerate=30000 --curation=path/to/curation.script

### Saving and resuming a MountainView session

To save a session, on the left-hand side of the GUI, under "Export/Upload/Download," select "export .mv2 document" This will save the session to the file.
To resume this session, simply relaunch mountainview using:

>mountainview mysavedfile.mv2

### Annotations in the GUI

When using a curation script, clusters will already be tagged with different labels. Clusters can also be tagged manually in the GUI by right-clicking on the cluster and adding a tag or clearing all tags. The most important tags are "rejected," "accepted," and "merge." The curation script will automatically tag clusters as rejected using the thresholds found in the script.
After examining clusters and determining that they should be merged, you can right-click on two or more clusters and merge them.
All annotations, including merge pairs, can be saved into a .mv2 file. This can be done under the "export/upload/download" tab on the left.

The mountainview GUI consists of two main panes on the right, and options and log display on the left. The panes can be filled with various types of views, and those views can also be popped out into their own window. By default, the cluster detail view and cluster metrics view should be launched.

On the left-hand side, the "open views" tab provides buttons to launch each one of the possible views.

The "timeseries" tab allows you to select raw, filtered, or filtered+whitened data for the displayed views. If the intermediate files such as the filtered data or preprocessed (filtered and whitened) data were deleted or cannot be found in the temporary directory, you will be asked if you want to regenerate those files at this time.

The "Cluster Visibility" tab allows you to hide subsets of clusters, based on what they are tagged with.

The "export/Upload/Download" tab allows for saving the session including all annotations using the "export .mv2 document." It also allows for you to export all firings, or only the curated firings. Curated firings only includes clusters tagged as "accepted," while the "export firings" button will save all clusters. All firings files are in [mda format](mda_format.md).

You can also open the prv manager under the "export/upload/download" tab, using the "Open PRV manager" button. This launches a window which will allow you to see which files (firings, raw, filtered, filtered+whitened) are on the local computer, and also the configured servers. Read more about the prv system [here](prv_system.md).

The "preferences" tab has many options which influence the views. After changing these values, you must click "recalculate" for the view to update. This can be done either in the top left with "recalc visible" and "recalc all," or with the calculator button above each view.

## Views

**Cluster Detail**
This view will show the mean waveform on channels for each cluster. In the top right, you can click on the 'gear' options icon and toggle standard deviation shading. You can scale the y-axis by using the arrow buttons at the top left. Note that the waveforms use the selected timeseries (such as raw, filtered, or whitened+filtered).

**Cluster Metrics**
This table will list all cluster metrics, and allow you to sort by any cluster metric

**Clips**
This will load all clips from the given cluster, for the selected timeseries. This may take some time to launch since the clips are not saved in a particular file - they are formed using the firing times and the full timeseries data.

**Curation Program**
If you used a curation.script, this will load here. You can edit the script in the GUI and click 'apply' to see how different metric thresholds will change the automated curation.

**Templates**
This loads the average waveform in the geometric configuration specified by the geom.csv. This is particularly useful with larger arrays.

**All Auto-correlograms**
This will load autocorrelograms for all non-hidden clusters using the options in 'preferences' at the bottom of the lefthand options pane. There, you can change the auto correlogram timescale "CC max. dt (msec)" and also the bin size "CC bin size (msec)."

**Selected Auto-correlograms**
Just as above, but only for the selected cluster(s).

**Cross-correlograms**
When selecting only one cluster, this will bring up cross-correlograms with all other non-hidden clusters. This uses the same options as for auto-correlograms, found under 'preferences' on the left-hand side.

**Matrix of Cross-correlograms**
When selecting a subset of the clusters, this will generate a cross-correlogram for every combination of the selected clusters, with the clusters' auto-correlogram on the diagonal. As above, this is subject to the options found under 'preferences' on the left-hand side.

**Timeseries Data**
This loads the timeseries as selected under "timeseries" on the left-hand side. The events are labeled with cluster id. You can zoom with the scroll-wheel, and also choose where to zoom into by clicking on the timeseries. This view synchronizes with the "Firing Events" view.

**PCA Features**
This loads a 3-dimensional view based upon the first 3 principle components for the selected clusters, using the selected timeseries. You can rotate with left-click, pan with right-click, and zoom with the scroll-wheel. Note that there are check-boxes at the bottom for different view types, including "density-plot," "label colors," "time colors," and "amplitude colors." In the "label colors" plot, you can click on the number to hide the cluster. Color selection can be changed using the "<-col" and "col->" buttons at the top left of the pane.

**Peak Amplitude Features**
Similar to PCA features, this loads a 3-dimensional view. However, this view is based upon the event peak amplitude for the three selected channels.

**Spike Spray**
This loads overlayed waveform traces from individual events in the selected clusters. The number of waveforms can be changed, alongside the visual properties of the individual traces. Note that this uses the selected timeseries (raw, filtered, or filtered+preprocessed). Also, the shown channels can be changed under "visible channels" found under "preferences" on the left-hand side.

**Firing Events**
This has time on the x-axis and amplitude on the y-axis for the selected cluster(s). Note that this synchronizes with "Timeseries Data" view, allowing you to view the raw timeseries data for the individual events.

**Amplitude Histograms**
This generates an amplitude histogram for all non-hidden clusters, ordered from lowest amplitude to highest amplitude. This allows for quick assessment if any of the clusters are being truncated by the detection threshold. The detection threshold is shown as a vertical dotted line, and can be changed using "Amp. thresh. (display)" under "preferences." Note that changing this value only changes the location of this line, and not which events are actually included in the sorting.

**Discrimination Histograms**
This view is meant to allow quick assessment of how well clusters are isolated from one another. The histogram is of the one-dimensional projection onto the line between the two centroids of the indicated clusters. Note that the projection is done from the PCA space, which depends upon the selected timeseries (raw, filtered, filtered+whitened). For the selected clusters, all possible pairs are shown.

**Close all**
This closes all open views.
