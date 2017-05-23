### MountainLab PRV System

### Separation of data storage from data management

Files and folders serve two purposes in MountainLab. First, they hold raw data – sometimes the raw data files can be enormous. Second, they help organize studies and experiments. The idea of the PRV system is to separate these two types of files. Among the many advantages, it allows studies to be packaged into relatively small .zip files, to be checked into git or svn repositories, and/or be hosted on servers with limited storage capabilities.

For example, a typical dataset in MountainLab is a directory containing a raw data file (raw.mda.prv), a parameter file (params.json), and a geometry file (geom.csv). All three of these files (including the raw file) are tiny text files. Normally the raw file would be a (potentially enormous) binary file, making it difficult to move the study around, email it to a collaborator or store in a reasonably sized repository. Here is an example raw.mda.prv file entitled raw.mda.prv:

```json
{
    "original_checksum": "9a42d2fb7d0e2cfd8c206c2ddc30640472ffab3d",
    "original_fcs": "head1000-da39a3ee5e6b4b0d3255bfef95601890afd80709",
    "original_path": "20160426_kanye_02_r1.nt16.mda",
    "original_size": 666616580,
    "prv_version": 0.1
}
```

The fields in this JSON document represent the stats of the file at the time the .prv file was created using the prv-create command-line utility as described below. This JSON document is a universal pointer to the original raw data file. The assumption is that the file is uniquely identified by its checksum (sha-1 hash).

Here is a description of the above fields:
**original_checksum**: The sha-1 hash of the entire raw file.

**original_fcs**: (optional) a code that can be used to quickly determine whether a given file is a candidate for a match.

**original_path**: Only used for information in case the user needs a reminder on the name and location of the raw file at the time the .prv file was created.

**original_size**: The size in bytes of the raw file.

**prv_version**: Always 0.1 for now.

Suppose we know that the original file is somewhere on the local disk, or on a particular remote server. The prv-locate utility may be used to efficiently find the path or url to the file.

### The prv command-line utility

The prv command-line utility can be used to create .prv files, locate the original files at a later date, or download the original files from a remote server.

Use **prv-create** to create a new .prv file that serves as a pointer to the original file:
```
prv-create [source_file_name] [destination_file_name]
prv-create raw.mda raw.mda.prv
```

Later, use prv-locate to find the original file, even if it had moved:
```
prv-locate [prv_file_name]
prv-locate raw.mda.prv
```
The full path to the file is written to stdout.

Use the mlconfig utility to set up the search paths for prv-locate on your system:
```
mlconfig
```
This utility can also be used to locate and/or download files stored on a remote computer that is running a prvbucket server (see below):
```
prv-locate [prv_file_name] --server=[prvbucket_server_url]
prv-locate raw.mda.prv --server=http://datalaboratory.org:8005
```

If found, this utility writes to stdout a url path to the remote file. The file can be downloaded using:
```
prv-download raw.mda.prv /path/to/destination/raw.mda --server=http://datalaboratory.org:8005
```

### Prvbucket server

The prvbucket server is a simple web server that interfaces with the prv command-line utility to allow binary files to be retrieved from a remote server. The source code is a single nodeJs file:
https://github.com/magland/mountainlab/tree/master/prv/prvbucketserver

```
Usage: prvbucketserver [data_directory] --listen_port=[port] --server_url=[http://localhost:8000]
```

[data_directory] -- location of data files to be served

[listen_port] -- the listen port for the server

[server_url] -- the url to be used as prefix for file download links

