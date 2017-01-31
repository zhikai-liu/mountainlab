var fs=require('fs');

function riverbot(path,query,callback) {
	var token;
	try {
		token=fs.readFileSync(__dirname+'/riverbot_token.txt','utf8').trim();
	}
	catch(err) {
		callback({text:'Problem reading riverbot_token.txt'});
		return;
	}
	if ((token!='*')&&(query.token!=token)) {
		callback({text:'Invalid token.'});
		return;
	}
	if (!query.text) {
		query.token=''; //hide the token
		callback({text:'QUERY: '+JSON.stringify(query)});
		return;
	}
	response_opts={
		public:false
	};
	var query_list=query.text.split(' ');
	var ii=0;
	option_args=[];
	while ((query_list[ii])&&(query_list[ii][0]=='-')) {
		option_args.push(query_list[ii]);
		ii++;
	}
	query_list=query_list.slice(ii);
	for (var i in option_args) {
		if (option_args[i]=='-p') {
			response_opts.public=true;
		}
	}
	if (command_is_allowed(query_list)) {
		//setTimeout(function() {
		run_process_and_read_stdout(query_list[0],query_list.slice(1),function(output) {
			callback(
				format_msg({title:'/riverbot '+query.text,text:output})
			);
		});
		//},10);
		return;
	}	
	function format_msg(X) {
		var ret={};
		ret.text='*'+X.title+'*'+'\n'+X.text;
		//ret.mrkdwn=true;
		if (response_opts.public) {
			ret.response_type='in_channel';
		}
		return ret;
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
		P.on('error',function(err) {
			var txt='Error spawning: '+exe+" "+args.join(" ")+":::"+JSON.stringify(err);
			txt+='PATH='+require('process').env.PATH;
			console.log(txt);
			callback(txt);
			return "";
		});
	}
	catch(err) {
		console.log(err);
		console.log("Problem launching: "+exe+" "+args.join(" "));
		callback("Problem launching: "+exec+" "+args.join(" ")+"::::"+err);
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
