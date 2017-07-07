function mp_run_process(processor_name,inputs,outputs,params)

%% Example: mp_run_process('mountainsort.bandpass_filter',struct('timeseries','tetrode6.mda'),struct('timeseries_out','tetrode6_filt.mda'),struct('samplerate',32556,'freq_min',300,'freq_max',6000));

cmd='mp-run-process';
cmd=[cmd,' ',processor_name];
cmd=[cmd,' ',create_arg_string(inputs)];
cmd=[cmd,' ',create_arg_string(outputs)];
cmd=[cmd,' ',create_arg_string(params)];
system_call(cmd);

function str=create_arg_string(params)
list={};
keys=fieldnames(params);
for i=1:length(keys)
    key=keys{i};
    val=params.(key);
    list{end+1}=sprintf('--%s=%s',key,create_val_string(val));
end;
str=strjoin(list,' ');

function str=create_val_string(val)
str=num2str(val);

function system_call(cmd)
cmd=sprintf('LD_LIBRARY_PATH=/usr/local/lib %s',cmd);
system(cmd);