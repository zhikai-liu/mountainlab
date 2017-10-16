#!/usr/bin/env nodejs

var AWS = require('aws-sdk');

var s3 = new AWS.S3({apiVersion: '2006-03-01'});
var aws_bucket='mlscratch';
var global_num_parallel=20;
var etags_found={};

var timer=new Date();

var sha1_index=null;
read_sha1_index(function() {
	sha1_index.sha1_by_etag=sha1_index.sha1_by_etag||{};
	sha1_index.info_by_sha1=sha1_index.info_by_sha1||{};
	
	process_directory('misc',function() {
		clean_up(function() {
			write_sha1_index(function() {
				console.log ('Done.');
			});	
		})
	});
});

var last_sha1_index={};
function read_sha1_index(callback) {
	console.log ('Reading sha1 index...');
	read_json_s3object('sha1.index',function(tmp) {
		sha1_index=tmp.object||{};
		last_sha1_index=JSON.parse(JSON.stringify(sha1_index));
		callback();
	});
}

function write_sha1_index(callback) {
	if (JSON.stringify(last_sha1_index)==JSON.stringify(sha1_index)) {
		console.log ('sha1 index unchanged.');
		callback({success:true});
		return;
	}
	console.log ('Writing sha1 index...');
	write_json_s3object('sha1.index',sha1_index,function(tmp) {
		last_sha1_index=JSON.parse(JSON.stringify(sha1_index));
		callback(tmp);
	});
}

function clean_up(callback) {
	for (var etag in sha1_index.sha1_by_etag) {
		if (!(etag in etags_found)) {
			console.log ('Removing etag: '+etag);
			delete sha1_index.sha1_by_etag[etag];
		}
	}
	for (var sha1 in sha1_index.info_by_sha1) {
		var etag=sha1_index.info_by_sha1[sha1].etag;
		if (!(etag in etags_found)) {
			console.log ('Removing from index: '+sha1_index.info_by_sha1[sha1].path_hint);
			delete sha1_index.info_by_sha1[sha1];
		}
	}
	callback();
}

function read_json_s3object(fname,callback) {
	s3.headObject({Bucket:aws_bucket,Key:fname},function(err1,data1) {
		if (err1) {
			callback({success:false,error:err1.code});
			return;
		}
		s3.getObject({Bucket:aws_bucket,Key:fname},function(err2,data2) {
			if (err2) {
				callback({success:false,error:'Error getting object '+fname+' in bucket '+aws_bucket+': '+err2.code})
				return;
			}
			var str=data2.Body.toString('utf-8');
			var obj;
			try {
				obj=JSON.parse(str);
			}
			catch(err) {
				callback({success:false,error:'Error parsing json'});
				return;
			}
			callback({success:true,object:obj});
		});
	});
}

function write_json_s3object(fname,obj,callback) {
	s3.upload({Bucket:aws_bucket,Key:fname,Body:JSON.stringify(obj)},function(err1,data1) {
		if (err1) {
			callback({success:false,error:err1.code});
			return;
		}
		callback({success:true});
	});
}

function process_directory(dirname,callback) {
	console.log ('Processing '+dirname);
	s3.listObjects({Bucket:aws_bucket,Prefix:dirname+'/',Delimiter:'/'},function(err1,data1) {
		if (err1) {
			console.log ('Error listing objects: '+err1.code);
			return;
		}
		var files=data1.Contents;
		var subdirs=[];
		for (var i in data1.CommonPrefixes) {
			var tmp=data1.CommonPrefixes[i].Prefix;
			var tmp2=tmp.split('/');
			subdirs.push(tmp2[tmp2.length-2]);
		}
		foreach(files,{num_parallel:global_num_parallel},function(ifile,file,cb1) {
			process_file(file,function(tmp) {
				if (!tmp.success) {
					console.error(tmp.error);
				}
				cb1();
			});
		},function() {
			foreach(subdirs,{num_parallel:global_num_parallel},function(isubdir,subdir,cb2) {
				process_directory(dirname+'/'+subdir,function() {
					cb2();
				});
			},function() {
				callback();
			});
		});
	});
}

var num_locked_resources=0;
function lock_resource(callback) {
	if (num_locked_resources<global_num_parallel) {
		num_locked_resources++;
		callback();
	}
	else {
		setTimeout(function() {
			lock_resource(callback)
		},100);
	}
}
function unlock_resource() {
	setTimeout(function() {
		num_locked_resources--;
	},1);
}

function compute_sha1_for_file(file,callback) {
	lock_resource(function() {
		console.log ('Computing sha1 for file: '+file.Key);
		var download_stream=s3.getObject({Bucket:aws_bucket,Key:file.Key}).createReadStream();
		var hash = require('crypto').createHash('sha1');
		hash.setEncoding('hex');
		var bytes_downloaded=0;
		download_stream.on('end', function() {
	    	hash.end();
	    	callback(hash.read());
	    	unlock_resource();
		});
		download_stream.pipe(hash);
	});
}

function compute_or_retrieve_sha1_for_file(file,callback) {
	var etag=file.ETag;
	if (etag in sha1_index.sha1_by_etag) {
		console.log ('Retrieved sha1 for file: '+file.Key);
		callback(sha1_index.sha1_by_etag[etag]);
		return;
	}
	else {
		compute_sha1_for_file(file,function(sha1) {
			if (sha1) {
				sha1_index.sha1_by_etag[etag]=sha1;
			}
			callback(sha1);
		});
	}
}

function process_file(file,callback) {
	var size0=file.Size;
	etags_found[file.ETag]=1;
	if (size0<1024*1024*1024*100) {
		compute_or_retrieve_sha1_for_file(file,function(sha1) {
			if (sha1) {
				sha1_index.info_by_sha1[sha1]={};
				sha1_index.info_by_sha1[sha1].path_hint=file.Key;
				sha1_index.info_by_sha1[sha1].etag=file.ETag;
				sha1_index.info_by_sha1[sha1].size=file.Size;
			}
			var elapsed=(new Date())-timer;
			if (elapsed>1000*20) {
				timer=new Date();
				write_sha1_index(function() {
					callback({success:true});
				});
			}
			else {
				callback({success:true});
			}
		});
	}
	else {
		callback({success:true});
	}
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
}