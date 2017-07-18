function generate_datasets

% first run mountainlab_setup.m

% Here's the dataset from which we will generate the background signal
%neuron_tetrode_path='/home/magland/dev/mountainsort_experiments/BIGFILES/neuron_paper/tetrode/20160426_kanye_02_r1.nt16.mda';
neuron_tetrode_path='/home/magland/dev/fi_ss/raw/20160426_kanye_02_r1.nt16.mda';
%neuron_tetrode_path='/home/magland/prvdata/neuron_paper/tetrode/20160426_kanye_02_r1.nt16.mda';
samplerate=30000;

K=15; % K = true number of units
max_amp=20;
peak_amplitudes=((1:K)/K).^2*max_amp; % in units of std.dev noise
num_datasets=1;
rng(1);

% amplitude variation ranges
amp_variation_ranges=repmat([1,1],K,1);
shape_variation_factors=zeros(1,K);
%amp_variation_ranges=[0.5,1.5; 0.5,1.5; 1,1; 1,1; 1,1; 1,1; 1,1; 1,1; 1,1; 1,1; 1,1; 1,1];
%shape_variation_factors=[0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
%random_amp_factor_range=[1,1];

projpath=[fileparts(mfilename('fullpath')),'/../project'];

mkdir(projpath);
mkdir([projpath,'/tmpdata']);
mkdir([projpath,'/raw']);
mkdir([projpath,'/datasets']);

system(sprintf('cp %s %s',[projpath,'/../pipelines.txt'],[projpath,'/pipelines.txt']));

background_signal_fname=[projpath,'/tmpdata/tet_background.mda'];
if (~exist(background_signal_fname))
    oo=struct;
    oo.samplerate=samplerate;
    oo.tempdir=[projpath,'/tmpdata'];
    create_background_signal_dataset(neuron_tetrode_path,background_signal_fname,oo);
end;
sz=readmdadims(background_signal_fname);
M=sz(1); N=sz(2);

for ii=1:num_datasets
    oo=struct;
    oo.K=K;
    oo.M=M;
    oo.N=N;
    oo.samplerate=samplerate;
    oo.amp_variation_ranges=amp_variation_ranges;
    oo.shape_variation_factors=shape_variation_factors;
    oo.peak_amplitudes=peak_amplitudes;
    %oo.random_amp_factor_range=random_amp_factor_range;
    str0=sprintf('tet_K=%d_%d',K,ii);
    create_signal_dataset(sprintf('%s/tmpdata/sig_%s.mda',projpath,str0),sprintf('%s/tmpdata/firings_%s.mda',projpath,str0),sprintf('%s/tmpdata/waveforms_%s.mda',projpath,str0),oo);
    
    sig_fname=sprintf('%s/tmpdata/sig_%s.mda',projpath,str0);
    firings_fname=sprintf('%s/tmpdata/firings_%s.mda',projpath,str0);
    waveforms_fname=sprintf('%s/tmpdata/waveforms_%s.mda',projpath,str0);
    raw_out_fname=sprintf('%s/raw/%s.mda',projpath,str0);
    firings_out_fname=sprintf('%s/raw/firings_%s.mda',projpath,str0);
    waveforms_out_fname=sprintf('%s/raw/waveforms_%s.mda',projpath,str0);
    oo=struct;
    create_dataset(background_signal_fname,sig_fname,firings_fname,waveforms_fname,raw_out_fname,firings_out_fname,waveforms_out_fname,oo);
    
end;

datasets_txt='';
A=dir([projpath,'/raw']);
oo=struct;
oo.samplerate=samplerate;
for j=1:length(A)
    B=A(j);
    raw_fname=B.name;
    if (~strcmp(raw_fname(1),'.'))&&(strcmp(raw_fname(end-3:end),'.mda'))&&(~strcmp(raw_fname(1:length('firings')),'firings'))&&(~strcmp(raw_fname(1:length('waveforms')),'waveforms'))
        str0=raw_fname(1:end-4);
        raw_fname1=sprintf([projpath,'/raw/%s.mda'],str0);
        firings_fname=sprintf([projpath,'/raw/firings_%s.mda'],str0);
        waveforms_fname=sprintf([projpath,'/raw/waveforms_%s.mda'],str0);
        dirname=sprintf([projpath,'/datasets/%s'],str0);
        fprintf('Creating prv dataset: %s -> %s...\n',raw_fname1,dirname);
        create_prv_dataset(raw_fname1,firings_fname,waveforms_fname,dirname,oo);
        datasets_txt=sprintf('%s%s datasets/%s\n',datasets_txt,str0,str0);
    end;
end;
write_text_file([projpath,'/datasets.txt'],datasets_txt);

function create_prv_dataset(raw_fname,firings_fname,s_fname,dirname_out,opts)
mkdir(dirname_out);
system(sprintf('prv-create %s %s',raw_fname,[dirname_out,'/raw.mda.prv']));
system(sprintf('prv-create %s %s',firings_fname,[dirname_out,'/firings_true.mda.prv']));
system(sprintf('cp %s %s',waveforms_fname,[dirname_out,'/waveforms_true.mda']));
params_txt=sprintf('{"samplerate":%g}',opts.samplerate);
write_text_file(sprintf('%s/params.json',dirname_out),params_txt);
geom_txt=sprintf('0,0\n1,0\n2,0\n3,0\n');
write_text_file(sprintf('%s/geom.csv',dirname_out),geom_txt);

function create_dataset(background_fname,sig_fname,firings_fname,waveforms_fname,timeseries_out_fname,firings_out_fname,waveforms_out_fname,opts)
fprintf('Reading background: %s...\n',background_fname);
Xbackground=readmda(background_fname);

fprintf('Reading signal: %s...\n',sig_fname);
Xsig=readmda(sig_fname);

fprintf('Combining signal and background...\n');
Y=Xbackground+Xsig;
max_16bit=2^14; %a bit safe

scale_factor_16bit=max_16bit/max(abs(Y(:)));
fprintf('Scaling for 16-bit output: factor=%g...\n',scale_factor_16bit);
Y=Y*scale_factor_16bit;

fprintf('Writing dataset: %s...\n',timeseries_out_fname);
writemda16i(Y,timeseries_out_fname);

fprintf('Writing true firings: %s...\n',firings_out_fname);
writemda(readmda(firings_fname),firings_out_fname);

fprintf('Writing true waveforms: %s...\n',waveforms_out_fname);
waveforms0=readmda(waveforms_fname);
writemda(waveforms0,waveforms_out_fname);

function create_signal_dataset(timeseries_out_path,firings_out_path,waveforms_out_path,opts)
oo=struct;
oo.M=opts.M;
oo.N=opts.N;
oo.K=opts.K;
oo.samplerate=opts.samplerate;
oo.refractory_period=10;
oo.noise_level=0;
oo.firing_rate_range=[0.5,3];
oo.amp_variation_ranges=opts.amp_variation_ranges;
oo.shape_variation_factors=opts.shape_variation_factors;
oo.peak_amplitudes=opts.peak_amplitudes;
%oo.random_amp_factor_range=opts.random_amp_factor_range;

fprintf('Synthesizing signal timeseries...\n');
[Y,firings_true,waveforms_true]=synthesize_timeseries_001(oo);

fprintf('Writing signal timeseries: %s...\n',timeseries_out_path);
writemda32(Y,timeseries_out_path);
fprintf('Writing true firings: %s...\n',firings_out_path);
writemda32(firings_true,firings_out_path);
fprintf('Writing true waveforms: %s...\n',waveforms_out_path);
writemda32(waveforms_true,waveforms_out_path);


function create_background_signal_dataset(raw_path,background_signal_out_path,oo)
opts.samplerate=oo.samplerate;
opts.detect_freq_min=800;
opts.detect_freq_max=6000;
opts.detect_threshold=3.5;
opts.min_segment_size=600;
opts.segment_buffer=100;
opts.blend_overlap_size=100;
opts.signal_scale_factor=100;
opts.num_units=8;
opts.tempdir=oo.tempdir;
generate_background_signal_dataset(raw_path,background_signal_out_path,opts);

function Y=generate_background_signal_dataset(raw_fname,background_signal_out_fname,opts);

[path0,fname0,ext0]=fileparts(raw_fname);
filt_fname=sprintf('%s/filt_%s.mda',opts.tempdir,fname0);

sz=readmdadims(raw_fname);
M=sz(1); N=sz(2);
disp('Bandpass filter (for detection)...');
oo=struct;
oo.samplerate=opts.samplerate;
oo.freq_min=opts.detect_freq_min;
oo.freq_max=opts.detect_freq_max;
run_bandpass_filter(raw_fname,filt_fname,oo);

fprintf('Reading %s...\n',filt_fname);
Xfilt=readmda(filt_fname);
fprintf('Reading %s...\n',raw_fname);
X=readmda(raw_fname);
fprintf('Normalizing channels for background signal dataset...\n');
% Question: should we really do this channel-by-channel?
for m=1:M
    scale_factor=1/sqrt(var(Xfilt(m,:)));
    Xfilt(m,:)=Xfilt(m,:)*scale_factor; %normalize channels
    X(m,:)=X(m,:)*scale_factor;
end;

disp('Finding background time segment intervals...');
maxabs=max(abs(Xfilt),[],1);
inds=find(maxabs >= opts.detect_threshold);
diffs=diff(inds);
aa=find(diffs>opts.min_segment_size+2*opts.segment_buffer);

disp('Gathering background signal segments...');
segments=cell(1,length(aa));
numpts=0;
sizes=[];
for j=1:length(aa)
    i1=inds(aa(j))+opts.segment_buffer;
    i2=inds(aa(j)+1)-opts.segment_buffer;
    segments{j}=X(:,i1:i2);
    sizes(end+1)=size(segments{j},2);
    numpts=numpts+size(segments{j},2);
end;

disp('Blending background signal segments...');
Y=blend_segments(segments,opts.blend_overlap_size);
fprintf('Using %g%% of dataset\n',size(Y,2)/size(X,2)*100);

fprintf('Writing %s...\n',background_signal_out_fname);
writemda32(Y,background_signal_out_fname);

function run_bandpass_filter(fname_in,fname_out,opts)
cmd=sprintf('mp-run-process mountainsort.bandpass_filter --timeseries=%s --timeseries_out=%s --samplerate=%g --freq_min=%g --freq_max=%g',...
    fname_in,fname_out,opts.samplerate,opts.freq_min,opts.freq_max);
cmd=adjust_system_command(cmd);
fprintf('Running: %s\n',cmd);
system(cmd);

function cmd=adjust_system_command(cmd)
cmd=sprintf('%s %s','LD_LIBRARY_PATH=/usr/local/lib',cmd);

function Y=blend_segments(segments,overlap_width)
M=size(segments{1},1);
NN=0;
for j=1:length(segments)
    NN=NN+size(segments{j},2);
end;
NN=NN-(overlap_width)*(length(segments)-1);
Y=zeros(M,NN);
aa=1;
for j=1:length(segments)
    N0=size(segments{j},2);
    [blend_func1,~]=blendpythag([0:N0-1],overlap_width-1);
    [~,blend_func2]=blendpythag([-N0+overlap_width:overlap_width-1],overlap_width-1);
    blend_func=ones(size(blend_func1));
    if (j>1)
        blend_func=blend_func.*blend_func1;
    end;
    if (j<length(segments))
        blend_func=blend_func.*blend_func2;
    end;
    segment0=segments{j};
    segment0=segment0.*repmat(blend_func,M,1);
    Y(:,aa:aa+N0-1)=Y(:,aa:aa+N0-1)+segment0;
    aa=aa+N0-overlap_width;
end;

function write_text_file(fname,txt)
F=fopen(fname,'w');
fprintf(F,'%s',txt);
fclose(F);

function [phi psi] = blendpythag(t,w)
% BLENDPYTHAG - evaluate complementary blending functions at time points
%
% [phi psi] = blendpythag(t,w)
% Inputs:
%  t = vector of time points. The blending region is [0 w].
%  w = width of blending function in time samples
% Outputs:
%  phi, psi = phi(t) and psi(t) evaluated at the vector t, same sizes as t.
%
%  Pythagorean blending is such that phi(t)^2 + psi(t)^2 = 1, for all t
%  and phi=0 for t<0, psi=0 for t>w.
%  phi and psi are currently only C^1 smooth, should be sufficient for blending
%  of noise.
%  This is a one-way blending function; to make a two-way function centered
%  at t=w, use [phi psi] = blendpythag(t-2*max(0,t-w),w);
%
% Barnett 3/29/17
if nargin==0, test_blendpythag; return; end

t = min(max(t,0),w);  % clip
th = sin((pi/(2*w))*t).^2;   % values in 
phi = sin((pi/2)*th);
psi = cos((pi/2)*th);

%%%%%%
function test_blendpythag
w = 10;
t = -10:20;
[phi psi] = blendpythag(t,w);
figure; plot(t, [phi;psi], '.-'); legend('phi','psi');
