//Node server for hgt-to-stl program
const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const execFile = require('child_process').execFile;
const config = require('./config');
const queue = require("queue");
const path = require('path');

var app = express();
app.use(bodyParser.json()); // support json encoded bodies
app.use(bodyParser.urlencoded({ extended: true })); // support encoded bodies

const PORT = process.env.PORT || 8080;
const DEMPATH = process.env.DEMPATH || "./hgt_files/";
const STLPATH = process.env.STLPATH || "./stls";

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

// numeric fields expected in the POST body
const PARAMS = ["lat", "lng", "boxWidth", "boxHeight", "vScale", "rotation",
	"waterDrop", "baseHeight", "boxScale"];

app.post("/gen",function(req,res){
	//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight

	// parse every parameter as a number so nothing else reaches the command line
	var b = {};
	for (const name of PARAMS) {
		const raw = req.body[name];
		const value = (typeof raw === "string" && raw.trim() !== "") || typeof raw === "number"
			? Number(raw) : NaN;
		if (!Number.isFinite(value)) {
			res.status(400).end("Invalid parameter: " + name);
			return;
		}
		b[name] = value;
	}

	var fileNum  = counter;
	var zipname  = path.join(STLPATH, "terrain-"+fileNum);
	var filename = path.join(STLPATH, "terrain-"+fileNum+".stl");

//b.rotation=0;

	// arguments are passed directly to the programs (no shell involved)
	const stlArgs = [b.lat, b.lng, b.boxWidth/3, b.boxHeight/3, b.vScale, b.rotation,
		b.waterDrop, b.baseHeight, b.boxScale, DEMPATH, filename].map(String);
	const zipArgs = ["--quiet", "--junk-paths", zipname, filename];
	const command = "./celevstl " + stlArgs.join(" ") + "; zip " + zipArgs.join(" ");

        console.log("> Request for "+b.lat+" "+b.lng);
	const startTime = Date.now()
	const paramLog = startTime+"\t"+b.lat+"\t"+b.lng+
		"\t"+b.boxHeight+"\t"+b.boxWidth+"\t"+b.boxScale+"\t"+
		b.vScale+"\t"+b.rotation+"\t"+b.waterDrop+"\t"+b.baseHeight+"\t";

	console.log(command);

	q.push(function(cb){
		execFile("./celevstl", stlArgs, function(stlError, stlStdout, stlStderr){
			// zip runs regardless of celevstl's result, as the old "; zip" did
			execFile("zip", zipArgs, function(error, stdout, zipStderr){
				const stderr = stlStderr + zipStderr;
				 console.log(stderr||"STL "+fileNum+ " created");
				 res.end(String(fileNum));
				 //res.type("application/zip");
				 //res.download(zipname+".zip");
				const logString = paramLog+Date.now()+"\n";
				fs.appendFile("logs/params.log", logString,function(err){
					if(err) console.log("> Error!: "+err);
				});
				fs.appendFile("logs/commands.log", command+"\n", function(err){
					if(err) console.log("> Error!: "+err);
				});
				cb();
			})
		})});
	counter++;
	//res.render("preview.ejs",{filename:"/test.stl",width:b.boxSize/3,height:b.boxSize/3});
});

var datetime = new Date();
console.log("terrainServer.js starting at:");
console.log(datetime);

console.log("Port: " + PORT);
console.log("DEM Path: " + DEMPATH);
console.log("STL Path: " + STLPATH);
