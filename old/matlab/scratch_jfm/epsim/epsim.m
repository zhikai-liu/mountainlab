function epsim

global X;

opts.samplerate=30000;
opts.detect_freq_min=800;
opts.detect_freq_max=6000;
opts.detect_threshold=3.5;
opts.min_segment_size=600;
opts.segment_buffer=100;
opts.blend_overlap_size=100;
opts.signal_scale_factor=100;
opts.num_units=8;

disp('Reading...');
X=readmda('/home/magland/dev/fi_ss/raw/20160426_kanye_02_r1.nt16.mda');
%X=X(:,1:1e7);

disp('Generating noise dataset...');
Y_noise=generate_noise_dataset(X,opts);

disp('Generating signal dataset...');
[M,N]=size(Y_noise);
oo=struct;
oo.M=M;
oo.N=N;
oo.K=opts.num_units;
oo.samplerate=opts.samplerate;
oo.refractory_period=10;
oo.noise_level=0;
oo.firing_rate_range=[0.5,3];
oo.amp_variation_range=[1,1];
[Y_signal,firings_true,waveforms]=synthesize_timeseries_001(oo);

Y=Y_signal*opts.signal_scale_factor+Y_noise;

disp('Writing...');
writemda16i(Y,'tetrode_synth.mda');

function Y=generate_noise_dataset(X,opts);

[M,N]=size(X);
disp('Bandpass filter (for detection)...');
oo=struct;
oo.samplerate=opts.samplerate;
oo.freq_min=opts.detect_freq_min;
oo.freq_max=opts.detect_freq_max;
Xfilt=ms_bandpass_filter(X,oo);
for m=1:M
    Xfilt(m,:)=Xfilt(m,:)/sqrt(var(Xfilt(m,:))); %normalize channels
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



