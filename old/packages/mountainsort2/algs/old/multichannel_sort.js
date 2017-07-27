#!/usr/bin/env nodejs

var common=require('./common.js');
var basic_sort=require('./basic_sort');

exports.spec=function() {
	var spec0=basic_sort.spec();
	spec0.name='mountainsort.multichannel_sort';
	spec0.inputs.push({name:'geom'});
	spec0.parameters.push({name:'adjacency_radius'});
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
	if (!opts.adjacency_radius) {
		console.error('opts.adjacency_radius is zero or empty');
		process.exit(-1);		
	}

	var info={};
	read_info_from_input_files(function() {
		var nbhd_steps=[];
		var all_nbhd_firings=[];
		for (var ch=1; ch<=info.M; ch++) {
			add_neighborhood_step(ch);
		}
		function add_neighborhood_step(ch) {
			nbhd_steps.push(function(cb) {
				var channels=get_neighborhood_channels(info.geom_coords,ch,opts.adjacency_radius);
				var timeseries0=mktmp('timeseries_nbhd'+ch+'.mda');
				var firings0=mktmp('firings_nbhd'+ch+'.mda');
				all_nbhd_firings.push(firings0);
				console.log('Extracting neighborhood timeseries for electrode '+ch+'...\n');
				extract_neighborhood_timeseries(opts.timeseries,timeseries0,channels,function() {
					var opts2=common.clone(opts);
					opts2.central_channel=1; //in the neighborhood, the central channel is always the first
					opts2.timeseries=timeseries0;
					opts2.firings_out=firings0;
					opts2.pre_out=null;
					opts2.filt_out=null;
					opts2._temp_prefix=(opts._temp_prefix||'00')+'-nbhd'+ch;
					console.log('Running basic sort for electrode '+ch+'...\n');
					basic_sort.run(opts2,function() {
						cb();
					});
				});
			});
		}
		//Run all steps
		common.foreach(nbhd_steps,{},function(ii,step,cb) {
			console.log('');
			console.log('--------------------------- NEIGHBORHOOD '+(ii+1)+' of '+nbhd_steps.length +' -----------');
			var timer=new Date();
			step(function() {
				console.log ('Elapsed time (sec): '+get_elapsed_sec(timer));
				cb();
			});
		},function() {
			combine_firings();
		});

		function extract_neighborhood_timeseries(ts_in,ts_out,channels,callback) {
			common.mp_exec_process('mountainsort.extract_neighborhood_timeseries',
				{timeseries:ts_in},
				{timeseries_out:ts_out},
				{channels:channels.join(',')},
				callback
			);
		}

		function combine_firings() {
			var firings_combined=mktmp('firings_combined.mda');
			common.mp_exec_process('mountainsort.combine_firings',
					{firings_list:all_nbhd_firings},
					{firings_out:firings_combined},
					{},
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
									console.log('Running basic preprocessing for output of filt.mda and/or pre.mda...\n');
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

	function read_info_from_input_files(callback) {
		common.read_mda_header(opts.timeseries,function (header) { // Read the .mda header for the timeseries
			info.M=header.dims[0];
			info.N=header.dims[1];
			info.geom_coords=read_geom_coords(opts.geom);
			if (info.geom_coords.length!=info.M) {
				console.error('Incompatible number of electrodes in geom file: '+info.geom_coords.length+' <> '+info.M);
				process.exit(-1);
			}
			callback();
		});
	}
};

function get_elapsed_sec(timer) {
	var stamp=new Date();
	return (stamp-timer)/1000;
}

function get_neighborhood_channels(coords,chan,adjacency_radius) {
	var pt0=coords[chan-1];
	var ret=[chan];
	for (var i=0; i<coords.length; i++) {
		if (i+1!=chan) {
			var dist0=compute_distance(coords[i],pt0);
			if (dist0<=adjacency_radius)
				ret.push(i+1);
		}
	}
	return ret;
}

function compute_distance(pt1,pt2) {
	var sumsqr=0;
	for (var i=0; i<pt1.length; i++) {
		sumsqr=sumsqr+(pt1[i]-pt2[i])*(pt1[i]-pt2[i]);
	}
	return Math.sqrt(sumsqr);
}

function read_geom_coords(geom) {
	var ret=[];
	var txt=common.read_text_file(geom);
	var lines=txt.split('\n');
	for (var i=0; i<lines.length; i++) {
		var line=lines[i].trim();
		if (line) {
			var vals=lines[i].split(',');
			if (vals.length>0) {
				var pt=[];
				for (var j=0; j<vals.length; j++) {
					pt.push(Number(vals[j].trim()));
				}
				ret.push(pt);
			}
		}
	}
	return ret;
}