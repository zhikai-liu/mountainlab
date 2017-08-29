var exports = module.exports = {};

exports.RequestHandler=function(hopts) {
	this.handle_request=function(REQ,RESP) {
		//parse the url of the request
		var url_parts = require('url').parse(REQ.url,true);
		var host=url_parts.host;
		var path=url_parts.pathname;
		var query=url_parts.query;

		var expected_path='/banjoserver';

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
			if (path!==expected_path) {
				send_json_response({success:false,error:'Unexpected path: '+path});
				return;
			}
			var action=query.a||'';
			if (!action) {
				send_json_response({success:false,error:'Empty action.'});
				return;
			}
			if (!query.passcode) {
				send_json_response({success:false,error:'Passcode needed.'});
				return;	
			}
			if (query.passcode!=hopts.passcode) {
				send_json_response({success:false,error:'Incorrect passcode.'});
				return;		
			}

			if (action=='prv-locate') {
				prv_locate(query,function(resp) {
					send_json_response(resp);
				});
			}
			else if (action=='queue-process') {
				queue_process(query,function(resp) {
					send_json_response(resp);
				});
			}
			else {
				send_json_response({success:false,error:'Unexpected action: '+action});
			}
		}
		else if(REQ.method=='POST') {
			send_json_response({success:false,error:"Unsuported request method: POST"});
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
				if (hopts.download_url) resp.url=hopts.download_url+'/'+txt;
				callback(resp);
			});	
		}
		function queue_process(query,callback) {
			var processor_name=query.processor_name||'';
			var inputs_str=query.inputs;
			var outputs_str=query.outputs;
			var parameters_str=query.parameters;
			if ((!processor_name)||(!inputs_str)||(!outputs_str)||(!parameters_str)) {
				callback({success:false,error:'Invalid query for queue-process'});
				return;
			}
			var inputs,outputs,parameters;
			try {
				inputs=JSON.parse(inputs_str);
				outputs=JSON.parse(outputs_str);
				parameters=JSON.parse(parameters_str);
			}
			catch(err) {
				callback({success:false,error:'Problem parsing json in query'});
				return;	
			}
			var opts={};
			queue_process2(processor_name,inputs,outputs,parameters,opts,callback);
		}
		function queue_process2(processor_name,inputs,outputs,parameters,opts,callback) {
			var request_fname=make_tmp_json_file(hopts.data_directory);
			var response_fname=make_tmp_json_file(hopts.data_directory);
			var mpreq={action:'queue_process',processor_name:processor_name,inputs:inputs,outputs:outputs,parameters:parameters};
			console.log(JSON.stringify(mpreq));
			if (!write_text_file(request_fname,JSON.stringify(mpreq))) {
				callback({success:false,error:'Problem writing mpreq to file'});
				return;
			}
			var done=false;
			var ppp=run_process_and_read_stdout(hopts.mp_exe,['handle-request','--prvbucket_path='+hopts.data_directory,request_fname,response_fname],function(txt) {
				done=true;
				remove_file(request_fname);
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
			REQ.on('close', function (err) {
				if (!done) {
					//request has been canceled
					if (ppp) {
						console.log( 'Terminating process because request has been canceled');
						console.log(ppp.pid);
						ppp.stdout.pause();
						ppp.kill('SIGKILL');
					}
				}
			});
		}
		function starts_with(str,substr) {
			return (str.slice(0,substr.length)==substr);
		}
		function make_tmp_json_file(data_directory) {
			return data_directory+'/tmp.'+make_random_id(10)+'.json';
		}
		function make_random_id(len) {
		    var text = "";
		    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

		    for( var i=0; i < len; i++ )
		        text += possible.charAt(Math.floor(Math.random() * possible.length));

		    return text;
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

