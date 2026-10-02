//Node server for hgt-to-stl program
import express from 'express';
import fs from 'node:fs';
import { execFile } from 'node:child_process';
import path from 'node:path';
import queue from 'queue';
import config from './config.js';

const app = express();
app.use(express.json()); // support json encoded bodies
app.use(express.urlencoded({ extended: true })); // support encoded bodies

const PORT = process.env.PORT || 8080;
const DEMPATH = process.env.DEMPATH || "./hgt_files/";
const STLPATH = process.env.STLPATH || "./stls";

let counter = 0;

//initialization from https://www.npmjs.com/package/queue
const q = queue()
q.concurrency = 2;
q.timeout=20000;
q.autostart = true;

// if NOSTATIC is not set, set up static file serving
if(!process.env.NOSTATIC) {
	console.log("Serving static files")
	app.use(express.static(path.join(import.meta.dirname, "public"),
		{index: "terrain2stl.html", dotfiles: "deny"}));
}

// serve finished models from STLPATH, which may be outside the app directory
app.get("/stls/:name", (req, res) => {
	if (!/^terrain-\d+\.zip$/.test(req.params.name)) return res.sendStatus(404);
	res.download(path.join(STLPATH, req.params.name), err => {
		if (err && !res.headersSent) res.sendStatus(404);
	});
});

// append a line to a log file without blocking; failures are reported but not fatal
function appendLog(file, line) {
	fs.promises.appendFile(file, line).catch(err => console.log("> Error!: "+err));
}

// numeric fields expected in the POST body and their allowed [min, max]
// ranges, matching the inputs in public/terrain2stl.html
const PARAMS = {
	lat:        [-69, 84],
	lng:        [-179, 180],
	boxWidth:   [60, 2700],
	boxHeight:  [60, 2700],
	boxScale:   [1, 20],
	rotation:   [-90, 90],
	vScale:     [0.5, 4],
	waterDrop:  [0, 5],
	baseHeight: [1, 10],
};

app.post("/gen",function(req,res){
	//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight

	// parse every parameter as a number so nothing else reaches the command line
	const b = {};
	for (const [name, [min, max]] of Object.entries(PARAMS)) {
		const raw = req.body[name];
		const value = (typeof raw === "string" && raw.trim() !== "") || typeof raw === "number"
			? Number(raw) : NaN;
		if (!Number.isFinite(value)) {
			res.status(400).end("Invalid parameter: " + name);
			return;
		}
		if (value < min || value > max) {
			res.status(400).end(`Parameter ${name} must be between ${min} and ${max}`);
			return;
		}
		b[name] = value;
	}

	const fileNum  = counter;
	const zipname  = path.join(STLPATH, "terrain-"+fileNum);
	const filename = path.join(STLPATH, "terrain-"+fileNum+".stl");

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

	// log the job and reply exactly once, whether it succeeded or failed
	function finish(cb, error, stderr) {
		if (error) {
			console.log("> Model "+fileNum+" failed: "+(stderr || error.message));
			res.status(500).end("Model generation failed");
		} else {
			console.log(stderr||"STL "+fileNum+ " created");
			res.end(String(fileNum));
		}
		const logString = paramLog+Date.now()+"\n";
		if(config.logParams) appendLog(config.paramLogPath, logString);
		if(config.logCommands) appendLog(config.commandLogPath, command+"\n");
		cb();
	}

	q.push(function(cb){
		execFile("./celevstl", stlArgs, function(stlError, stlStdout, stlStderr){
			if (stlError) return finish(cb, stlError, stlStderr);
			execFile("zip", zipArgs, function(zipError, zipStdout, zipStderr){
				finish(cb, zipError, stlStderr + zipStderr);
			})
		})});
	counter++;
	//res.render("preview.ejs",{filename:"/test.stl",width:b.boxSize/3,height:b.boxSize/3});
});

app.listen(PORT);

const datetime = new Date();
console.log("terrainServer.js starting at:");
console.log(datetime);

console.log("Port: " + PORT);
console.log("DEM Path: " + DEMPATH);
console.log("STL Path: " + STLPATH);
