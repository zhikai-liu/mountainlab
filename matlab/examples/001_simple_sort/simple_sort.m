function simple_sort

% You must first compile mountainlab and run matlab/mountainlab_setup.m

rng(1);

raw_dirname='MAGLAND_SIM001';

if ~exist(raw_dirname,'dir'), mkdir(raw_dirname); end;
if ~exist('data','dir'), mkdir('data'); end;
samplerate=30000;

% Simulate a random dataset and save to raw directory
if ~exist([raw_dirname,'/raw.mda'],'file')||~exist([raw_dirname,'/geom.csv'],'file')
    ogen.samplerate=samplerate; % Hz
    ogen.duration=600; % seconds
    ogen.num_channels=4; % Number of channels
    ogen.num_units=20; % Number of simulated neurons
    [X,firings_true,waveforms_true]=generate_raw(ogen);

    % Write to file
     writemda16i(X,[raw_dirname,'/raw.mda']);
     writemda64(firings_true,[raw_dirname,'/firings_true.mda']);
     writemda32(waveforms_true,[raw_dirname,'/waveforms_true.mda']);

    % Write lineary geometry file
    geom=1:ogen.num_channels;
    write_geometry_file(geom,[raw_dirname,'/geom.csv']);
end;

% View the simulated raw dataset
% view_timeseries(X);

% Bandpass filter
mp_run_process('mountainsort.bandpass_filter',...
    struct('timeseries',[raw_dirname,'/raw.mda']),...
    struct('timeseries_out','data/filt.mda'),...
    struct('freq_min',300, 'freq_max',6000, 'samplerate',samplerate));
    
% Whiten
mp_run_process('mountainsort.whiten',...
    struct('timeseries','data/filt.mda'),...
    struct('timeseries_out','data/pre.mda'),...
    struct());

% Spike sorting using MountainSort
sort_opts.adjacency_radius=100;
sort_opts.t1=-1; sort_opts.t2=-1;
mp_run_process('mountainsort.mountainsort3',...
    struct('timeseries','data/pre.mda', 'geom',[raw_dirname,'/geom.csv']),...
    struct('firings_out','data/firings.mda'),...
    sort_opts);

% Compute templates
mp_run_process('mountainsort.compute_templates',...
    struct('timeseries','data/filt.mda', 'firings','data/firings.mda'),...
    struct('templates_out','data/templates_filt.mda'),...
    struct('clip_size',100));
mp_run_process('mountainsort.compute_templates',...
    struct('timeseries','data/pre.mda', 'firings','data/firings.mda'),...
    struct('templates_out','data/templates_pre.mda'),...
    struct('clip_size',100));

% Compute metrics
metrics_list={};
mp_run_process('mountainsort.cluster_metrics',...
    struct('timeseries','data/pre.mda', 'firings','data/firings.mda'),...
    struct('cluster_metrics_out','data/cluster_metrics_1.json'),...
    struct('samplerate',samplerate));
metrics_list{end+1}='data/cluster_metrics_1.json';

mp_run_process('mountainsort.isolation_metrics',...
    struct('timeseries','data/pre.mda', 'firings','data/firings.mda'),...
    struct('metrics_out','data/cluster_metrics_2.json'),...
    struct('samplerate',samplerate));
metrics_list{end+1}='data/cluster_metrics_2.json';

mp_run_process('mountainsort.combine_cluster_metrics',...
    struct('metrics_list',{metrics_list}),...
    struct('metrics_out','data/cluster_metrics.json'));

% Automated annotation
script_fname='example_annotation.script';
mp_run_process('mountainsort.run_metrics_script',...
    struct('metrics','data/cluster_metrics.json','script',script_fname),...
    struct('metrics_out','data/cluster_metrics_annotated.json'),...
    struct());

% View the templates
figure; ms_view_templates(readmda('data/templates_filt.mda'));

% Launch the viewer
mountainview(struct(...
    'raw',[raw_dirname,'/raw.mda'],... 
    'filt','data/filt.mda',...
    'pre','data/pre.mda',...
    'firings','data/firings.mda',...
    'samplerate',samplerate,...
    'cluster_metrics','data/cluster_metrics_annotated.json'...
));

end

function [X,firings_true,waveforms_true]=generate_raw(opts)
ooo.K=opts.num_units;
ooo.M=opts.num_channels;
ooo.samplerate=opts.samplerate;
ooo.duration=opts.duration;
ooo.show_figures=false;
[X,firings_true,waveforms_true]=synthesize_timeseries_002(ooo);

end

function write_geometry_file(geom,fname_out)
csvwrite(fname_out,geom');

end

function view_timeseries(X)
raw_path=pathify32(X);
ld_library_str='LD_LIBRARY_PATH=/usr/local/lib';
cmd=sprintf('%s mountainview --raw=%s',ld_library_str,raw_path);
system(cmd);

end

function mountainview(A)
ld_library_str='LD_LIBRARY_PATH=/usr/local/lib';
args='';
keys=fieldnames(A);
for j=1:length(keys)
    args=sprintf('%s--%s=%s ',args,keys{j},num2str(A.(keys{j})));
end;
cmd=sprintf('%s mountainview %s &',ld_library_str,args);
fprintf('%s\n',cmd);
system(cmd);
end
