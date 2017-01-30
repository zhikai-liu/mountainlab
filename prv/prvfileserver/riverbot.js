var fs=require('fs');

function riverbot(path,query,callback) {
	var token=fs.readFileSync(__dirname+'/riverbot_token.txt','utf8');
	if ((token!='*')&&(query.token!=token)) {
		callback({text:'Invalid token.'});
		return;
	}
	if (!query.text) {
		query.token=''; //hide the token
		callback({text:'QUERY: '+JSON.stringify(query)});
		return;
	}
	if (command_is_allowed(query.text)) {
		var list=query.text.split(' ');
		run_process_and_read_stdout(list[0],list.slice(1),function(output) {
			callback({text:output});
		});
		return;
	}	
}

function command_is_allowed() {
	return true;
}

function run_process_and_read_stdout(exe,args,callback) {
	console.log ('RUNNING:'+exe+' '+args.join(' '));
	var P;
	try {
		P=require('child_process').spawn(exe,args);
	}
	catch(err) {
		console.log(err);
		console.log("Problem launching: "+exe+" "+args.join(" "));
		return "";
	}
	var txt='';
	P.stdout.on('data',function(chunk) {
		txt+=chunk;
	});
	P.on('close',function(code) {
		callback(txt);
	});
}


exports.riverbot=riverbot;