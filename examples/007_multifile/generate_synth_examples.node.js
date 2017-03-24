#!/usr/bin/env nodejs

var fs=require('fs');
var child_process=require('child_process');

var CLP=new CLParams(process.argv);

var M0=CLP.namedParameters["M"]||5;
var K0=CLP.namedParameters["K"]||20;
var duration0=CLP.namedParameters["duration"]||3600;
var amp_variation_min=CLP.namedParameters["amp_variation_min"]||1;
var amp_variation_max=CLP.namedParameters["amp_variation_max"]||1;
var firing_rate_min=CLP.namedParameters["firing_rate_min"]||1;
var firing_rate_max=CLP.namedParameters["firing_rate_max"]||1;
var noise_level=CLP.namedParameters["noise_level"]||1;

var dspath=__dirname+'/datasets';
var rawpath=__dirname+'/raw';

mkdir_safe(dspath);
mkdir_safe(rawpath);
mkdir_safe(rawpath+'/timeseries');

var dspath0=dspath+'/dataset1';
mkdir_safe(dspath0);

var params0={samplerate:30000,sign:1};
write_text_file(dspath0+'/params.json',JSON.stringify(params0));
var geom0='';
for (var m=1; m<=M0; m++) {
	geom0+='0,'+m+'\n';
}
write_text_file(dspath0+'/geom.csv',geom0);


var list=[{},{},{}];
foreach(list,{},function(ii,a,cb) {
	generate_raw_file(ii+1,cb);
},function() {
	make_prv_file(rawpath+'/timeseries',dspath0+'/raw.mda.prv',function() {
		//done
	});
});

function generate_raw_file(num,cb) {
	while ((num+'').length<3) num='0'+num;
	//generate the raw data
	var cmd='mountainprocess';
	var args='run-process synthesize_timeseries_001_matlab';
	args+=' --waveforms='+rawpath+'/waveforms.mda';
	args+=' --timeseries='+rawpath+'/timeseries/raw_'+num+'.mda';
	args+=' --firings_true='+rawpath+'/firings_true.mda';
	args+=" --M="+M0+" --T=800 --K="+K0+" --duration="+duration0+" --amp_variation_min="+amp_variation_min+" --amp_variation_max="+amp_variation_max;
	args+=" --firing_rate_min="+firing_rate_min+" --firing_rate_max="+firing_rate_max;
	args+=" --noise_level="+noise_level;
	if ('_force_run' in CLP.namedParameters) args+=" --_force_run";
	args=args.split(' ');

	make_system_call(cmd,args,function(tmp) {
		if (tmp.success) {
			cb();
		}
		else {
			console.log('Error in system call. Please be sure that mountainlab/bin is in your path.\n');
		}
	});
}



function make_prv_file(fname,fname_out,callback) {
	var cmd='prv';
	var args='create '+fname+' '+fname_out;
	args=args.split(' ');
	make_system_call(cmd,args,callback);
}

function make_system_call(cmd,args,callback) {
	console.log ('Running '+cmd+' '+args.join(' '));
	var pp=child_process.spawn(cmd,args);
	pp.stdout.setEncoding('utf8');
	pp.stderr.setEncoding('utf8');
	var done=false;
	var success=true;
	pp.on('close', function(code) {
  		done=true;
  		if (callback)
			callback({success:success});
	});
	pp.on('error',function(err) {
		success=false;
		console.log ('Process error: '+cmd+' '+args.join(' '));
		console.log (err);
	});
	var all_stdout='';
	var all_stderr='';
	pp.stdout.on('data',function(data) {
		console.log (data);
		all_stdout+=data;
	});
	pp.stderr.on('data',function(data) {
		console.log (data);
		all_stderr+=data;
	});
}

function mkdir_safe(path) {
	try {
		fs.mkdirSync(path);
	}
	catch (err) {

	}
}

function CLParams(argv) {
	this.unnamedParameters=[];
	this.namedParameters={};

	var args=argv.slice(2);
	for (var i=0; i<args.length; i++) {
		var arg0=args[i];
		if (arg0.indexOf('--')==0) {
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
		else {
			this.unnamedParameters.push(arg0);
		}
	}
}

function read_text_file(path) {
	return fs.readFileSync(path,'utf8');
}

function write_text_file(path,txt) {
	fs.writeFileSync(path,txt,'utf8');
}

function foreach(array,opts,step_function,end_function) {
	var num_parallel=opts.num_parallel||1;
	var num_running=0;
	var num_finished=0;
	var already_called_end=false;
	var ii=0;
	next_step();
	function next_step() {
		if (num_finished>=array.length) {
			setTimeout(function() { //important to do it this way so we don't accumulate a call stack
				if (!already_called_end) { 
					already_called_end=true;
					end_function();
				}
			},0);
			return;
		}
		while ((ii<array.length)&&(num_running<num_parallel)) {
			num_running++;
			ii++;
			step_function(ii-1,array[ii-1],function() {
				num_running--;
				num_finished++;
				setTimeout(function() { //important to do it this way so we don't accumulate a call stack
					next_step();
				},0);
			});
		}
	}
};