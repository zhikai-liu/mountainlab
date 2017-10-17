MountainSort uses the .mda format for the raw timeseries data (input), the intermediate arrays, and the output file (firings.mda) containing the sorting labels and timestamps.

### Motivation

The .mda file format was created as a simple method for storing multi-dimensional arrays of numbers. Of course the simplest way would be to store the array as a raw binary file, but the problem with this is that fundamental information required to read the data is missing – specifically,

1) the data type (e.g., float32, int16, byte, complex float, etc).
2) the number of dimensions (e.g., number of rows in a matrix)
3) the size of the dimensions (e.g., number of columns in a matrix)

How should this information be included? There are many strategies, but we choose to include these in a minimal binary header

### Principles of the .mda format

The . mda format conforms to the following principles:

Each .mda file contains one and only one multi-dimensional array of byte, integer, or floating point numbers.
The .mda file contains a small well-defined header containing only the minimal information required to read the array, namely the number and size of the dimensions as well as the data format of the entries.
Immediately following the header, the data of the multi-dimensional array is stored in raw binary format.

### File format description

The .mda file format has evolved slightly over time (for example the first version only supported complex numbers), so please forgive the few arbitrary choices.

The first four bytes contains a 32-bit signed integer containing a negative number representing the data format:

* -1 is complex float32
* -2 is byte
* -3 is float32
* -4 is int16
* -5 is int32
* -6 is uint16
* -7 is double
* -8 is uint32

In the rare case the this integer is a positive number (from earliest versions), it represents the number of dimensions, and the data format is complex float32. In this rare case, skip the next 8 bytes (since we already know the number of dimensions and the number of bytes per entry).
The next four bytes contains a 32-bit signed integer representing the number of bytes in each entry (okay a bit redundant, I know).
The next four bytes contains a 32-bit signed integer representing the number of dimensions (num_dims should be between 1 and 50).
The next 4*num_dims bytes contains a list of signed 32-bit integers representing the size of each of the dimensions.
That's it! Next comes the raw data.

### Raw/Preprocessed files

The raw and preprocessed data are stored in MxN arrays (M = #channels, N=#timepoints). In other words, these are two-dimensional .mda files.

### Input/Output .mda functions

There are readmda and writemda functions in the mountainlab repostory written in matlab (readmda.m; writemda.m) in mountainlab/matlab/msutils. Also found in the repository are .mda I/O functions in C++. Python I/O functions are under development.

### Format of the firings.mda

"firings.mda" is the output file containing the times (sample number or index, NOT in seconds) and corresponding labels.

The output of a sorting run is provided in a 2D array usually named "firings.mda". The dimensions are RxL where L is the number of events and R is at least 3.

Each column is a firing event.

The first row contains the integer channels corresponding to the primary channels for each firing event. It is important to note that the channel identification number is relative. In other words, if you only sort channels 61-64, the channel identifications will be 1-4.
This primary identification channel information is optional and can be filled with zeros. It is especially useful for algorithms that sort on (neighborhoods of) individual channels and then consolidate the spike types.

The second row contains the integer time points (1-based indexing) of the firing events

The third row contains the integer labels, or the sorted spike types.

The fourth row (optional and not currently exported by default) contains the peak amplitudes for the firing events.

Further rows may be used in the future for providing reliability metrics for individual events, or quantities that identify outliers.

### Convert to .mda

mdaconvert is a command-line utility in the repository which allows for conversion to .mda from: .dat,.csv,.ncs,.nrd

For example:
> mdaconvert input.dat output.mda --dtype=uint16 --dims=32x100x44

Type `mdaconvert` from the command line to get more information.

### Trodes and .mda

[Trodes](http://spikegadgets.com/software/trodes.html) has an `extractmda` function that reads in the .rec file and produces a .mda for each electrode array.

```bash
>exportmda -h
Used to extract data from a raw rec file and save to a set of files in MDA format.
NOTE: FILTERING IS TURNED OFF.
Usage:  exportmda -rec INPUTFILENAME OPTION1 VALUE1 OPTION2 VALUE2 ...

Input arguments
Defaults:
-outputrate -1 (full)
-usespikefilters 1
-interp -1 (inf)
-userefs 1

-rec <filename>  -- Recording filename. Required. Muliple -rec <filename> entries can be used to append data in output.
-highpass <integer> -- High pass filter value. Overrides settings in file.
-lowpass <integer> -- Low pass filter value. Overrides settings in file.
-outputrate <integer> -- Define the output sampling rate in the output file(s).
-interp <integer> -- Maximum number of dropped packets to interpolate.
-userefs <1 or 0> -- Whether or not to subtract digital references.
-usespikefilters <1 or 0> -- Whether or not to apply the spike filter settings in the file.
-output <basename> -- The base name for the output files. If not specified, the base name of the first .rec file is used.
-outputdirectory <directory> -- A root directory to extract output files to (default is directory of .rec file).-reconfig <filename> -- Use a different workspace than the one embedded in the recording file.
```
