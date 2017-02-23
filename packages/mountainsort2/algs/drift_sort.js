#!/usr/bin/env nodejs

var common=require('./common.js');
var basic_sort=require('./basic_sort');

exports.spec=function() {
	var spec0=basic_sort.spec();
	spec0.name='mountainsort.drift_sort';
	spec0.parameters.push({name:'segment_duration_sec'});
	spec0.parameters.push({name:'shift_duration_sec'});
	return common.clone(spec0);
};

exports.run=function(opts,callback) {
	var tmpfiles=[];
	opts.temp_prefix=opts.temp_prefix||'00';
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

	var info={};
	read_info_from_input_files(function() {
		var segment_steps=[];
		var all_segment_firings=[];
		var segment_duration=Math.ceil(opts.segment_duration_sec*opts.samplerate);
		var shift_duration=Math.ceil(opts.shift_duration_sec*opts.samplerate);
		var segments=create_segments(info.N,segment_duration,shift_duration);
		for (var iseg=0; iseg<segments.length; iseg++) {
			add_segment_step(iseg);
		}
		function add_segment_step(iseg) {
			var segment=segments[iseg];
			segment_steps.push(function(cb) {
				var timeseries0=mktmp('timeseries_segment_'+iseg+'.mda');
				var firings0=mktmp('firings_segment_'+iseg+'.mda');
				var firings1=mktmp('firings1_segment_'+iseg+'.mda'); //timestamp offset
				var firings2=mktmp('firings2_segment_'+iseg+'.mda'); //linked!!
				var Kmax=mktmp('Kmax_segment_'+iseg+'.mda'); //linked!!
				segments[iseg].firings1=firings1;
				segments[iseg].firings2=firings2;
				segments[iseg].Kmax=Kmax;
				all_segment_firings.push(firings2);
				console.log('>>>>>>>>>>>> Extracting timeseries for segment '+iseg+' ('+segment.t1+','+segment.t2+')...\n');
				extract_segment_timeseries(opts.timeseries,timeseries0,segment.t1,segment.t2,function() {
					var opts2=common.clone(opts);
					opts2.timeseries=timeseries0;
					opts2.firings_out=firings0;
					opts2.pre_out=null;
					opts2.filt_out=null;
					opts2._temp_prefix=(opts._temp_prefix||'00')+'-segment_'+iseg;
					console.log('>>>>>>>>>>>> Running basic sort for segment '+iseg+'...\n');
					basic_sort.run(opts2,function() {
						apply_timestamp_offset(firings0,firings1,segment.t1,function() {
							var seg_prev={};
							if (iseg>0) seg_prev=segments[iseg-1];
							link_segments(firings1,seg_prev.firings2||'',seg_prev.Kmax||'',firings2,Kmax,segment,seg_prev,function() {
								cb();	
							});
						});
					});
				});
			});
		}
		//Run all steps
		common.foreach(segment_steps,{},function(ii,step,cb) {
			console.log('');
			console.log('--------------------------- SEGMENT '+(ii+1)+' of '+segment_steps.length +' -----------');
			var timer=new Date();
			step(function() {
				console.log ('Elapsed time (sec): '+get_elapsed_sec(timer));
				cb();
			});
		},function() {
			combine_firings();
		});

		function extract_segment_timeseries(ts_in,ts_out,t1,t2,callback) {
			common.mp_exec_process('mountainsort.extract_segment_timeseries',
				{timeseries:ts_in},
				{timeseries_out:ts_out},
				{t1:t1,t2:t2},
				callback
			);
		}

		function apply_timestamp_offset(firings,firings_out,timestamp_offset,callback) {
			common.mp_exec_process('mountainsort.apply_timestamp_offset',
				{firings:firings},
				{firings_out:firings_out},
				{timestamp_offset:timestamp_offset},
				callback
			);
		}

		function link_segments(firings1,firings2_prev,Kmax_prev,firings2,Kmax,seg,seg_prev,callback) {
			if (firings2_prev) {
				common.mp_exec_process('mountainsort.link_segments',
					{firings:firings1,firings_prev:firings2_prev,Kmax_prev:Kmax_prev},
					{firings_out:firings2,Kmax_out:Kmax},
					{t1:seg.t1,t2:seg.t2,t1_prev:seg_prev.t1,t2_prev:seg_prev.t2},
					callback
				);
			}
			else {
				common.copy_file(firings1,firings2,function() {
					common.make_system_call('mda',['create',Kmax,'--dtype=float64','--size=1x1'],{},function() {
						callback();
					});
				});
			}
			
		}

		function combine_firings() {
			var firings_combined=mktmp('firings_combined.mda');
			console.log('>>>>>>>>>>>> Combining '+all_segment_firings.length+' firings...')
			common.mp_exec_process('mountainsort.combine_firings',
					{firings_list:all_segment_firings},
					{firings_out:firings_combined},
					{increment_labels:'false'},
					function() {
						common.copy_file(firings_combined,opts.firings_out,function() {
							cleanup(function() {
								if ((opts.pre_out)||(opts.filt_out)) {
									var opts2=common.clone(opts);
									opts2.central_channel=0;
									opts2.timeseries=opts.timeseries;
									opts2.firings_out=''; //preprocess only
									opts2.pre_out=opts.pre_out;
									opts2.filt_out=opts.filt_out;
									opts2._temp_prefix=(opts._temp_prefix||'00')+'-preprocess';
									console.log('>>>>>>>>>>>> Running basic preprocessing for output of filt.mda and/or pre.mda...\n');
									basic_sort.run(opts2,function() {
										//done
									});
								}
								else {
									//done
								}
							});
						});
					}
			)	
		}

	});
	
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	function cleanup(callback) {
		common.remove_temporary_files(tmpfiles,callback);
	}

	function mktmp(name) {
		var temp_prefix=opts._temp_prefix||'00';
		var path=opts._tempdir+'/'+temp_prefix+'-'+name;
		tmpfiles.push(path);
		return path;
	}

	function read_info_from_input_files(callback) {
		common.read_mda_header(opts.timeseries,function (header) { // Read the .mda header for the timeseries
			info.M=header.dims[0];
			info.N=header.dims[1];
			callback();
		});
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
		}
		ret.push({t1:t1,t2:t2});
	}
	return ret;
}
