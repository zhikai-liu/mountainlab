function eeg_spectrogram(timeseries_in_path,spectrogram_out_path,opts)

if (~isfield(opts,'time_resolution')) opts.time_resolution=32; end;

X=readmda(timeseries_in_path);

samplerate=128; %Hz
freqs=1:1:samplerate/2; %Hz

[M,N]=size(X);
N2=floor(N/opts.time_resolution)*opts.time_resolution;
N3=N2/opts.time_resolution;

tt=[0:N/2-1,-N/2:-1]/samplerate;
beta=2;

kernels=zeros(N,length(freqs));
for jj=1:length(freqs)
    sig=beta/freqs(jj);
    kernels(:,jj)=exp(-tt.^2/(2*sig^2)).*exp(2*pi*i*freqs(jj).*tt);
end;

channels=1:M;

spectrogram=zeros(length(channels),N3,length(freqs));

for ich=1:length(channels)
    fprintf('Channel %d\n',channels(ich));
    result0=zeros(N,length(freqs));
    for jj=1:length(freqs)
        kernel=kernels(:,jj);
        denom=sum(abs(kernel).^2);
        tmp=ifft(fft(X(channels(ich),:)).*conj(fft(kernel')))';
        result0(:,jj)=tmp/denom;
    end;
    result0=result0(1:N2,:);
    result0=reshape(result0,opts.time_resolution,N3,length(freqs));
    result0=max(abs(result0),[],1);
    spectrogram(ich,:,:)=squeeze(result0);

    %figure;
    %image(real(result0)'*10);
    %drawnow;
end;

writemda32(spectrogram,spectrogram_out_path);

