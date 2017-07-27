function [X,firings_true,waveforms]=synthesize_timeseries_002(opts)

if (~isfield(opts,'M')), opts.M=4; end; % #channels
if (~isfield(opts,'T')), opts.T=800; end; % #timepoints in synthetic waveforms
if (~isfield(opts,'K')), opts.K=20; end; % #synthetic units
if (~isfield(opts,'samplerate')), opts.samplerate=30000; end; %Hz
if ((~isfield(opts,'duration'))&&(~isfield(opts,'N'))), opts.duration=600; end; %total duration in seconds
if (~isfield(opts,'N')), opts.N=ceil(opts.duration*opts.samplerate); end
if (~isfield(opts,'upsamplefac')), opts.upsamplefac=13; end; %upsampling for synthetic waveforms
if (~isfield(opts,'refractory_period')), opts.refractory_period=10; end; %ms
if (~isfield(opts,'noise_level')), opts.noise_level=1; end; %
if (~isfield(opts,'firing_rate_range')), opts.firing_rate_range=[0.5,3]; end;
if (~isfield(opts,'amp_variation_ranges')), opts.amp_variation_ranges=[1,1]; end;
if (~isfield(opts,'shape_variation_factors')), opts.shape_variation_factors=[0]; end;
if (~isfield(opts,'show_figures')), opts.show_figures=0; end;
if (isfield(opts,'random_seed')), random_seed(opts.random_seed); end;
if (~isfield(opts,'peak_amplitudes')), opts.peak_amplitudes=[]; end;
if (~isfield(opts,'show_figures')), opts.show_figures=false; end;

% seed=1;
% try
%   rng(seed); %not available in octave
% catch err
%   rand('seed',seed);
%   randn('seed',seed);
% end

M=opts.M;
T=opts.T;
K=opts.K;
samplerate=opts.samplerate;
N=opts.N;
noise_level=opts.noise_level;
synth_opts.upsamplefac=opts.upsamplefac;
refr=opts.refractory_period;

if (size(opts.amp_variation_ranges,1)==1) opts.amp_variation_ranges=repmat(opts.amp_variation_ranges,opts.K,1); end;
if (length(opts.shape_variation_factors)==1) opts.shape_variation_factors=ones(opts.K,1)*opts.shape_variation_factors; end;
if (isempty(opts.peak_amplitudes))
    max_amp=20;
    opts.peak_amplitudes=((1:K)/K).^2*max_amp; % in units of std.dev noise
end;


firing_rates=opts.firing_rate_range(1)+rand(1,K)*(opts.firing_rate_range(2)-opts.firing_rate_range(1));
%tmp=opts.amp_variation_ranges(1)+rand(1,K)*(opts.amp_variation_ranges(2)-opts.amp_variation_ranges(1));
%amp_variations=zeros(2,K); amp_variations(1,:)=tmp; amp_variations(2,:)=1; %amplitude variation
amp_variation_ranges=opts.amp_variation_ranges;

opts2=struct;
opts2.geom_spread_coef1=0.4;
opts2.upsamplefac=synth_opts.upsamplefac;
opts2.peak_amplitudes=opts.peak_amplitudes;
pp=randperm(K);
opts2.peak_amplitudes=opts2.peak_amplitudes(pp); % apply a random permutation so they are not related to geometry location
waveforms=synthesize_random_waveforms(M,T,K,opts2);

if (opts.show_figures)
    figure; ms_view_templates(waveforms);
end;

% events/sec * sec/timepoint * N
populations=ceil(firing_rates/samplerate*N);
times=zeros(1,0);
labels=zeros(1,0);
ampls=zeros(1,0);
shape_offset_factors=zeros(1,0);
for k=1:K
    refr_timepoints=refr/1000*samplerate;
    
%     a=refr_timepoints;
%     b=2*N/populations(k)-a; %(a+b)/2=N/pop so b=2*N/pop-a
%     isi=rand_uniform(a,b,[1,populations(k)*2]); %x2 to be safe
%     times0=cumsum(isi);
%     times0=times0((times0>=1)&(times0<=N));
    
    times0=rand(1,populations(k))*(N-1)+1;
    times0=[times0,times0+rand_distr2(refr_timepoints,refr_timepoints*20,size(times0))];
    times0=times0(randsample0(length(times0),ceil(length(times0)/2)));
    times0=enforce_refractory_period(times0,refr_timepoints);
    times0=times0((times0>=1)&(times0<=N));
    
    times=[times,times0];
    labels=[labels,k*ones(size(times0))];
    amp1=amp_variation_ranges(k,1);
    amp2=amp_variation_ranges(k,2);
    ampls=[ampls,rand_uniform(amp1,amp2,size(times0)).*ones(size(times0))];
    shape_offset_factors=[shape_offset_factors,randn(size(times0))*opts.shape_variation_factors(k)];
