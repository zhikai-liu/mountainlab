#!/usr/bin/env nodejs

var common=require('./common.js');
var spec={processors:[]};
var processors_by_name={};

register_processor(require('./basic_sort.js'));
register_processor(require('./multichannel_sort.js'));

function register_processor(P) {
	var spec0=P.spec();
	spec0.exe_command=__filename+' '+spec0.name+' $(arguments)';
	spec.processors.push(spec0);
	processors_by_name[spec0.name]=P;
}

var fs=require('fs');
var os=require('os');

var CLP=new common.CLParams(process.argv);
var arg1=CLP.unnamedParameters[0]||''; // name of processor or "spec"
if (arg1=='spec') {
	// display the spec for this processor library
	console.log (JSON.stringify(spec,null,'  '));
	return;
}

processors_by_name[arg1].run(CLP.namedParameters,function() {
	//done
});

