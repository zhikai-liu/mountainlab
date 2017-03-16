#!/usr/bin/env nodejs

var common=require('./common.js');
var os=require('os');

exports.spec=function() {
	var spec0={};
	spec0.name='mountainsort.subsample_sort';
	spec0.version='0.1';
	spec0.parameters.push({name:'segment_duration_sec'});
	spec0.parameters.push({name:'num_intrasegment_threads',optional:true});
	spec0.parameters.push({name:'num_intersegment_threads',optional:true});
	spec0.parameters.push({name:"samplerate",description:"sample rate for timeseries"});
	spec0.parameters.push({name:"central_channel",optional:true});
	spec0.parameters.push({name:"freq_min",optional:true},{name: "freq_max",optional:true},{name: "freq_wid",optional:true});
	spec0.parameters.push({name:"clip_size_msec",optional:true});
	spec0.parameters.push({name:"detect_interval_msec",optional:true},{name: "detect_threshold",optional:true},{name: "detect_sign",optional:true});
	spec0.parameters.push({name:"whiten",optional:true},{name:"consolidate_clusters",optional:true},{name:"fit_stage",optional:true});
	spec0.parameters.push({name:'subsample_factor',optional:true});
	return common.clone(spec0);
};

exports.run=function(opts,callback) {
	var tmpfiles=[];
	opts.temp_prefix=opts.temp_prefix||'00';
	opts.num_intrasegment_threads=Number(opts.num_intrasegment_threads||1);
	opts.num_intersegment_threads=opts.num_intersegment_threads||0;
	if (!opts.num_intersegment_threads) {
		// determine whether to use multi-threading within each segment
		// depending on whether we are doing intrasegment threads
		if (opts.num_intrasegment_threads<=1)
			opts.num_intersegment_threads=os.cpus().length;
		else
			opts.num_intersegment_threads=1;
	}
	if (!opts._tempdir) {
		console.error('opts._tempdir is empty');
		process.exit(-1);
	}
	if (!opts.samplerate) {
		console.error('opts.samplerate is zero or empty');
		process.exit(-1);	
	}
	if (!opts.segment_duration_sec) {
		console.error('opts.segment_duration_sec is zero or empty');
		process.exit(-1);		
	}

	var steps=[];
	var info={};
	var segments=[];
	var all_event_times=mktmp('all_event_times.mda');
	var whitening_matrix=mktmp('whitening_matrix.mda');
	var all_clips=mktmp('all_clips.mda');
	var all_whitened_clips=mktmp('all_whitened_clips.mda');
	var all_labels=mktmp('all_labels.mda');
	//var all_labels2=mktmp('all_labels.mda'); //consolidated
	var all_firings=mktmp('all_firings.mda');

	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		read_info_from_input_files(function() {
			cb();
		});		
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		process1_segments(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		combine_event_times(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		extract_all_clips(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		compute_whitening_matrix(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		whiten_all_clips(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		sort_all_whitened_clips(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		STEP_fit_stage(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		cluster_metrics(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		cleanup(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	//Run all steps
	common.foreach(steps,{},function(ii,step,cb) {
		console.log('');
		console.log('--------------------------- DRIFT SORT STEP '+(ii+1)+' of '+steps.length +' -----------');
		var timer=new Date();
		step(function() {
			console.log ('DRIFT SORT STEP '+(ii+1)+': Elapsed time (sec): '+get_elapsed_sec(timer));
			cb();
		});
	},function() {
		callback();
	});
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////

	function process1_segments(process1_segments_callback) {
		var intrasegment_steps=[];
		var segment_duration=Math.ceil(opts.segment_duration_sec*opts.samplerate);
		segments=create_segments(info.N,segment_duration,segment_duration);
		for (var iseg=0; iseg<segments.length; iseg++) {
			new add_intrasegment_step(iseg);
		}
		function add_intrasegment_step(iseg) {
			var segment=segments[iseg];
			segment.timeseries0=mktmp('timeseries_segment_'+iseg+'.mda');
			segment.filt0=mktmp('filt_segment_'+iseg+'.mda');
			segment.event_times0=mktmp('event_times0_segment_'+iseg+'.mda');
			segment.event_times1=mktmp('event_times1_segment_'+iseg+'.mda'); //timestamp offset
			var intersegment_steps=[];
			intrasegment_step.push(function(cb) {
				intersegment_steps.push(function(cb2) {
					console.log('>>>>>>>>>>>> Extracting timeseries for segment '+(iseg+1)+' ('+segment.t1+','+segment.t2+')...\n');	
					extract_segment_timeseries(opts.timeseries,segment.timeseries0,segment.t1,segment.t2,function() {
						cb2();
					});
				});
				intersegment_steps.push(function(cb2) {
					console.log('>>>>>>>>>>>> Bandpass filter for segment '+(iseg+1)+' ('+segment.t1+','+segment.t2+')...\n');	
					bandpass_filter(segment.timeseries0,segment.filt0,function() {
						cb2();
					});
				});
				intersegment_steps.push(function(cb2) {
					console.log('>>>>>>>>>>>> Detect events for segment '+(iseg+1)+' ('+segment.t1+','+segment.t2+')...\n');	
					detect_events(segment.timeseries0,segment.event_times0,function() {
						cb2();
					});
				});
				intersegment_steps.push(function(cb2) {
					console.log('>>>>>>>>>>>> Applying timestamp offset for events in segment '+(iseg+1)+' ('+segment.t1+','+segment.t2+')...\n');	
					apply_timestamp_offset(segment.event_times0,segment.event_times1,segment.t1,function() {
						cb2();
					});
				});
			});
		}
		//Run all intrasegment steps
		common.foreach(intrasegment,{num_parallel:opts.num_intrasegment_threads},function(ii,step,cb) {
			console.log('');
			console.log('--------------------------- PROCESS1 SEGMENT '+(ii+1)+' of '+segment_steps.length +' -----------');
			var timer=new Date();
			step(function() {
				console.log ('Elapsed time for process1 '+(ii+1)+' of '+(intrasegment_steps.length)+' (sec): '+get_elapsed_sec(timer));
				cb();
			});
		},function() {
			process1_segments_callback();
		});
	}

	function combine_event_times(combine_event_times_callback) {
		console.log('-------------------- COMBINING EVENT TIMES -------------------');
		var event_times_list=[];
		for (var ii in segments) {
			var segment=segments[ii];
			event_times_list.push(segment.event_times1);
		}
		common.mp_exec_process('mountainsort.concat_event_times',
			{event_times_list:event_times_list},
			{event_times_out:all_event_times},
			{},
			combine_event_times_callback
		);
	}

	function extract_all_clips(extract_all_clips_callback) {
		console.log('-------------------- EXTRACTING ALL CLIPS -------------------');
		var clip_size=Math.ceil(opts.clip_size_msec/1000*opts.samplerate);
		var filt_list=[];
		for (var ii in segments) {
			var segment=segments[ii];
			filt_list.push(segment.filt0);
		}
		common.mp_exec_process('mountainsort.extract_clips',
			{timeseries:filt_list,event_times:all_event_times},
			{clips_out:all_clips},
			{clip_size:clip_size},
			extract_all_clips_callback
		);
	}

	function compute_whitening_matrix(compute_whitening_matrix_callback) {
		console.log('-------------------- COMPUTING WHITENING MATRIX -------------------');
		var filt_list=[];
		for (var ii in segments) {
			var segment=segments[ii];
			filt_list.push(segment.filt0);
		}
		common.mp_exec_process('mountainsort.compute_whitening_matrix',
			{timeseries_list:filt_list},
			{whitening_matrix_out:whitening_matrix},
			{_request_num_threads:opts.num_intersegment_threads||1},
			compute_whitening_matrix_callback
		);	
	}

	function whiten_all_clips(whiten_all_clips_callback) {
		console.log('-------------------- WHITENING ALL CLIPS -------------------');
		common.mp_exec_process('mountainsort.whiten_clips',
			{clips:all_clips,whitening_matrix:whitening_matrix},
			{clips_out:all_whitened_clips},
			{_request_num_threads:opts.num_intersegment_threads||1},
			whiten_all_clips_callback
		);	
	}	

	function sort_all_whitened_clips(sort_all_whitened_clips_callback) {
		console.log('-------------------- SORTING WHITENING CLIPS -------------------');
		common.mp_exec_process('mountainsort.sort_clips',
			{clips:all_whitened_clips},
			{labels_out:all_labels},
			{_request_num_threads:opts.num_intersegment_threads||1},
			sort_all_whitened_clips_callback
		);	
	}	

	//to remove:
	function sort_segments(sort_segments_callback) {
		var segment_steps=[];
		var segment_duration=Math.ceil(opts.segment_duration_sec*opts.samplerate);
		var shift_duration=Math.ceil(opts.shift_duration_sec*opts.samplerate);
		segments=create_segments(info.N,segment_duration,shift_duration);
		for (var iseg=0; iseg<segments.length; iseg++) {
			new add_segment_step(iseg);
		}
		function add_segment_step(iseg) {
			var segment=segments[iseg];
			var timeseries0=mktmp('timeseries_segment_'+iseg+'.mda');
			var firings0=mktmp('firings_segment_'+iseg+'.mda');
			var firings1=mktmp('firings1_segment_'+iseg+'.mda'); //timestamp offset
			segments[iseg].firings1=firings1;
			segment_steps.push(function(cb) {
				console.log('>>>>>>>>>>>> Extracting timeseries for segment '+(iseg+1)+' ('+segment.t1+','+segment.t2+')...\n');
				extract_segment_timeseries(opts.timeseries,timeseries0,segment.t1,segment.t2,function() {
					var opts2=common.clone(opts);
					opts2.timeseries=timeseries0;
					opts2.firings_out=firings0;
					opts2.pre_out=null; //do not generate pre/filt output
					opts2.filt_out=null; //do not generate pre/filt output
					opts2.cluster_metrics_out='';
					opts2._temp_prefix=(opts._temp_prefix||'00')+'-segment_'+iseg;
					opts2._request_num_threads=opts.num_basic_intersegment_threads;
					opts2.console_prefix='SEGMENT '+(iseg+1)+'/'+segments.length+' --- ';
					console.log('>>>>>>>>>>>> Running basic sort for segment '+(iseg+1)+'...\n');
					basic_sort.run(opts2,function() {
						apply_timestamp_offset(firings0,firings1,segment.t1,function() {
							cb();
						});
					});
				});
			});
		}
		//Run all segment steps
		common.foreach(segment_steps,{num_parallel:opts.num_intrasegment_threads},function(ii,step,cb) {
			console.log('');
			console.log('--------------------------- SEGMENT '+(ii+1)+' of '+segment_steps.length +' -----------');
			var timer=new Date();
			step(function() {
				console.log ('Elapsed time for step sort '+(ii+1)+' of '+(segment_steps.length)+' (sec): '+get_elapsed_sec(timer));
				cb();
			});
		},function() {
			sort_segments_callback();
		});

		function apply_timestamp_offset(firings,firings_out,timestamp_offset,callback) {
			common.mp_exec_process('mountainsort.apply_timestamp_offset',
				{firings:firings},
				{firings_out:firings_out},
				{timestamp_offset:timestamp_offset},
				callback
			);
		}
	}

	function link_all_segments(link_all_segments_callback) {
		console.log('>>>>>>>>>>>> Linking '+segments.length+' segments...')
		common.foreach(segments,{},function(iseg,segment,cb) {
			console.log('Linking segment '+(iseg+1));
			var firings1=segment.firings1;
			var firings1_linked=mktmp('firings1_linked_segment_'+iseg+'.mda');
			var firings2=mktmp('firings2_segment_'+iseg+'.mda');
			var Kmax=mktmp('Kmax_segment_'+iseg+'.mda');

			segments[iseg].firings1_linked=firings1_linked;
			segments[iseg].firings2=firings2;
			segments[iseg].Kmax=Kmax;

			all_segment_firings.push(firings2);

			var seg_prev={};
			if (iseg>0) seg_prev=segments[iseg-1];
			link_segments(firings1,seg_prev.firings1_linked||'',seg_prev.Kmax||'',firings1_linked,firings2,Kmax,segment,seg_prev,function() {
				cb();	
			});
		},function() {
			link_all_segments_callback();
			//done
		});
		function link_segments(firings1,firings2_prev,Kmax_prev,firings_out,firings_subset_out,Kmax,seg,seg_prev,callback) {
			if (firings2_prev) {
				common.mp_exec_process('mountainsort.link_segments',
					{firings:firings1,firings_prev:firings2_prev,Kmax_prev:Kmax_prev},
					{firings_out:firings_out,firings_subset_out:firings_subset_out,Kmax_out:Kmax},
					{t1:seg.t1,t2:seg.t2,t1_prev:seg_prev.t1,t2_prev:seg_prev.t2},
					callback
				);
			}
			else {
				common.copy_file(firings1,firings_out,function() {
					common.copy_file(firings1,firings_subset_out,function() {
						common.make_system_call('mda',['create',Kmax,'--dtype=float64','--size=1x1'],{},function() {
							callback();
						});
					});
				});
			}
		}
	}

	function combine_firings(combine_firings_callback) {
		console.log('>>>>>>>>>>>> Combining '+all_segment_firings.length+' firings...')
		common.mp_exec_process('mountainsort.combine_firings',
				{firings_list:all_segment_firings},
				{firings_out:firings_combined},
				{increment_labels:'false'},
				function() {
					combine_firings_callback();
					//done;
				}
		);
	}

	function write_output_files(write_output_files_callback) {
		common.copy_file(firings_combined,opts.firings_out,function() {
			if ((opts.pre_out)||(opts.filt_out)) {
				var opts2=common.clone(opts);
				opts2.central_channel=0;
				opts2.timeseries=opts.timeseries;
				opts2.firings_out=''; //preprocess only
				opts2.pre_out=opts.pre_out;
				opts2.filt_out=opts.filt_out;
				opts2.console_prefix='Preprocessing for visualization -- ';
				opts2._temp_prefix=(opts._temp_prefix||'00')+'-preprocess';
				opts2._request_num_threads=os.cpus().length; //use all the threads for this part (user has requested something potentially time-consuming)
				if (typeof(opts2.timeseries)=='object') {
					var ts_list=opts2.timeseries;
					opts2.timeseries=mktmp('timeseries_for_preprocessing_output.mda');
					concatenate_timeseries_list(ts_list,opts2.timeseries,function() {
						run_basic_preprocessing();
					});
				}
				else {
					run_basic_preprocessing();
				}
				
				function run_basic_preprocessing() {
					console.log('>>>>>>>>>>>> Running basic preprocessing for output of filt.mda and/or pre.mda...\n');
					basic_sort.run(opts2,function() {
						write_output_files_callback();
						//done
					});
				}
			}
			else {
				cluster_metrics(function() {
					write_output_files_callback();
					//done
				});
			}
		});
	}

	function cluster_metrics(cluster_metrics_callback) {
		////////////////////////////////////////////////////////
		//cluster metrics
		if (opts.cluster_metrics_out) {
			console.log ('>>>>> Cluster metrics -> '+opts.cluster_metrics_out);
			common.mp_exec_process('mountainsort.cluster_metrics',
					{timeseries:opts.timeseries,firings:opts.firings_out},
					{cluster_metrics_out:opts.cluster_metrics_out},
					{samplerate:opts.samplerate},
					function() {
						cluster_metrics_callback();
						//done
					}
			);
		}
		else {
			cluster_metrics_callback();
			//done
		}
	}

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
				freq_wid:opts.freq_wid||0,
				//testcode:'noread,nowrite',
				_request_num_threads:opts.num_intersegment_threads||1
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
				sign:opts.detect_sign,
				subsample_factor:opts.subsample_factor||1
			},
			callback
		);
	}

	function compute_amplitudes(timeseries,event_times,central_channel,amplitudes_out,callback) {
		common.mp_exec_process('mountainsort.compute_amplitudes',
			{timeseries:timeseries,event_times:event_times},
			{amplitudes_out:amplitudes_out},
			{
				central_channel:central_channel
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

	function create_firings(event_times,labels,amplitudes,firings_out,callback) {
		common.mp_exec_process('mountainsort.create_firings',
			{event_times:event_times,labels:labels,amplitudes:amplitudes},
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
	
	
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	

	function mktmp(name) {
		var temp_prefix=opts._temp_prefix||'00';
		var path=opts._tempdir+'/'+temp_prefix+'-'+name;
		tmpfiles.push(path);
		return path;
	}

	function read_info_from_input_files(callback) {
		if (typeof(opts.timeseries)!='object') {
			common.read_mda_header(opts.timeseries,function (header) { // Read the .mda header for the timeseries
				info.M=header.dims[0];
				info.N=header.dims[1];
				callback();
			});
		}
		else {
			get_header_for_concatenation_of_timeseries(opts.timeseries,function(header) {
				info.M=header.dims[0];
				info.N=header.dims[1];
				callback();
			});
		}
	}

	function concatenate_timeseries_list(ts_list,ts_out,callback) {
		get_header_for_concatenation_of_timeseries(ts_list,function(header) {
			var M=header.dims[0];
			var N=header.dims[1];
			extract_segment_timeseries(ts_list,ts_out,0,N-1,function() {
				callback();
				//done
			});
		});	
	}
	function extract_segment_timeseries(ts_in,ts_out,t1,t2,callback) {
		common.mp_exec_process('mountainsort.extract_segment_timeseries',
			{timeseries:ts_in},
			{timeseries_out:ts_out},
			{t1:t1,t2:t2},
			callback
		);
	}
};

function get_elapsed_sec(timer) {
	var stamp=new Date();
	return (stamp-timer)/1000;
}

function create_segments(N,segment_size,shift_size) {
	var ret=[];
	for (var i=0; i<N; i+=shift_size) {
		var t1=i,t2=t1+segment_size-1;
		if (t2>=N) {
			t2=N-1
			t1=t2-segment_size+1;
			if (t1<0) t1=0;
			ret.push({t1:t1,t2:t2});
			break;
		}
		else {
			ret.push({t1:t1,t2:t2});
		}
	}
	return ret;
}

function get_header_for_concatenation_of_timeseries(ts_list,callback) {
	if (ts_list.length==0) {
		console.error('ts_list is empty.');
		process.exit(-1);
	}
	common.read_mda_header(ts_list[0],function(header0) {
		header0.dims[1]=0;
		common.foreach(ts_list,{},function(ii,ts,cb) {
			common.read_mda_header(ts,function(header1) {
				header0.dims[1]+=header1.dims[1];
				cb();
			});
		},function() {
			callback(header0);
		});
	});
	
}
