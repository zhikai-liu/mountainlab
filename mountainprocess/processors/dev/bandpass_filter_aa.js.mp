#!/usr/bin/env nodejs

var CLP=new CLParams(process.argv);
arg1=CLP.unnamedParameters[0]||'';
if (arg1=='spec') {
	var spec=get_spec();
	console.log(JSON.stringify(spec));
}
else if (arg1=='bandpass_filter_aa') {
	bandpass_filter_aa(CLP.namedParameters);
}

function bandpass_filter_aa(params) {
	timeseries=params.timeseries;
	read_mda_header(timeseries,function (header) {
		console.log(JSON.stringify(header));
	});
}

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
	        //{name: "timeseries_out"}
	    ],
	    parameters: [
	        //{name: "samplerate",optional: false},
	        //{name: "freq_min",optional: false},
	        //{name: "freq_max",optional: false},
	        //{name: "freq_wid",optional: true}
	    ]
	});
	return {processors:processors};
}

function read_mda_header(path,callback) {
	var exe='mdaconvert';
	var args=[path,'--readheader'];
	console.log('Running: '+exe+' '+args.join(' '));
	make_system_call(exe,args,function(aa) {
		callback({str:aa.stdout})
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

function make_system_call(cmd,args,callback) {
	var child_process=require('child_process');
	var pp=child_process.spawn(cmd,args);
	pp.stdout.setEncoding('utf8');
	pp.stderr.setEncoding('utf8');
	var done=false;
	pp.on('close', function(code) {
		return_it();
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
		all_stdout+=data;
	});
	pp.stderr.on('data',function(data) {
		all_stderr+=data;
	});
	function return_it() {
		if (done) return;
  		done=true;
		if (callback) {
			console.log('returning');
			callback({stdout:all_stdout,stderr:all_stderr});
		}
	}
};