end;

firings_true=zeros(3,length(times));
firings_true(2,:)=times;
firings_true(3,:)=labels;

X=synthesize_timeseries_2(waveforms,N,times,labels,ampls,shape_offset_factors,synth_opts);
X=X+randn(size(X))*noise_level;

end

function X=rand_distr2(a,b,sz)
X=rand(sz);
X=a+(b-a)*X.^2;
end

function X=rand_uniform(a,b,sz)
X=rand(sz);
X=a+(b-a)*X;
end

function times0=enforce_refractory_period(times0,refr)
if (length(times0)==0) return; end;
times0=sort(times0);
done=0;
while ~done
    diffs=times0(2:end)-times0(1:end-1);
    diffs=[diffs,inf]; %hack to make sure we handle the last one
    inds0=find((diffs(1:end-1)<=refr)&(diffs(2:end)>refr)); %only first violator in every group
    if (length(inds0)>0)
        times0(inds0)=-1; %kind of a hack, what's the better way?
        times0=times0(times0>=0);
    else
        done=1;
    end;
end
end

function Y=randsample0(n,k)
Y=randperm(n);
Y=Y(1:k);
end

function random_seed(seed)
try
  rng(seed); %not available in octave
catch err
  rand('seed',seed);
  randn('seed',seed);
end
end

function Y = synthesize_timeseries_2(W,N,times,labels,ampls,shape_offset_factors,opts)
% SYNTHESIZE_TIMESERIES.  Make timeseries via linear addition of spikes forward model
%
% Y = synthesize_timeseries(W,T,times,labels,ampls,opts) outputs a synthetic time series
% given waveforms and firing information; ie, it applies the forward model.
% No noise is added.
%
% Inputs:
%  W - waveform (templates) array, M by T by K. (note: assumed not upsampled
%      unless opts.upsamplefac > 1).
%  N - integer number of time points to generate.
%  times - (1xL) list of firing times, integers or real,
%          where 1st output time is 1 and last is N (ie, sample index units).
%  labels - (1xL) list of integer labels in range 1 to K.
%  ampls - (optional, 1xL) list of firing amplitudes (if absent or empty,
%          taken as 1).
%  opts - (optional struct) controls various options such as
%         opts.upsamplefac : integer ratio between sampling rate of W and for
%                            the output timeseries.
% Outputs:
%  Y - (MxN, real-valued) timeseries

% Barnett 2/19/16 based on validspike/synthesis/spikemodel.m; 2/25/16 upsampled.
% name change on 3/18/16 (used to be ms_synthesize) - jfm
% todo: faster C executable acting on MDAs I/O.

if nargin==0, test_synthesize_timeseries; return; end
if nargin<5 || isempty(ampls), ampls = 1.0+0*times; end      % default ampl
if nargin<6 || isempty(shape_offset_factors), shape_offset_factors=0*times; end;
if nargin<7, opts = []; end
if isfield(opts,'upsamplefac'), fac = opts.upsamplefac; else fac = 1; end
if fac~=round(fac), warning('opts.upsamplefac should be integer!'); end

[M T K] = size(W);
tcen = floor((T+1)/2);    % center firing time in waveform samples
ptsleft = tcen - 1; ptsright = T - tcen;
L = numel(times);
if numel(labels)~=L, error('times and labels must have same # elements!'); end
times = round(times*fac);    % times now in upsampled grid units
Y = zeros(M,N);
for j=1:L            % loop over spikes adding in each one
  iput = max(1,ceil((times(j)-ptsleft)/fac)):min(N,floor((times(j)+ptsright)/fac));  % indices to write to in output
  iget = tcen + fac*iput - times(j);   % inds to get from W; must be in 1 to T
  Y(:,iput) = Y(:,iput) + ampls(j)*apply_shape_offset(W(:,iget,labels(j)),shape_offset_factors(j));
end
end

function W2=apply_shape_offset(W,factor)
[~,maxind]=max(abs(W(:)));
Wsqrt=sqrt(abs(W)).*sign(W);
Wsqrt=Wsqrt*abs(W(maxind))/abs(Wsqrt(maxind));
W2=W+Wsqrt*factor;
W2=W2*W(maxind)/W2(maxind);
end
