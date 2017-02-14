function bandpass_filter_kernel(timeseries,timeseries_out,opts)

X=readmda(timeseries);
X=do_filter(X,opts);
writemda(X,timeseries_out);

function X=do_filter(X,opts)
opts2.samplerate=opts.samplerate;
opts2.freq_min=opts.freq_min;
opts2.freq_max=opts.freq_max;
X=ms_bandpass_filter(X,opts2);
