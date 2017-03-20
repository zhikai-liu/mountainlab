#!/usr/bin/env nodejs

var common=require('./common.js');
var basic_sort=require('./basic_sort');
var os=require('os');

exports.spec=function() {
	var spec0=basic_sort.spec();
	spec0.name='mountainsort.drift_sort';
	spec0.version='0.1--'+spec0.version;
	spec0.parameters.push({name:'segment_duration_sec'});
	spec0.parameters.push({name:'shift_duration_sec'});
	spec0.parameters.push({name:'num_time_segment_threads',optional:true});
	spec0.parameters.push({name:'num_basic_sort_threads',optional:true});
	return common.clone(spec0);
};

exports.run=function(opts,callback) {
	var tmpfiles=[];
	opts.temp_prefix=opts.temp_prefix||'00';
	opts.num_time_segment_threads=Number(opts.num_time_segment_threads||1);
	opts.num_basic_sort_threads=opts.num_basic_sort_threads||0;
	if (!opts.num_basic_sort_threads) {
		// determine whether to use multi-threading on the basic sorts
		// depending on whether we are doing time segment threads
		if (opts.num_time_segment_threads<=1)
			opts.num_basic_sort_threads=os.cpus().length;
		else
			opts.num_basic_sort_threads=1;
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
	if (!opts.shift_duration_sec) {
		console.error('opts.shift_duration_sec is zero or empty');
		process.exit(-1);		
	}

	var steps=[];
	var info={};
	var segments=[];
	var all_segment_firings=[];
	var firings_combined=mktmp('firings_combined.mda');

	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		read_info_from_input_files(function() {
			cb();
		});		
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		sort_segments(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		link_all_segments(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		combine_firings(function() {
			cb();
		});
	});
	///////////////////////////////////////////////////////////////
	steps.push(function(cb) {
		write_output_files(function() {
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
					opts2._request_num_threads=opts.num_basic_sort_threads;
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
		common.foreach(segment_steps,{num_parallel:opts.num_time_segment_threads},function(ii,step,cb) {
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
