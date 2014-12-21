//Node server for hgt-to-stl program
//Listens on port 8081
var app = require('http').createServer(handler)
var io = require('socket.io')();
var fs = require('fs');
var exec = require('child_process').exec;
var config = require('./config');

var router = {};
router['/'] = '/terrain2stl.html';
router['/index.html'] = '/terrain2stl.html';
router['/gmaps-gridlines.js'] = '/gmaps-gridlines.js';

app.listen(8080);
io.listen(app);

function handler(req, res){
	//add an about page?

	console.log("req");

	var filename;
	if(router[req.url]!=null){
		if(config.logRequests & req.url=='/'){fs.appendFile(config.requestLogPath,req.connection.remoteAddress+"\t"+new Date().toString()+"\n");}
		filename = router[req.url];
	}else if(req.url.slice(-4)=='.stl'){
		filename = "/stls"+req.url;
		//next two lines come from: http://stackoverflow.com/a/7288883/2019017
		res.setHeader('Content-disposition', 'attachment; filename=' + req.url.slice(1));
		res.setHeader('Content-type', "application/sla");
	}else if(req.url="favicon.ico"){
		res.end();
	}else{
		filename = "404.html";
	}
	if(filename!=undefined){
		fs.readFile(__dirname + filename,
			function (err, data) {
				if (err) {
				  console.log(err);
				  res.writeHead(500);
				  return res.end('Error loading page');
				}
				res.writeHead(200);
				res.end(data);
		 	 });
	}
}



io.on('connection',function(socket){
	console.log("connection");
	socket.on('parameters',function(params){
		exec("./elevstl "+params.lat+" "+params.lng+" "+params.scale+" "+params.name+
				" "+params.waterDrop+" "+params.baseHeight ,function(error,stdout,stderr){
			console.log(stderr||"STL created");
			if(stderr==""){
				console.log("No errors");
				socket.emit('download',{'status':'ready'});
				if(config.logParams){fs.appendFile(config.paramLogPath,params.lat+"\t"+params.lng+"\t"+params.scale+"\n");}
			}else{
				socket.emit('download',{'status':'failed'});
			};
		});
	});
});
