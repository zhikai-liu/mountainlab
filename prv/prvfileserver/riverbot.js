function riverbot(path,query,callback) {
	var resp={};
	resp.text='QUERY: '+JSON.stringify(query);
	callback(resp);
}

exports.riverbot=riverbot;
