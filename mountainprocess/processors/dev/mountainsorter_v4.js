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
params.clip_size_msec=1;
params.detect_threshold=3.5;
params.detect_interval_msec=1;

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
		for (var i=0; i<neighborhoods.length; i++) {
			firings_files.push(neighborhoods[i].firings());
		}
		combine_firings(firings_files,outp.firings_out,function() {
			remove_temporary_files(firings_files);
			callback();
		});
	}
}

function ElectrodeNeighborhood(chan) {
	var channels=get_neighborhood_channels(inp.geom_coords,chan,params.adjacency_radius);
	this.sort=function() {
		var tmpfiles={
			X:tempdir+'/timeseries_'+chan+'.mda',
			event_times:tempdir+'/event_times_'+chan+'.mda',
			clips:tempdir+'/clips_'+chan+'.mda',
			labels:tempdir+'/labels_'+chan+'.mda',
			labels2:tempdir+'/labels2_'+chan+'.mda',
			firings:tempdir+'/firings_'+chan+'.mda',
		};
		extract_neighborhood_timeseries(inp.timeseries,tmpfiles.X,channels,function() {
			detect_events(tmpfiles.X,tmpfiles.event_times,function() {
				extract_clips(tmpfiles.X,tmpfiles.event_times,tmpfiles.clips,function() {
					sort_clips(tmpfiles.clips,tmpfiles.labels,function() {
						consolidate_clusters(tmpfiles.clips,tmpfiles.labels,tmpfiles.labels2,function() {
							create_firings(tmpfiles.event_times,tmpfiles.labels2,tmpfiles.firings,function() {
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

function detect_events(timeseries,event_times_out,callback) {
	var detect_interval=Math.ceil(detect_interval_msec/1000*params.samplerate);
	mp_exec_process('mountainsort.detect_events',
		{timeseries:timeseries},
		{detect_out:event_times_out},
		{
			central_channel:1, //in the neighborhood, the central channel is always the first
			detect_threshold:params.detect_threshold,
			detect_interval:detect_interval
		},
		callback
	);
}

function extract_clips(timeseries,detect,clips_out,callback) {
	var clip_size=Math.ceil(clip_size_msec/1000*params.samplerate);
	mp_exec_process('mountainsort.extract_clips',
		{timeseries:timeseries,detect:detect},
		{clips_out:clips_out},
		{
			clip_size:clip_size
		},
		callback
	);	
}

function sort_clips(clips,labels_out,callback) {
	mp_exec_process('mountainsort.sort_clips',
		{clips:clips},
		{labels_out:labels_out},
		{
		},
		callback
	);	
}

function consolidate_clusters(clips,labels,labels_out,callback) {
	mp_exec_process('mountainsort.consolidate_clusters',
		{clips:clips,labels:labels},
		{labels_out:labels_out},
		{
			central_channel:1 //in the neighborhood, the central channel is always the first
		},
		callback
	);
}

function create_firings(event_times,labels,firings_out,channel,callback) {
	mp_exec_process('mountainsort.create_firings',
		{event_times:event_times,labels:labels},
		{firings_out:firings_out},
		{
			channel:channel
		},
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

function read_text_file(path) {
	return fs.readFileSync(path,'utf8');
}

function write_text_file(path,txt) {
	return fs.writeFileSync(path,txt,'utf8');
}

function remove_temporary_files(paths) {
	for (var i in paths)
		remove_temporary_file(paths[i]);
}

function remove_temporary_file(path) {
	fs.unlink(path);
}

function foreach(array,opts,step_function,end_function) {
	var num_parallel=opts.num_parallel||1;
	var num_running=0;
	var ii=0;
	next_step();
	function next_step() {
		if (ii>=array.length) {
			setTimeout(function() { //important to do it this way so we don't accumulate a call stack
				end_function();
			});
			return;
		}
		while ((ii<array.length)&&(num_running<num_parallel)) {
			num_running++;
			ii++;
			step_function(ii-1,array[ii-1],function() {
				num_running--;
				setTimeout(function() { //important to do it this way so we don't accumulate a call stack
					next_step();
				},0);
			});
		}
	}
}

function mp_exec_process(processor_name,inputs,outputs,params,callback) {
	var exe='mountainprocess';
	var args=['exec-process',processor_name];
	var all_params=[];
	for (var key in inputs) {
		all_params[key]=inputs[key];
	}
	for (var key in outputs) {
		all_params[key]=outputs[key];
	}
	for (var key in params) {
		all_params[key]=params[key];
	}
	for (var key in all_params) {
		var val=all_params[key];
		if (typeof(val)!='object')
			args.push('--'+key+'='+val);
		else { // a list of values
			for (var j in val)
				args.push('--'+key+'='+val[j]);
		}
	}
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:4},function(aa) {
		if (aa.return_code!=0) {
			console.error('Subprocess '+processor_name+' returned with a non-zero exit code.');
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}

function CLParams(argv) {
	this.unnamedParameters=[];
	this.namedParameters={};

	var args=argv.slice(2);
	for (var i=0; i<args.length; i++) {
		var arg0=args[i];
		if (arg0.indexOf('--')===0) {
			arg0=arg0.slice(2);
			var ind=arg0.indexOf('=');
			if (ind>=0) {
				this.namedParameters[arg0.slice(0,ind)]=arg0.slice(ind+1);
			}
			else {
				this.namedParameters[arg0]=args[i+1]||'';
				i++;
			}
		}
		else if (arg0.indexOf('-')===0) {
			arg0=arg0.slice(1);
			this.namedParameters[arg0]='';
		}
		else {
			this.unnamedParameters.push(arg0);
		}
	}
};

function read_mda_header(path,callback) {
	var exe='mdaconvert';
	var args=[path,'--readheader'];
	make_system_call(exe,args,{num_tries:5},function(aa) {
		callback(JSON.parse(aa.stdout));
	});
}

function make_system_call(cmd,args,opts,callback) {
	var num_tries=opts.num_tries||1;
	var child_process=require('child_process');
	console.log ('Calling: '+cmd+' '+args.join(' '));
	var pp=child_process.spawn(cmd,args);
	pp.stdout.setEncoding('utf8');
	pp.stderr.setEncoding('utf8');
	var done=false;
	pp.on('close', function(code) {
		return_it(code);
	});
	//pp.on('exit', function(code) {
	//	return_it();
	//});
	pp.on('error',function(err) {
		console.error('Process error: '+cmd+' '+args.join(' '));
		console.error(err);
	});
	var all_stdout='';
	var all_stderr='';
	pp.stdout.on('data',function(data) {
		if (opts.show_stdout) {
			console.log (data);
		}
		all_stdout+=data;
	});
	pp.stderr.on('data',function(data) {
		if (opts.show_stderr) {
			console.log (data);
		}
		all_stderr+=data;
	});
	function return_it(code) {
		if (done) return;
		if (code!=0) {
			if (num_tries>1) {
				console.error ('Re-trying system call: '+cmd+' '+args.join(' '));
				opts.num_tries=opts.num_tries-1; //todo, I really should not manipulate the opts here. very bad idea
				make_system_call(cmd,args,opts,callback);
				return;
			}
			else {
				console.log('***************\n'+all_stderr+'****************\n');
				console.error('Error in system call: '+cmd+' '+args.join(' '));
				process.exit(-1);
			}
		}
  		done=true;
		if (callback) {
			callback({stdout:all_stdout,stderr:all_stderr,return_code:code});
		}
	}
}
