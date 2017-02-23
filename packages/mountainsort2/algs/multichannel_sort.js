#!/usr/bin/env nodejs

var common=require('./common.js');
var basic_sort=require('./basic_sort');

exports.spec=function() {
	var spec0=basic_sort.spec();
	spec0.inputs.push({name:'geom'});
	spec0.parameters.push({name:'adjacency_radius'});
	return spec0;
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
	if (!opts.adjacency_radius) {
		console.error('opts.adjacency_radius is zero or empty');
		process.exit(-1);		
	}

	var info={};
	read_info_from_input_files(function() {
		var nbhd_steps=[];
		for (var ch=1; ch<=info.M; ch++) {
			add_neighborhood_step(ch);
		}
		//Run all steps
		common.foreach(nbhd_steps,{},function(ii,step,cb) {
			console.log('');
			console.log('--------------------------- NEIGHBORHOOD '+(ii+1)+' of '+steps.length +' -----------');
			var timer=new Date();
			step(function() {
				console.log ('Elapsed time (sec): '+get_elapsed_sec(timer));
				cb();
			});
		},function() {
			callback();
		});
		function add_neighborhood_processing(ch) {
			nbhd_steps.push(function(cb) {
				var channels=get_neighborhood_channels(info.geom_coords,ch,opts.adjacency_radius);
				var timeseries0=mktmp('timeseries_ch'+ch+'.mda');
				var firings0=mktmp('firings_ch'+ch+'.mda');
				extract_neighborhood_timeseries(opts.timeseries,ts0,channels,function() {
					var opts2=common.clone(opts);
					opts2.timeseries=timeseries0;
					opts2.firings_out=firings0;
					opts2.pre_out=null;
					opts2.filt_out=null;
					basic_sort(opts2,function() {
						cb();
					});
				});
			});
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
		var txt=read_text_file(geom);
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