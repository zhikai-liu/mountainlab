function generate_datasets_for_neuron_simulations

random_seed(1);

neuron_tetrode_path='/home/magland/prvdata/neuron_paper/tetrode/20160426_kanye_02_r1.nt16.mda';
samplerate=30000;

mkdir('tmpdata');
mkdir('raw');
mkdir('datasets');

noise_fname='tmpdata/tet_noise.mda';
oo=struct;
oo.samplerate=samplerate;
create_noise_dataset(neuron_tetrode_path,noise_fname,oo);
sz=readmdadims(noise_fname);
M=sz(1); N=sz(2);

oo=struct;
oo.K=8;
oo.M=M;
oo.N=N;
oo.samplerate=samplerate;
create_signal_dataset('tmpdata/sig_tet_K=8.mda',oo);

snrs=[3,6,9,12];
K=8;
datasets_txt='';
for j=1:length(snrs)
    str0=sprintf('tet_K=%d',K);
    str1=sprintf('%s_snr=%g',str0,snrs(j));
    sig_fname=sprintf('tmpdata/sig_%s.mda',str0);
    raw_fname=sprintf('raw/%s.mda',str1);
    oo=struct;
    oo.snr=snrs(j);
    create_dataset(noise_fname,sig_fname,raw_fname,oo);
    datasets_txt=sprintf('%s%s datasets/%s\n',datasets_txt,str1,str1);
end;

A=dir('raw');
oo=struct;
oo.samplerate=samplerate;
for j=1:length(A)
    B=A(j);
    raw_fname=B.name;
    if (~strcmp(raw_fname(1),'.'))&&(strcmp(raw_fname(end-3:end),'.mda'))
        str0=raw_fname(1:end-4);
        raw_fname=sprintf('raw/%s',raw_fname);
        dirname=sprintf('datasets/%s',str0);
        fprintf('Creating prv dataset: %s -> %s...\n',raw_fname,dirname);
        create_prv_dataset(raw_fname,dirname,oo);
    end;
end;
write_text_file('datasets.txt',datasets_txt);

function create_prv_dataset(raw_fname,dirname_out,opts)
mkdir(dirname_out);
system(sprintf('prv-create %s %s',raw_fname,[dirname_out,'/raw.mda.prv']));
params_txt=sprintf('{"samplerate":%g}',opts.samplerate);
write_text_file(sprintf('%s/params.json',dirname_out),params_txt);

function create_dataset(noise_fname,sig_fname,out_fname,opts)
fprintf('Reading noise: %s...\n',noise_fname);
Xnoise=readmda(noise_fname);

fprintf('Reading signal: %s...\n',sig_fname);
Xsig=readmda(sig_fname);

fprintf('Combining signal and noise, snr=%g...\n',opts.snr);
Y=Xnoise+opts.snr*Xsig;
max_16bit=2^14; %a bit safe

scale_factor_16bit=max_16bit/max(abs(Y(:)));
fprintf('Scaling for 16-bit output: factor=%g...\n',scale_factor_16bit);
Y=Y*scale_factor_16bit;

fprintf('Writing dataset: %s...\n',out_fname);
writemda16i(Y,out_fname);

function create_signal_dataset(out_path,opts)
oo=struct;
oo.M=opts.M;
oo.N=opts.N;
oo.K=opts.K;
oo.samplerate=opts.samplerate;
oo.refractory_period=10;
oo.noise_level=0;
oo.firing_rate_range=[0.5,3];
oo.amp_variation_range=[1,1];

fprintf('Synthesizing signal timeseries...\n');
Y=synthesize_timeseries_001(oo);

fprintf('Writing signal timeseries: %s...\n',out_path);
writemda32(Y,out_path);

function create_noise_dataset(raw_path,noise_out_path,oo)
opts.samplerate=oo.samplerate;
opts.detect_freq_min=800;
opts.detect_freq_max=6000;
opts.detect_threshold=3.5;
opts.min_segment_size=600;
opts.segment_buffer=100;
opts.blend_overlap_size=100;
opts.signal_scale_factor=100;
opts.num_units=8;

generate_noise_dataset(raw_path,noise_out_path,opts);

function Y=generate_noise_dataset(raw_fname,noise_out_fname,opts);

[path0,fname0,ext0]=fileparts(raw_fname);
filt_fname=sprintf('%s/filt_%s.mda','tmpdata',fname0);

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
fprintf('Normalizing channels for noise dataset...\n');
% Question: should we really do this channel-by-channel?
for m=1:M
    scale_factor=1/sqrt(var(Xfilt(m,:)));
    Xfilt(m,:)=Xfilt(m,:)*scale_factor; %normalize channels
    X(m,:)=X(m,:)*scale_factor;
end;

disp('Finding noise time segment intervals...');
maxabs=max(abs(Xfilt),[],1);
inds=find(maxabs >= opts.detect_threshold);
diffs=diff(inds);
aa=find(diffs>opts.min_segment_size+2*opts.segment_buffer);

disp('Gathering noise segments...');
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

disp('Blending noise segments...');
Y=blend_segments(segments,opts.blend_overlap_size);
fprintf('Using %g%% of dataset\n',size(Y,2)/size(X,2)*100);

fprintf('Writing %s...\n',noise_out_fname);
writemda32(Y,noise_out_fname);

function run_bandpass_filter(fname_in,fname_out,opts)
cmd=sprintf('mp-run-process mountainsort.bandpass_filter --timeseries=%s --timeseries_out=%s --samplerate=%g --freq_min=%g --freq_max=%g',...
    fname_in,fname_out,opts.samplerate,opts.freq_min,opts.freq_max);
cmd=sprintf('%s %s','LD_LIBRARY_PATH=/usr/local/lib',cmd);
fprintf('Running: %s\n',cmd);
system(cmd);

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


function random_seed(seed)
try
  rng(seed); %not available in octave
catch err
  rand('seed',seed);
  randn('seed',seed);
end

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
