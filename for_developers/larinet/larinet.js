var express = require('express');
var app = express();

var config=read_json_file(__dirname+'/larinet.user.json');
if (!config) {
	console.log ('Missing, empty, or invalid config file');
	return;
}

var data_directory=config.data_directory||'';
var prv_exe=__dirname+'/../../bin/prv';
var mp_exe=__dirname+'/../../bin/mountainprocess';

if (!data_directory) {
	console.log ('problem: data_directory is empty.');
	return;
}

var larinetserver=require(__dirname+'/larinetserver.js');
var handler_opts={};
handler_opts.prv_exe=prv_exe;
handler_opts.mp_exe=mp_exe;
handler_opts.data_directory=data_directory;
var larinetserver=require(__dirname+'/larinetserver.js');
var Handler=new larinetserver.RequestHandler(handler_opts);

app.set('port', (process.env.PORT || 5005));

app.use(function(req,resp,next) {
	var url_parts = require('url').parse(req.url,true);
	var host=url_parts.host;
	var path=url_parts.pathname;
	var query=url_parts.query;
	if (query.mode!='download') {
		Handler.handle_request(req,resp);
	}
	else {
		next();
	}
});
app.use(express.static(data_directory));

app.listen(app.get('port'), function() {
  console.log ('Node app is running on port', app.get('port'));
});

///////////////////////////////////////////////////////////////////////////////////////////////


function read_text_file(fname) {
	try {
		return require('fs').readFileSync(fname,'utf8');
	}
	catch(err) {
		console.log ('Problem reading text file: '+fname);
		return '';
	}
}

function read_json_file(fname) {
	var txt=read_text_file(fname);
	if (!txt) return null;
	try {
		return JSON.parse(txt);
	}
	catch(err) {
		console.log ('Error parsing json: '+txt);
		return null;
	}	
}