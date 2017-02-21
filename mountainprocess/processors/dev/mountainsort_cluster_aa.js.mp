#!/usr/bin/env nodejs

function get_spec() {
	var processors=[];
	processors.push({
	    name: "mountainsort_cluster_aa",
	    version: "0.1",
	    exe_command: __filename+' mountainsort_cluster_aa $(arguments)', //todo: implement $(processor_name)
	    description: "",
	    inputs: [
	        {name: "timeseries",description:"preprocessed timeseries (M x N)"},
	        {name: "detect",description:"detected events (2 x L) -- first row is channel number, second row is timestamp"},
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
var arg1=CLP.unnamedParameters[0]||''; // name of processor
var request_num_threads=CLP.namedParameters['_request_num_threads']||os.cpus().length;
var tempdir=CLP.namedParameters['_tempdir']||''; // the working directory for this processor
if (arg1=='spec') {
	// display the spec for this processor library
	var spec=get_spec();
	console.log (JSON.stringify(spec));
}
else if (arg1=='mountainsort_cluster_aa') {
	if (!tempdir) {
		console.log ('Missing required system parameter: _tempdir');
		process.exit(-1);
	}

	mountainsort_cluster_aa(CLP.namedParameters);
}
else {
	console.log ('Unexpected processor name: '+arg1);
	process.exit(-1);
}

function mountainsort_cluster_aa(params) {
	var timeseries=params.timeseries||'';
	var detect=params.detect||'';
	var geom=params.geom||'';
	var firings_out=params.firings_out||'';
	if (!timeseries) {
		console.log ('timeseries is empty.');
		process.exit(-1);
	}
	if (!detect) {
		console.log ('detect is empty.');
		process.exit(-1);
	}
	if (!geom) {
		console.log ('geom is empty.');
		process.exit(-1);
	}
	if (!firings_out) {
		console.log ('firings_out is empty.');
		process.exit(-1);
	}

	var chunk_size=30000*60*10;
	var chunk_shift=Math.ceil(chunk_size/2);
	var clip_size=50;
	
	read_mda_header(timeseries,function (header) { // Read the .mda header for the timeseries
		do_sorting(header.dims[0],header.dims[1]);
	});

	function do_sorting(M,N) {
		console.log ('Num. channels = '+M);
		console.log ('Num. timepoints = '+N);
		var time_chunks=get_time_chunks(N,{chunk_size:chunk_size,chunk_shift:chunk_shift});
		console.log ('Num. time chunks = '+time_chunks.length);
		var channels=[];
		for (var m=0; m<M; m++) channels.push(m);
		var channel_firings=[];
		foreach(channels,{num_parallel:1},function(ichan,chan,cb) { //loop through the channels
			console.log ('Sorting on channel '+(chan+1)+' of '+M+'...');
			sort_on_channel(M,N,time_chunks,chan,function(firings_files) {
				var linked_firings_path=tempdir+'/linked_firings_'+(chan+1)+'.mda';
				link_firings_files(firings_files,linked_firings_path,function() {
					channel_firings.push(linked_firings_path);
					cb();
				});
			});
		},function() {
			combine_firings_files(channel_firings,firings_out,function() {
				cleanup();
			});
		});
	}

	function sort_on_channel(M,N,time_chunks,chan,cb) {
		var neighborhood_channels=get_neighborhood_channels(geom,chan,params.adjacency_radius);
		var firings_files=[];
		foreach(time_chunks,{num_parallel:request_num_threads},function(ichunk,time_chunk,cb2) {
			console.log ('Clustering time chunk '+(ichunk+1)+' of '+time_chunks.length+', channel '+(chan+1)+' of '+M+' ['+time_chunk.t1+','+time_chunk.t2+']...');
			var paths={
				clips:tempdir+'/clips_'+chan+'_'+ichunk+'.mda',
				firings:tempdir+'/firings_'+chan+'_'+ichunk+'.mda',
				detect2:tempdir+'/detect2_'+chan+'_'+ichunk+'.mda'
			};
			console.log('chan='+chan);
			mp_exec_process('extract_clips_aa',
					{timeseries:timeseries,detect:detect},
					{clips_out:paths.clips,detect_out:paths.detect2},
					{central_channel:(chan+1),channels:neighborhood_channels.join(','),t1:time_chunk.t1,t2:time_chunk.t2,clip_size:clip_size},
					function() {
				mp_exec_process('cluster_aa',
						{clips:paths.clips,detect:paths.detect2},
						{firings_out:paths.firings},
						{num_features:10,num_features2:10},
						function() {
					firings_files.push(paths.firings);
					remove_temporary_files([paths.clips,paths.detect2]);
					cb2();
				});
				
			});
		},function() {
			cb(firings_files);
		});
	}
}

function link_firings_files(firings_files,linked_firings_path,callback) {
	mp_exec_process('link_firings_files_aa',
			{firings_list:firings_files},
			{firings_out:linked_firings_path},
			{},
			function() {
		callback();
	});
}

function combine_firings_files(firings_files,combined_firings_path,callback) {
	mp_exec_process('combine_firings_files_aa',
			{firings_list:firings_files},
			{firings_out:combined_firings_path},
			{},
			function() {
		callback();
	});
}

function read_geom_coords(geom) {
	var ret=[];
	var txt=read_text_file(geom);
	var lines=txt.split('\n');
	for (var i in lines) {
		var line=lines[i].trim();
		if (line) {
			var vals=lines[i].split(',');
			if (vals.length>0) {
				var pt=[];
				for (var j in vals) {
					pt.push(Number(vals[j].trim()));
				}
				ret.push(pt);
			}
		}
	}
	return ret;
}

function compute_distance(pt1,pt2) {
	var sumsqr=0;
	for (var i in pt1) {
		sumsqr=sumsqr+(pt1[i]-pt2[i])*(pt1[i]-pt2[i]);
	}
	return Math.sqrt(sumsqr);
}

function get_neighborhood_channels(geom,m,adjacency_radius) {
	var coords=read_geom_coords(geom);
	var pt0=coords[m];
	var ret=[Number(m)+1];
	for (var i in coords) {
		if (i!=m) {
			var dist0=compute_distance(coords[i],pt0);
			if (dist0<=adjacency_radius)
				ret.push(Number(i)+1);
		}
	}
	return ret;
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

function cleanup() {

}

function create_array(path,M,N,callback) {
	var exe='mda';
	var args=['create',path,'--size='+M+'x'+N,'--dtype=float32'];
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:1},function(aa) {
		if (aa.return_code!=0) {
			console.log ('Returned non-zero exit code: '+exe+' '+args.join(' '));
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}

/*
function get_chunk(path,chunk_path,M,t1,t2,callback) {
	var exe='mda';
	var args=['get_chunk',path,chunk_path,'--index=0,'+t1,'--size='+M+'x'+(t2-t1+1)];
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:5},function(aa) {
		if (aa.return_code!=0) {
			console.log ('Returned non-zero exit code: '+exe+' '+args.join(' '));
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}
*/

/*
function set_chunk(path,chunk_path,M,t1,t2,callback) {
	var exe='mda';
	var args=['set_chunk',path,chunk_path,'--index=0,'+t1];
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:5},function(aa) {
		if (aa.return_code!=0) {
			console.log ('Returned non-zero exit code: '+exe+' '+args.join(' '));
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}
*/

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
		else {
			for (var j in val)
				args.push('--'+key+'='+val[j]);
		}
	}
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:4},function(aa) {
		if (aa.return_code!=0) {
			console.log('Subprocess '+processor_name+' returned with a non-zero exit code.');
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}

var fs=require('fs');
function mkdir(path) {
	try {
		fs.mkdirSync(path);
	}
	catch(err) {
		//maybe already exists. Don't want to abort
	}
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

function get_time_chunks(N,opts) {
	var ret=[];
	for (var t1a=0; t1a<N; t1a+=opts.chunk_shift) {
		var t1=t1a;
		var t2=t1+opts.chunk_size-1;
		if (t2>=N) {
			t2=N-1;
			t1=t2-opts.chunk_shift+1;
			if (t1<0) t1=0;
		}
		ret.push({t1:t1,t2:t2});
	}
	return ret;
}

function read_mda_header(path,callback) {
	var exe='mdaconvert';
	var args=[path,'--readheader'];
	make_system_call(exe,args,{num_tries:5},function(aa) {
		callback(JSON.parse(aa.stdout));
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

function make_system_call(cmd,args,opts,callback) {
	var num_tries=opts.num_tries||1;
	var child_process=require('child_process');
	console.log('Calling: '+cmd+' '+args.join(' '));
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
		console.log ('Process error: '+cmd+' '+args.join(' '));
		console.log (err);
	});
	var all_stdout='';
	var all_stderr='';
	pp.stdout.on('data',function(data) {
		if (opts.show_stdout) {
			console.log(data);
		}
		all_stdout+=data;
	});
	pp.stderr.on('data',function(data) {
		if (opts.show_stderr) {
			console.log(data);
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
				console.log('Error in system call: '+cmd+' '+args.join(' '));
				process.exit(-1);
			}
		}
  		done=true;
		if (callback) {
			callback({stdout:all_stdout,stderr:all_stderr,return_code:code});
		}
	}
}


function make_random_id(len)
{
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for( var i=0; i < len; i++ )
        text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
}