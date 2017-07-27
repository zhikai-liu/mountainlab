var fs=require('fs');

var user_contexts={};

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
	
	var user_id=query.user_id||'anonymous';
	if (!user_contexts[user_id])
		user_contexts[user_id]=new UserContext();
	var user_context=user_contexts[user_id];

	response_opts={
		public:false
	};
	var query_list=(query.text||'').split(' ');
	var ii=0;
	option_args=[];
	while ((query_list[ii])&&(query_list[ii][0]=='-')) {
		option_args.push(query_list[ii]);
		ii++;
	}
	query_list=query_list.slice(ii);


	if (query_list[0]=='cd') {
		user_context.setWorkingDirectory(query_list[1]||'');
		callback(
			format_msg({text:'Working directory is: '+user_context.workingDirectory()})
		);
	}
	else if (query_list[0]=='debug') {
		query.token=''; //hide the token
		callback(
			format_msg({text:'QUERY: '+JSON.stringify(query)})
		);
	}
	else if (query_list[0]=='git-pull') {
		git_pull(callback);
	}
	else if (query_list[0]=='compile-mountainlab') {
		compile_mountainlab(callback);
	}
	else {
		for (var i in option_args) {
			if (option_args[i]=='-p') {
				response_opts.public=true;
			}
		}
		if (command_is_allowed(query_list)) {
			//setTimeout(function() {
			run_process_and_read_stdout(query_list[0],query_list.slice(1),user_context.workingDirectory(),function(output) {
				callback(
					format_msg({text:output})
				);
			});
			//},10);
			return;
		}	
		else {
			callback({text:'command is not allowed: '+query_list.join(' ')});
			return;
		}
	}
	function git_pull(callback) {
		var exe='git';
		var args=['pull'];
		var working_directory=__dirname+'/../..';
		run_process_and_read_stdout(exe,args,working_directory,function(output) {
			callback(
				format_msg({text:'Note: git-pull will not restart the server\n'+output})
			);
		});
	}
	function compile_mountainlab(callback) {
		var exe=__dirname+'/../../compile_components.sh';
		var args=[];
		var working_directory=__dirname+'/../..';
		run_process_and_read_stdout(exe,args,working_directory,function(output) {
			callback(
				format_msg({text:output})
			);
		});	
	}
	function format_msg(X) {
		var title='/riverbot '+query.text;
		/*
		var ret={};
		ret.text='*'+title+'*'+'\n'+X.text;
		//ret.mrkdwn=true;
		if (response_opts.public) {
			ret.response_type='in_channel';
		}
		return ret;
		*/
		var ret={
			attachments:[{
				fallback:title,
				title:title,
				text:X.text
			}]
		};
		return ret;
	}
}

function command_is_allowed(cmd) {
	if (cmd.length==1) {
		if (cmd[0]=='ls') return true;
		if (cmd[0]=='debug') return true;
		if (cmd[0]=='kron-list-datasets') return true;
		if (cmd[0]=='kron-list-pipelines') return true;
		if (cmd[0]=='mdaconvert') return true;
		if (cmd[0]=='git-pull') return true;
		if (cmd[0]=='compile-mountainlab') return true;
	}
	if (cmd[0]=='kron-run') return true;
	if (cmd[0]=='mp-daemon-start') return true;
	if (cmd[0]=='mp-daemon-stop') return true;
	if (cmd[0]=='mp-daemon-state') return true;
	if (cmd[0]=='mp-daemon-state-summary') return true;
	return false;
}

function run_process_and_read_stdout(exe,args,working_directory,callback) {
	console.log ('RUNNING:'+exe+' '+args.join(' ')+' in '+working_directory);
	var P;
	try {
		var opts={};
		if (working_directory) opts.cwd=working_directory;
		P=require('child_process').spawn(exe,args,opts);
		P.on('error',function(err) {
			var txt='Error spawning: '+exe+" "+args.join(" ")+":::"+JSON.stringify(err);
			txt+='PATH='+process.env.PATH;
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

function UserContext() {
	this.setWorkingDirectory=function(path) {m_working_directory=path;};
	this.workingDirectory=function(path) {return m_working_directory;};

	var m_working_directory='';
};


exports.riverbot=riverbot;
