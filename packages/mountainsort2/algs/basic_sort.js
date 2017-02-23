#!/usr/bin/env nodejs

var common=require('./common.js');

exports.spec=function() {
	var spec0={
	    name: "mountainsort.basic_sort",
	    version: "0.1",
	    description: "",
	    inputs: [
	        {name:"timeseries",description:"preprocessed timeseries (M x N)"}
	    ],
	    outputs: [
	        {name:"firings_out",description:"The labeled events (3 x L)"},
	        {name:"filt_out",description:"",optional:true},
	        {name:"pre_out",description:"",optional:true}
	    ],
	    parameters: [
	        {name:"samplerate",description:"sample rate for timeseries"},
	        {name:"central_channel",optional:true},
	        {name:"freq_min",optional:true},{name: "freq_max",optional:true},{name: "freq_wid",optional:true},
	        {name:"clip_size_msec",optional:true},
	        {name:"detect_interval_msec",optional:true},{name: "detect_threshold",optional:true},{name: "detect_sign",optional:true},
	        {name:"whiten",optional:true},{name:"consolidate_clusters",optional:true},{name:"fit_stage",optional:true},
	        {name:"_temp_prefix",optional:true} //internal
	    ]
	};
	return common.clone(spec0);
};

exports.run=function(opts,callback) {
	if (!opts._tempdir) {
		console.error('opts._tempdir is empty');
		process.exit(-1);
	}
	if (!opts.samplerate) {
		console.error('opts.samplerate is zero or empty');
		process.exit(-1);	
	}
	opts.central_channel=opts.central_channel||0;
	opts.clip_size_msec=opts.clip_size_msec||2;
	opts.detect_interval_msec=opts.detect_interval_msec||1;
	opts.detect_threshold=opts.detect_threshold||3.5;
	opts.whiten=opts.whiten||false;

	var raw=opts.timeseries;
	var filt='';
	var pre='';
	var event_times='';
	var clips='';
	var labels='';
	var labels2=''; //consolidated
	var firings='';
	var firings2=''; //after fitting

	var steps=[]; //processing steps
	var tmpfiles=[]; //files to be removed at the end
	
	////////////////////////////////////////////////////////
	//bandpass filter
	if (opts.freq_max) {
		filt=mktmp('filt.mda');
		steps.push(function(cb) {
			console.log ('>>>>> Bandpass Filter');
			bandpass_filter(raw,filt,cb);
		});
	}
	else {
		filt=raw;
	}

	////////////////////////////////////////////////////////
	//whiten
	if (opts.whiten) {
		pre=mktmp('pre.mda');
		steps.push(function(cb) {
			console.log ('>>>>> Whiten');
			whiten(filt,pre,cb);
		});
	}
	else {
		pre=filt;
	}

	if (opts.firings_out) {
		////////////////////////////////////////////////////////
		//detect
		event_times=mktmp('event_times.mda');
		steps.push(function(cb) {
			console.log ('>>>>> Detect');
			detect_events(pre,event_times,cb);
		});

		////////////////////////////////////////////////////////
		//extract clips
		steps.push(function(cb) {
			console.log ('>>>>> Extract clips');
			clips=mktmp('clips.mda');
			extract_clips(pre,event_times,clips,cb);
		});

		////////////////////////////////////////////////////////
		//sort clips
		labels=mktmp('labels.mda');
		steps.push(function(cb) {
			console.log ('>>>>> Sort clips');
			sort_clips(clips,labels,cb);
		});

		////////////////////////////////////////////////////////
		//consolidate clusters
		if (opts.consolidate_clusters) {
			labels2=mktmp('labels2.mda');
			steps.push(function(cb) {
				console.log ('>>>>> Consolidate clusters');
				consolidate_clusters(clips,labels,labels2,cb);
			});
		}
		else {
			labels2=labels;
		}

		////////////////////////////////////////////////////////
		//create firings
		firings=mktmp('firings.mda');
		steps.push(function(cb) {
			console.log ('>>>>> Create firings');
			create_firings(event_times,labels2,firings,cb);
		});

		////////////////////////////////////////////////////////
		//fit stage
		if (opts.fit_stage) {
			firings2=mktmp('firings2.mda');
			steps.push(function(cb) {
				console.log ('>>>>> Fit stage');
				fit_stage(pre,firings,firings2,cb);
			});
		}
		else {
			firings2=firings;
		}

		////////////////////////////////////////////////////////
		//copy to firings_out
		steps.push(function(cb) {
			console.log ('>>>>> Copying file '+firings2+' -> '+opts.firings_out);
			common.copy_file(firings2,opts.firings_out,cb);
		});
	}

	////////////////////////////////////////////////////////
	//copy to filt_out
	if (opts.filt_out) {
		steps.push(function(cb) {
			console.log ('>>>>> Copying file '+filt+' -> '+opts.filt_out);
			common.copy_file(filt,opts.filt_out,cb);
		});
	}

	////////////////////////////////////////////////////////
	//copy to pre_out
	if (opts.pre_out) {
		steps.push(function(cb) {
			console.log ('>>>>> Copying file '+pre+' -> '+opts.pre_out);
			common.copy_file(pre,opts.pre_out,cb);
		});
	}

	////////////////////////////////////////////////////////
	//cleanup
	steps.push(function(cb) {
		console.log ('>>>>> Cleanup');
		cleanup(cb);
	});
	
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	//Run all steps
	common.foreach(steps,{},function(ii,step,cb) {
		console.log('');
		console.log('--------------------------- STEP '+(ii+1)+' of '+steps.length +' -----------');
		var timer=new Date();
		step(function() {
			console.log ('Elapsed time (sec): '+get_elapsed_sec(timer));
			cb();
		});
	},function() {
		callback();
	});
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	function cleanup(callback) {
		common.remove_temporary_files(tmpfiles,callback);
	}

	function bandpass_filter(timeseries,timeseries_out,callback) {
		common.mp_exec_process('mountainsort.bandpass_filter',
			{timeseries:timeseries},
			{timeseries_out:timeseries_out},
			{
				samplerate:opts.samplerate,
				freq_min:opts.freq_min||0,
				freq_max:opts.freq_max||0,
				freq_wid:opts.freq_wid||0
			},
			callback
		);
	}

	function whiten(timeseries,timeseries_out,callback) {
		common.mp_exec_process('mountainsort.whiten',
			{timeseries:timeseries},
			{timeseries_out:timeseries_out},
			{
			},
			callback
		);	
	}

	function detect_events(timeseries,event_times_out,callback) {
		var detect_interval=Math.ceil(opts.detect_interval_msec/1000*opts.samplerate);
		common.mp_exec_process('mountainsort.detect_events',
			{timeseries:timeseries},
			{event_times_out:event_times_out},
			{
				central_channel:opts.central_channel,
				detect_threshold:opts.detect_threshold,
				detect_interval:detect_interval,
				sign:opts.detect_sign
			},
			callback
		);
	}

	function extract_clips(timeseries,event_times,clips_out,callback) {
		var clip_size=Math.ceil(opts.clip_size_msec/1000*opts.samplerate);
		common.mp_exec_process('mountainsort.extract_clips',
			{timeseries:timeseries,event_times:event_times},
			{clips_out:clips_out},
			{
				clip_size:clip_size
			},
			callback
		);	
	}

	function sort_clips(clips,labels_out,callback) {
		common.mp_exec_process('mountainsort.sort_clips',
			{clips:clips},
			{labels_out:labels_out},
			{
			},
			callback
		);	
	}

	function consolidate_clusters(clips,labels,labels_out,callback) {
		common.mp_exec_process('mountainsort.consolidate_clusters',
			{clips:clips,labels:labels},
			{labels_out:labels_out},
			{
				central_channel:opts.central_channel
			},
			callback
		);
	}

	function create_firings(event_times,labels,firings_out,callback) {
		common.mp_exec_process('mountainsort.create_firings',
			{event_times:event_times,labels:labels},
			{firings_out:firings_out},
			{
				central_channel:opts.central_channel
			},
			callback
		);
	}

	function fit_stage(timeseries,firings,firings_out,callback) {
		common.mp_exec_process('mountainsort.fit_stage',
			{timeseries:timeseries,firings:firings},
			{firings_out:firings_out},
			{
			},
			callback
		);
	}

	function mktmp(name) {
		var temp_prefix=opts._temp_prefix||'00';
		var path=opts._tempdir+'/'+temp_prefix+'-'+name;
		tmpfiles.push(path);
		return path;
	}
};

function get_elapsed_sec(timer) {
	var stamp=new Date();
	return (stamp-timer)/1000;
}