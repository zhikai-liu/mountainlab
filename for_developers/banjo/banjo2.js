#!/usr/bin/env nodejs

var fs=require('fs');

CLP=new CLParams(process.argv);
var arg1=CLP.unnamedParameters[0]||'';

var BanjoGenerator=require(__dirname+'/banjogenerator.js').BanjoGenerator;

var context={
	http_post_json:http_post_json,
	http_get_json:http_get_json,
	http_get_text:http_get_text,
	btoa:encode_base64
}

var params0=CLP.namedParameters;

var BG=new BanjoGenerator(context);

if ((params0.firings)&&((params0.clusters)||('t1' in params0)||('t2' in params0))) {
	BG.extractFirings(params0.firings,{clusters:params0.clusters||'',t1:params0.t1||0,t2:params0.t2||-1},function(tmp) {
		if (!tmp.success) {
			console.log('Error extracting firings: '+tmp.error);
			return;
		}
		params0.firings=tmp.firings_out;
		step2();
	});
}
else {
	step2();
}

function step2() {

if (params0.templates) {
	var templates=read_json_file(params0.templates);

	BG.createTemplatesView(templates,{},function(tmp) {
		if (!tmp.success) {
			console.log(tmp.error);
			return;
		}
		BG.addView('north',tmp.view);
		finalize();
	});
}
else if ((params0.firings)&&(params0.timeseries)) {
	var timeseries=read_json_file(params0.timeseries);
	var firings=read_json_file(params0.firings);
	BG.computeTemplates(timeseries,firings,{},function(tmp) {
		if (!tmp.success) {
			console.log(tmp.error);
			return;
		}
		console.log('---');
		console.log(tmp);
		var templates=tmp.templates_out;
		BG.createTemplatesView(templates,{},function(tmp2) {
			if (!tmp2.success) {
				console.log(tmp2.error);
				return;
			}
			BG.addView('north',tmp2.view);
			finalize();
		});
	});
	BG.computeAutocorrelograms(firings,{},function(tmp) {
		if (!tmp.success) {
			console.log(tmp.error);
			return;
		}
		var correlograms=tmp.correlograms_out;
		BG.createCorrelogramsView(correlograms,{label:'Autocorrelograms'},function(tmp2) {
			if (!tmp2.success) {
				console.log(tmp2.error);
				return;
			}
			BG.addView('south',tmp2.view);
			finalize();
		});
	});
}

}

function finalize() {
	BG.createPageUrl(function(tmp2) {
		if (!tmp2.success) {
			console.log('Error creating page url: '+tmp.error);
			return;
		}
		console.log(tmp2.url);
	});
}

function http_get_json(url,callback) {
	http_get_text(url,function(tmp) {
		if (!tmp.success) {
			callback(tmp);
			return;
		}
		var obj;
		try {
			obj=JSON.parse(tmp.text);
		}
		catch(err) {
			callback({success:false,error:'Error parsing json response: '+tmp.text});
			return;
		}
		callback({success:true,object:obj});
	});
}

function http_get_text(url,callback) {
	// Set up the request
	var get_req = require('http').get(url, function(res) {
		res.setEncoding('utf8');
		res.on('data', function (body) {
			callback({success:true,text:body});
		});
	});
	get_req.on('error',function() {
		callback({success:false,error:'Problem in http get: '+url});
	});
}


function http_post_json(url,data,callback) {
	var url_parts = require('url').parse(url,true);
	var host=url_parts.hostname;
	var path=url_parts.pathname;
	var port=url_parts.port;

	var post_data=JSON.stringify(data);

	// An object of options to indicate where to post to
	var post_options = {
		host: host,
		port: port,
		path: path,
		method: 'POST',
		headers: {
		  'Content-Type': 'application/json',
		  'Content-Length': Buffer.byteLength(post_data)
		}
	};

	// Set up the request
	var post_req = require('http').request(post_options, function(res) {
		res.setEncoding('utf8');
		res.on('data', function (body) {
			try {
				var obj=JSON.parse(body);
				callback({success:true,response:obj});
			}
			catch(err) {
				callback({success:false,error:'Problem parsing json response: '+body});
			}
		});
	});
	post_req.on('error',function() {
		callback({success:false,error:'Error in post: '+url});
	});

	// post the data
	post_req.write(post_data);
	post_req.end();
}

function encode_base64(str) {
	return (new Buffer(str)).toString('base64');
}

function read_json_file(fname) {
	var txt=read_text_file(fname);
	if (!txt) {
		console.error('File does not exist or is empty: '+fname);
		return 0;
	}
	var obj=parse_json(txt);
	if (!obj) {
		console.error('Error parsing json file: '+fname);
		return 0;
	}
	return obj;
}

function read_text_file(fname) {
	try {
		return require('fs').readFileSync(fname,'utf8');
	}
	catch(err) {
		return '';
	}
}

function parse_json(txt) {
	try {
		var obj=JSON.parse(txt);
		return obj;
	}
	catch(err) {
		return 0;
	}
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
}
