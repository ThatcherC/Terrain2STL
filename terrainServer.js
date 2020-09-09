//Node server for hgt-to-stl program
//Listens on port 8081
var express = require('express');
var bodyParser = require('body-parser');
var fs = require('fs');
var exec = require('child_process').exec;
var config = require('./config');
var queue = require("queue");

var app = express();
app.use(bodyParser.json()); // support json encoded bodies
app.use(bodyParser.urlencoded({ extended: true })); // support encoded bodies

app.listen(8080);
var counter = 0;

//initialization from https://www.npmjs.com/package/task-queue
var q = queue()
q.concurrency = 2;
q.timeout=20000;
q.autostart = true;

app.post("/",function(req,res){
	var b = req.body;
	//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight

	var fileNum  = counter;
	var zipname  = "./stls/terrain-"+fileNum;
	var filename = "./stls/rawmodel-"+fileNum+".stl";

	var command = "./celevstl "+b.lat+" "+b.lng+" "+b.boxWidth/3+" "
			+b.boxHeight/3+" "+b.vScale+" "+b.rotation+" "+b.waterDrop+" "
			+b.baseHeight+" "+b.boxScale+" "+filename;
	command += "; zip -q "+zipname+" "+filename;

	/*
	var command = "./elevstl "+b.lat+" "+b.lng+" "+b.boxSize/3+" "
			+b.boxSize/3+" "+b.vScale+" "+b.rotation+" "+b.waterDrop+" "+b.baseHeight+" "+b.boxScale+" > "+filename;
	command += "; zip -q "+zipname+" "+filename;

	var cfilename = "./stls/crawmodel-"+fileNum+".stl";
	var czipname  = "./stls/cterrain-"+fileNum;

        command += "; ./celevstl "+b.lat+" "+b.lng+" "+b.boxSize/3+" "
                        +b.boxSize/3+" "+b.vScale+" "+b.rotation+" "+b.waterDrop+" "+b.baseHeight+" "+b.boxScale;
	command += "; mv out.stl "+cfilename;
        command += "; zip -q "+czipname+" "+cfilename;
	*/

        console.log("> Request for "+b.lat+" "+b.lng);
	startTime = Date.now()
	paramLog = startTime+"\t"+b.lat+"\t"+b.lng+
		"\t"+b.boxHeight+"\t"+b.boxWidth+"\t"+b.boxScale+"\t"+
		b.vScale+"\t"+b.rotation+"\t"+b.waterDrop+"\t"+b.baseHeight+"\t";

	console.log(command);

	q.push(function(cb){
			exec(command, function(error,stdout,stderr){
				 console.log(stderr||"STL "+fileNum+ " created");
				 res.end(String(fileNum));
				 //res.type("application/zip");
				 //res.download(zipname+".zip");
				logString = paramLog+Date.now()+"\n";
				fs.appendFile("logs/params.log", logString,function(err){
					if(err) console.log("> Error!: "+err);
				});
				fs.appendFile("logs/commands.log", command+"\n", function(err){
					if(err) console.log("> Error!: "+err);
				});
				cb();
			})});
	counter++;
	//res.render("preview.ejs",{filename:"/test.stl",width:b.boxSize/3,height:b.boxSize/3});
});

var datetime = new Date();
console.log("terrainServer.js starting at:");
console.log(datetime);
