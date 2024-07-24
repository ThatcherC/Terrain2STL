//Node server for hgt-to-stl program
const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const exec = require('child_process').exec;
const config = require('./config');
const queue = require("queue");
const path = require('path');

var app = express();
app.use(bodyParser.json()); // support json encoded bodies
app.use(bodyParser.urlencoded({ extended: true })); // support encoded bodies

const PORT = process.env.PORT || 8080;

app.listen(PORT);
var counter = 0;

//initialization from https://www.npmjs.com/package/task-queue
var q = queue()
q.concurrency = 2;
q.timeout=20000;
q.autostart = true;

// if NOSTATIC is not set, set up static file serving
if(!process.env.NOSTATIC) {
	console.log("Serving static files")
	app.use(express.static(__dirname, {index: "terrain2stl.html"}));
}

app.post("/",function(req,res){
	var b = req.body;
	//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight

	var fileNum  = counter;
	var zipname  = "./stls/terrain-"+fileNum;
	var filename = "./stls/rawmodel-"+fileNum+".stl";

//b.rotation=0;

	var command = "./celevstl "+b.lat+" "+b.lng+" "+b.boxWidth/3+" "
			+b.boxHeight/3+" "+b.vScale+" "+b.rotation+" "+b.waterDrop+" "
			+b.baseHeight+" "+b.boxScale+" "+filename;
	command += "; zip -q "+zipname+" "+filename;

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
