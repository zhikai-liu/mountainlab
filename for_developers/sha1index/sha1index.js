#!/usr/bin/env nodejs

var AWS = require('aws-sdk');

var s3 = new AWS.S3({apiVersion: '2006-03-01'});
var aws_bucket='mlscratch';

var sha1_index=null;
read_sha1_index(function() {
	sha1_index.sha1_by_etag=sha1_index.sha1_by_etag||{};
	sha1_index.info_by_sha1=sha1_index.info_by_sha1||{};
	
	process_directory('misc',function() {
		write_sha1_index(function() {
			console.log ('Done.');
		});
	});
});

function read_sha1_index(callback) {
	console.log ('Reading sha1 index...');
	read_json_s3object('sha1.index',function(tmp) {
		sha1_index=tmp.object||{};
		callback();
	});
}

function write_sha1_index(callback) {
	console.log ('Writing sha1 index...');
	write_json_s3object('sha1.index',sha1_index,function(tmp) {
		callback(tmp);
	});
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
		foreach(files,{},function(ifile,file,cb1) {
			process_file(file,function(tmp) {
				if (!tmp.success) {
					console.error(tmp.error);
				}
				cb1();
			});
		},function() {
			foreach(subdirs,{},function(isubdir,subdir,cb2) {
				process_directory(dirname+'/'+subdir,function() {
					cb2();
				});
			},function() {
				callback();
			});
		});
	});
}

function compute_sha1_for_file(file,callback) {
	console.log ('Computing sha1 for file: '+file.Key);
	var download_stream=s3.getObject({Bucket:aws_bucket,Key:file.Key}).createReadStream();
	var hash = require('crypto').createHash('sha1');
	hash.setEncoding('hex');
	var bytes_downloaded=0;
	download_stream.on('end', function() {
    	hash.end();
    	callback(hash.read());
	});
	download_stream.pipe(hash);
}

function compute_or_retrieve_sha1_for_file(file,callback) {
	var etag=file.ETag;
	if (etag in sha1_index.sha1_by_etag) {
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
	if (size0<1024*1024*1024*10) {
		compute_or_retrieve_sha1_for_file(file,function(sha1) {
			if (sha1) {
				sha1_index.info_by_sha1[sha1]={};
				sha1_index.info_by_sha1[sha1].path_hint=file.Key;
				sha1_index.info_by_sha1[sha1].etag=file.etag;
				sha1_index.info_by_sha1[sha1].size=file.Size;
			}
			callback({success:true});
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