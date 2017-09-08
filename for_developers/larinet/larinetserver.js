var exports = module.exports = {};
//var base64_arraybuffer=require('base64-arraybuffer');

function larinetserver(req,onclose,callback,hopts) {
	var action=req.a||'';
	if (!action) {
		callback({success:false,error:'Empty action.'});
		return;
	}

	if (action=='prv-locate') {
		prv_locate(req,function(resp) {
			callback(resp);
		});
	}
	else if (action=='prv-upload') {
		prv_upload(req,function(resp) {
			callback(resp);
		});
	}
	else if (action=='queue-process') {
		queue_process(req,function(resp) {
			callback(resp);
		});
	}
	else {
		callback({success:false,error:'Unexpected action: '+action});
	}

	function prv_locate(query,callback) {
		if ((!query.checksum)||(!query.size)||(!('fcs' in query))) {
			callback({success:false,error:"Invalid query."});	
			return;
		}
		run_process_and_read_stdout(hopts.prv_exe,['locate','--search_path='+hopts.data_directory,'--checksum='+query.checksum,'--size='+query.size,'--fcs='+(query.fcs||'')],function(txt) {
			txt=txt.trim();
			if (!starts_with(txt,hopts.data_directory+'/')) {
				callback({success:true,found:false});
				return;
			}
			txt=txt.slice(hopts.data_directory.length+1);
			var resp={success:true,found:true,path:txt};
			if (hopts.download_base_url)
				resp.url=hopts.download_base_url+'/'+resp.path;
			callback(resp);
		});	
	}
	function prv_upload(query,callback) {
		if (!query.size) {
			callback({success:false,error:"Invalid query."});	
			return;
		}
		if (!query.data_base64) {
			callback({success:false,error:"Invalid query. Missing data_base64."});	
			return;	
		}
		//var buf = base64_arraybuffer.decode(query.data_base64);
		var buf = new Buffer(query.data_base64,'base64');;
		var fname=uploads_directory()+"/"+make_random_id(10)+'.dat';
		require('fs').writeFile(fname,buf,function(err) {
			if (err) {
				callback({success:false,error:'Problem writing file: '+err.message});
				return;
			}
			run_process_and_read_stdout(hopts.prv_exe,['stat',fname],function(txt) {
				txt=txt.trim();
				var stat=null;
				try {
					stat=JSON.parse(txt);
				}
				catch(err) {
					callback({success:false,error:'Problem parsing json from prv stat'});
					return;
				}
				if (stat.size!=req.size) {
					callback({success:false,error:'Unexpected size of new file: '+stat.size+' <> '+req.size});
					return;	
				}
				callback({success:true,prv_stat:stat});
			});
		});	
	}
	function queue_process(query,callback) {
		var processor_name=query.processor_name||'';
		var inputs=query.inputs||{};
		var outputs=query.outputs||{};
		var parameters=query.parameters||{};
		var resources=query.resources||{};
		var opts={};
		queue_process2(processor_name,inputs,outputs,parameters,resources,opts,callback);
	}
	function queue_process2(processor_name,inputs,outputs,parameters,resources,opts,callback) {
		var request_fname=make_tmp_json_file(hopts.data_directory);
		var response_fname=make_tmp_json_file(hopts.data_directory);
		var mpreq={action:'queue_process',processor_name:processor_name,inputs:inputs,outputs:outputs,parameters:parameters,resources:resources};
		if (!write_text_file(request_fname,JSON.stringify(mpreq))) {
			callback({success:false,error:'Problem writing mpreq to file'});
			return;
		}
		var done=false;
		var ppp=run_process_and_read_stdout(hopts.mp_exe,['handle-request','--prvbucket_path='+hopts.data_directory,request_fname,response_fname],function(txt) {
			done=true;
			//remove_file(request_fname);
			if (!require('fs').existsSync(response_fname)) {
				callback({success:false,error:'Response file does not exist: '+response_fname});
				return;
			}
			var json_response=read_json_file(response_fname);
			remove_file(response_fname);
			if (json_response) {
				callback(json_response);
			}
			else {
				callback({success:false,error:'unable to parse json in response file'});
			}
		});
		onclose(function() {
			if (!done) {
				//request has been canceled
				if (ppp) {
					console.log ('Terminating process because request has been canceled');
					console.log (ppp.pid);
					ppp.stdout.pause();
					ppp.kill('SIGKILL');
				}
			}
		});
		/*if (REQ) {
			REQ.on('close', function (err) {
				
			});
		}*/
	}

	function starts_with(str,substr) {
		return (str.slice(0,substr.length)==substr);
	}
	function make_tmp_json_file(data_directory) {
		return data_directory+'/tmp.'+make_random_id(10)+'.json';
	}
	function uploads_directory() {
		var ret=hopts.data_directory+"/_uploads";
		try {require('fs').mkdirSync(ret);}
		catch(err) {}
		return ret;
	}
	function make_random_id(len) {
	    var text = "";
	    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	    for( var i=0; i < len; i++ )
	        text += possible.charAt(Math.floor(Math.random() * possible.length));

	    return text;
	}
}

