function extract_time_chunk(timeseries,timeseries_out,opts)

dims=readmdadims(timeseries);
index0=[1,opts.t1+1];
size0=[dims(1),opts.t2-opts.t1+1];
X=readmda_block(timeseries,index0,size0);
writemda(X,timeseries_out);