#!/usr/bin/env nodejs

function get_spec() {
	var processors=[];
	processors.push({
	    name: "bandpass_filter_aa",
	    version: "0.1",
	    exe_command: __filename+' bandpass_filter_aa $(arguments)', //todo: implement $(processor_name)
	    description: "",
	    inputs: [
	        {name: "timeseries"}
	    ],
	    outputs: [
	        {name: "timeseries_out"}
	    ],
	    parameters: [
	        {name: "samplerate",optional: false},
	        {name: "freq_min",optional: false},
	        {name: "freq_max",optional: false},
	        {name: "freq_wid",optional: true}
	    ]
	});
	return {processors:processors};
}

var CLP=new CLParams(process.argv);
arg1=CLP.unnamedParameters[0]||'';
var request_num_threads=CLP.namedParameters['_request_num_threads']||1;
if (arg1=='spec') {
	var spec=get_spec();
	console.log (JSON.stringify(spec));
}
else if (arg1=='bandpass_filter_aa') {
	bandpass_filter_aa(CLP.namedParameters);
}

function bandpass_filter_aa(params) {
	var timeseries=params.timeseries;
	var timeseries_out=params.timeseries_out;
	read_mda_header(timeseries,function (header) {
		var M=header.dims[0]; //number of channels
		var N=header.dims[1]; //number of timepoints
		create_array(timeseries_out,M,N,function() {
			var time_chunks=get_time_chunks(M,N,{chunk_size:1000000,overlap_pct:0.1});
			foreach(time_chunks,{num_parallel:request_num_threads},function(ii,time_chunk,cb) {
				console.log ('Processing chunk '+(ii+1)+' of '+time_chunks.length+' ('+Math.floor(100*ii/time_chunks.length)+'%)'+'('+time_chunk.t1+','+time_chunk.t2+') ('+time_chunk.t1_minus_overlap+','+time_chunk.t2_plus_overlap+')');
				process_chunk(time_chunk,function() {
					cb();
				});
			},function() {
				cleanup();
				//done
			});
		});

		function process_chunk(time_chunk,callback) {
			var chunk_path=create_temporary_mda_path();
			get_chunk(params.timeseries,chunk_path,M,time_chunk.t1_minus_overlap,time_chunk.t2_plus_overlap,function() {
				mp_exec_process('bandpass_filter_kernel',{timeseries:chunk_path},{timeseries_out:chunk_path+'.out.mda'},{samplerate:params.samplerate,freq_min:params.freq_min,freq_max:params.freq_max},function() {
					remove_temporary_file(chunk_path);
					get_chunk(chunk_path+'.out.mda',chunk_path+'.out2.mda',M,time_chunk.t1-time_chunk.t1_minus_overlap,time_chunk.t2-time_chunk.t1_minus_overlap,function() {
						remove_temporary_file(chunk_path+'.out.mda');
						set_chunk(params.timeseries_out,chunk_path+'.out2.mda',M,time_chunk.t1,time_chunk.t2,function() {
							remove_temporary_file(chunk_path+'.out2.mda');
							callback();
						});
					});
				});
			});
		}
	});	
}

var fs=require('fs');
function remove_temporary_file(path) {
	fs.unlink(path);
}

function cleanup() {

}

function create_array(path,M,N,callback) {
	var exe='mda';
	var args=['create',path,'--size='+M+'x'+N,'--dtype=float32'];
	make_system_call(exe,args,{show_stdout:false,show_stderr:false,num_tries:5},function(aa) {
		if (aa.return_code!=0) {
			console.log ('Returned non-zero exit code: '+exe+' '+args.join(' '));
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}

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
		args.push('--'+key+'='+all_params[key]);
	}
	make_system_call(exe,args,{show_stdout:false,show_stderr:true,num_tries:5},function(aa) {
		if (aa.return_code!=0) {
			console.log('Subprocess '+processor_name+' returned with a non-zero exit code.');
			cleanup();
			process.exit(-1);
		}
		callback(aa);
	});
}



function create_temporary_mda_path() {
	var dirname='/tmp/mountainlab/tmp_short_term/bandpass_filter_aa';
	mkdir(dirname);
	return dirname+'/'+make_random_id(12)+'.tmp.mda';
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
			end_function();
			return;
		}
		while ((ii<array.length)&&(num_running<num_parallel)) {
			num_running++;
			ii++;
			step_function(ii-1,array[ii-1],function() {
				num_running--;
				next_step();
			});
		}
	}
}

function get_time_chunks(M,N,opts) {
	var ret=[];
	for (var t1a=0; t1a<N; t1a+=opts.chunk_size) {
		var t1=t1a;
		var t2=t1+opts.chunk_size-1;
		if (t2>=N) {
			t2=N-1;
		}
		var t1_minus_overlap=t1-Math.ceil(opts.overlap_pct*opts.chunk_size);
		if (t1_minus_overlap<0) t1_minus_overlap=0;
		var t2_plus_overlap=t2+Math.ceil(opts.overlap_pct*opts.chunk_size);
		if (t2_plus_overlap>=N) t2_plus_overlap=N-1;
		ret.push({t1:t1,t2:t2,t1_minus_overlap:t1_minus_overlap,t2_plus_overlap:t2_plus_overlap});
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
		if ((code!=0)&&(num_tries>1)) {
			console.error ('Re-trying system call: '+cmd+' '+args.join(' '));
			opts.num_tries=opts.num_tries-1; //todo, I really should not manipulate the opts here. very bad idea
			make_system_call(cmd,args,opts,callback);
			return;
		}
  		done=true;
		if (callback) {
			callback({stdout:all_stdout,stderr:all_stderr,return_code:code});
		}
	}
};

function make_random_id(len)
{
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for( var i=0; i < len; i++ )
        text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
}