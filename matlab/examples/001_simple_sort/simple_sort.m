function simple_sort

% You must first compile mountainlab and run matlab/mountainlab_setup.m

mkdir('data');

% Simulate a random dataset and save to a temporary file
ogen.samplerate=30000; % Hz
ogen.duration=600; % seconds
ogen.num_channels=4; % Number of channels
ogen.num_units=20; % Number of simulated neurons
X=generate_raw(ogen);

% View the simulated raw dataset
% view_timeseries(X);

% Write to file
% writemda16i(X,'data/raw.mda');

% Write lineary geometry file
geom=1:ogen.num_channels;
%write_geometry_file(geom,'data/geom.csv');

% Bandpass filter
mp_run_process('mountainsort.bandpass_filter',...
    struct('timeseries','data/raw.mda'),...
    struct('timeseries_out','data/filt.mda'),...
    struct('freq_min',300, 'freq_max',6000, 'samplerate',ogen.samplerate));
    
% Whiten
mp_run_process('mountainsort.whiten',...
    struct('timeseries','data/filt.mda'),...
    struct('timeseries_out','data/pre.mda'),...
    struct());

% Spike sorting using MountainSort
sort_opts.adjacency_radius=100;
mp_run_process('mountainsort.mountainsort3',...
    struct('timeseries','data/pre.mda', 'geom','data/geom.csv'),...
    struct('firings_out','data/firings.mda'),...
    sort_opts);

metrics_list={};
mp_run_process('mountainsort.cluster_metrics',...
    struct('timeseries','data/pre.mda', 'firings','data/firings.mda'),...
    struct('cluster_metrics_out','data/cluster_metrics_1.json'),...
    struct('samplerate',ogen.samplerate));
metrics_list{end+1}='data/cluster_metrics_1.json';

mp_run_process('mountainsort.isolation_metrics',...
    struct('timeseries','data/pre.mda', 'firings','data/firings.mda'),...
    struct('metrics_out','data/cluster_metrics_2.json'),...
    struct('samplerate',ogen.samplerate));
metrics_list{end+1}='data/cluster_metrics_2.json';

mp_run_process('mountainsort.combine_cluster_metrics',...
    struct('metrics_list',{metrics_list}),...
    struct('metrics_out','data/cluster_metrics.json'));

mountainview(struct(...
    'raw','data/raw.mda',... 
    'filt','data/filt.mda',...
    'pre','data/pre.mda',...
    'firings','data/firings.mda',...
    'samplerate',ogen.samplerate,...
    'cluster_metrics','data/cluster_metrics.json'...
));

function X=generate_raw(opts)
ooo.K=opts.num_units;
ooo.M=opts.num_channels;
ooo.samplerate=opts.samplerate;
ooo.duration=opts.duration;
ooo.show_figures=false;
X=synthesize_timeseries_002(ooo);

function write_geometry_file(geom,fname_out)
csvwrite(fname_out,geom');

function view_timeseries(X)
raw_path=pathify32(X);
ld_library_str='LD_LIBRARY_PATH=/usr/local/lib';
cmd=sprintf('%s mountainview --raw=%s',ld_library_str,raw_path);
system(cmd);

function mountainview(A)
ld_library_str='LD_LIBRARY_PATH=/usr/local/lib';
args='';
keys=fieldnames(A);
for j=1:length(keys)
    args=sprintf('%s--%s=%s ',args,keys{j},num2str(A.(keys{j})));
end;
cmd=sprintf('%s mountainview %s',ld_library_str,args);
fprintf('%s\n',cmd);
system(cmd);