exports.larinetserver=larinetserver;

exports.RequestHandler=function(hopts) {
	this.handle_request=function(REQ,RESP) {
		//parse the url of the request
		var url_parts = require('url').parse(REQ.url,true);
		var host=url_parts.host;
		var path=url_parts.pathname;
		var query=url_parts.query;

		if (REQ.method == 'OPTIONS') {
			var headers = {};
			
			//allow cross-domain requests
			/// TODO refine this
			
			headers["Access-Control-Allow-Origin"] = "*";
			headers["Access-Control-Allow-Methods"] = "POST, GET, PUT, DELETE, OPTIONS";
			headers["Access-Control-Allow-Credentials"] = false;
			headers["Access-Control-Max-Age"] = '86400'; // 24 hours
			headers["Access-Control-Allow-Headers"] = "X-Requested-With, X-HTTP-Method-Override, Content-Type, Accept";
			RESP.writeHead(200, headers);
			RESP.end();
		}
		else if (REQ.method=='GET') {
			console.log ('GET: '+REQ.url);
			var onclose0=function(to_run_on_close) {
				REQ.on('close',function() {
					to_run_on_close();
				});
			}
			larinetserver(query,onclose0,function(resp) {
				if (resp) send_json_response(resp);
			},hopts);
		}
		else if(REQ.method=='POST') {
			receive_json_post(REQ,function(obj) {
				if (!obj) {
					var err='Problem parsing json from post';
					console.log (err);
					send_json_response({success:false,error:err});
					return;
				}
				var onclose0=function(to_run_on_close) {
					REQ.on('close',function() {
						to_run_on_close();
					});
				}
				larinetserver(obj,onclose0,function(resp) {
					if (resp) send_json_response(resp);
				},hopts);
			});
		}
		else {
			send_text_response("Unsuported request method.");
		}
		
		function send_json_response(obj) {
			console.log ('RESPONSE: '+JSON.stringify(obj));
			RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"application/json"});
			RESP.end(JSON.stringify(obj));
		}
		function send_text_response(text) {
			console.log ('RESPONSE: '+text);
			RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/plain"});
			RESP.end(text);
		}
		function send_html_response(text) {
			console.log ('RESPONSE: '+text);
			RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/html"});
			RESP.end(text);
		}

		
		
	};
};

function run_process_and_read_stdout(exe,args,callback) {
	console.log ('RUNNING:'+exe+' '+args.join(' '));
	var P;
	try {
		P=require('child_process').spawn(exe,args);
	}
	catch(err) {
		console.log (err);
		callback({success:false,error:"Problem launching: "+exe+" "+args.join(" ")});
		return;
	}
	var txt='';
	P.stdout.on('data',function(chunk) {
		txt+=chunk;
	});
	P.on('close',function(code) {
		callback(txt);
	});
	return P;
}

function write_text_file(fname,txt) {
	try {
		require('fs').writeFileSync(fname,txt);
		return true;
	}
	catch(err) {
		return false;
	}
}

function read_text_file(fname) {
	try {
		return require('fs').readFileSync(fname,'utf8');
	}
	catch(err) {
		return '';
	}
}

function read_json_file(fname) {
	try {
		return JSON.parse(read_text_file(fname));
	}
	catch(err) {
		return '';
	}	
}

function remove_file(fname) {
	try {
		require('fs').unlinkSync(fname);
		return true;
	}
	catch(err) {
		return false;
	}
}

function receive_json_post(REQ,callback_in) {
	var callback=callback_in;
	var body='';
	REQ.on('data',function(data) {
		body+=data;
	});
	REQ.on('error',function() {
		if (callback) callback(null);
		callback=null;
	})
	
	REQ.on('end',function() {
		var obj;
		try {
			var obj=JSON.parse(body);
		}
		catch (e) {
			if (callback) callback(null);
			callback=null;
			return;
		}
		if (callback) callback(obj);
		callback=null;
	});
}
