#!/usr/bin/env nodejs

var common=require('./common.js');

var num_runs=40;

var runs=[];
for (var i=0; i<num_runs; i++) {
	runs.push({id:i});
}

var params0={
	num_cpu_ops:0,
	num_read_bytes:5e10,
	num_write_bytes:0
};

var timer=new Date();
common.foreach(runs,{num_parallel:num_runs},function(ii,run,cb) {
	do_run(run,cb);
},function() {
	console.log('Elapsed time (sec): '+get_elapsed_sec(timer));
});

function do_run(run,cb) {
	var stats_fname='tmp-'+run.id+'.json';
	common.mp_exec_process(
			'mountainsort.load_test',
			{},
			{stats_out:stats_fname},
			params0,
			function() {
				var txt=common.read_text_file(stats_fname);
				console.log(txt);
				cb();
			}
	);
}

function get_elapsed_sec(timer) {
	var stamp=new Date();
	return (stamp-timer)/1000;
}

function get_elapsed_sec(timer) {
	var stamp=new Date();
	return (stamp-timer)/1000;
}