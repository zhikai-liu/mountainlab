#!/usr/bin/env nodejs

function get_spec() {
	var processors=[];
	processors.push({
	    name: "mountainsorter_v4",
	    version: "0.1",
	    exe_command: __filename+' $(arguments)',
	    description: "",
	    inputs: [
	        {name: "timeseries",description:"preprocessed timeseries (M x N)"},
	        {name: "geom",description:"geom.csv file -- electrode geometry"}
	    ],
	    outputs: [
	        {name: "firings_out",description:"The labeled events (3 x L)"}
	    ],
	    parameters: [
	        {name: "samplerate",optional: false,description:"sample rate for timeseries"},
	        {name: "adjacency_radius",optional: false,description:"corresponds to geom; determines electrode neighborhoods"}
	    ]
	});
	return {processors:processors};
}

var keep_temporary_files=true;

var fs=require('fs');
var os=require('os');

var CLP=new CLParams(process.argv);
var arg1=CLP.unnamedParameters[0]||''; // name of processor or "spec"
if (arg1=='spec') {
	// display the spec for this processor library
	var spec=get_spec();
	console.log (JSON.stringify(spec,null,'  '));
	return;
}

var request_num_threads=CLP.namedParameters['_request_num_threads']||os.cpus().length;
var tempdir=CLP.namedParameters['_tempdir']||''; // the working directory for this processor
if (!tempdir) {
	console.error('_tempdir is empty');
	process.exit(-1);
}

// set and check inputs, outputs, and params
var inp={};
inp.timeseries=CLP.namedParameters.timeseries||'';
inp.geom=CLP.namedParameters.geom||'';
var outp={};
outp.firings_out=CLP.namedParameters.firings_out||'';
var params={};
params.samplerate=CLP.namedParameters.samplerate||0;
params.adjacency_radius=CLP.namedParameters.adjacency_radius||0;
check_inputs_outputs(inp,outp,params);

//params.chunk_size=30000*60*10;
//params.chunk_shift=Math.ceil(params.chunk_size/2);
params.clip_size_msec=2.0;
params.detect_threshold=3.5;
params.detect_interval_msec=1;
params.detect_sign=0;

read_info_from_input_files(function() {
	sort(function() {
		//done sorting
	});
});

function sort(callback) {
	var neighborhoods=[];
	for (var m=1; m<=inp.M; m++) {
		var X=new ElectrodeNeighborhood(m);
		neighborhoods.push(X);
	}
	foreach(neighborhoods,{},function(index,neighborhood,cb) {
		console.log ('Sorting neighborhood '+(index+1)+' of '+neighborhoods.length+'...');
		neighborhood.sort(function() {
			cb();
		});
	},function() {
		//done with all neighborhoods
		combine_neighborhoods();
	});

	function combine_neighborhoods() {
		var firings_files=[];
		for (var i=0; i-<neighborhoods.length; i++) {
			firings_files.push(neighborhoods[i].firings());
		}
		var firings_combined=tempdir+'/firings_combined.mda';
		combine_firings(firings_files,firings_combined,function() {
			remove_temporary_files(firings_files);
			fit_stage(firings_combined,outp.firings_out,function() {
				callback();
			});
		});
	}
}

function ElectrodeNeighborhood(chan) {
	var channels=get_neighborhood_channels(inp.geom_coords,chan,params.adjacency_radius);
	var tmpfiles={
		X:tempdir+'/timeseries_'+chan+'.mda',
		event_times:tempdir+'/event_times_'+chan+'.mda',
		clips:tempdir+'/clips_'+chan+'.mda',
		labels:tempdir+'/labels_'+chan+'.mda',
		labels2:tempdir+'/labels2_'+chan+'.mda',
		firings:tempdir+'/firings_'+chan+'.mda',
	};
	this.sort=function(callback) {
		extract_neighborhood_timeseries(inp.timeseries,tmpfiles.X,channels,function() {
			detect_events(tmpfiles.X,tmpfiles.event_times,function() {
				extract_clips(tmpfiles.X,tmpfiles.event_times,tmpfiles.clips,function() {
					sort_clips(tmpfiles.clips,tmpfiles.labels,function() {
						consolidate_clusters(tmpfiles.clips,tmpfiles.labels,tmpfiles.labels2,function() {
							create_firings(tmpfiles.event_times,tmpfiles.labels2,tmpfiles.firings,chan,function() {
								cleanup();
								callback();
							});
						});
					});
				});
			});
		});
	}
	this.firings=function() {
		return tmpfiles.firings;
	}
	function cleanup() {
		remove_temporary_files([
			tmpfiles.X,
			tmpfiles.event_times,
			tmpfiles.clips,
			tmpfiles.labels,
			tmpfiles.labels2
		]);
	}
}

function extract_neighborhood_timeseries(ts_in,ts_out,channels,callback) {
	mp_exec_process('mountainsort.extract_neighborhood_timeseries',
		{timeseries:ts_in},
		{timeseries_out:ts_out},
		{channels:channels.join(',')},
		callback
	);
}



function combine_firings(firings_list,firings_out,callback) {
	mp_exec_process('mountainsort.combine_firings',
		{firings_list:firings_list},
		{firings_out:firings_out},
		{
		},
		callback
	);
}

function fit_stage(firings,firings_out,callback) {
	mp_exec_process('mountainsort.fit_stage',
		{timeseries:inp.timeseries,firings:firings},
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


function read_info_from_input_files(callback) {
	read_mda_header(inp.timeseries,function (header) { // Read the .mda header for the timeseries
		inp.M=header.dims[0];
		inp.N=header.dims[1];
		inp.geom_coords=read_geom_coords(inp.geom);
		if (inp.geom_coords.length!=inp.M) {
			console.error('Incompatible number of electrodes in geom file: '+inp.geom_coords.length+' <> '+inp.M);
			process.exit(-1);
		}
		callback();
	});
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


function check_inputs_outputs(inp,outp,params) {
	if (!inp.timeseries) {
		console.error('timeseries is empty.');
		process.exit(-1);
	}
	if (!inp.geom) {
		console.error('geom is empty.');
		process.exit(-1);
	}
	if (!outp.firings_out) {
		console.error('firings_out is empty.');
		process.exit(-1);
	}
	if (!params.samplerate) {
		console.error('samplerate is empty or zero.');
		process.exit(-1);	
	}
	if (!params.adjacency_radius) {
		console.error('adjacency_radius is empty or zero.');
		process.exit(-1);		
	}
}

