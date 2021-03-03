// https://socket.io/get-started/chat/
// source:
var app = require('express')();
var http = require('http').Server(app);
var fs = require('fs')

var amazon =[], facebook = [], google = [], microsoft = [];
// need to populate chart options
// title, ways to sort csv, axis labels, legends, type of chart (spline)
var chartOptions = {
	animationEnabled: true,
	title:{
		text: "Stock prices"
	},
	axisX: {
		Prefix: "Day",
		interval: 1
	},
	axisY: {
		title: "Closing Price",
		Prefix: "$"
	},
	legend: {
		cursor: "pointer",
		verticalAlign: "top",
		horizontalAlign: "center",
		dockInsidePlotArea: true,
		itemclick: toogleDataSeries
	},
	toolTip: {
		shared: true
	},
	data: [{
		name: "Amazon",
		type: "spline",
		yValueFormatString: "$####",
		showInLegend: true,
		dataPoints: amazon
	},
	{
		name: "Facebook",
		type: "spline",
		yValueFormatString: "$####",
		showInLegend: true,
		dataPoints: facebook
	},
	{
		name: "Google",
		type: "spline",
		yValueFormatString: "$####",
		showInLegend: true,
		dataPoints: google
	},
	{
		name: "Microsoft",
		type: "spline",
		yValueFormatString: "$####",
		showInLegend: true,
		dataPoints: microsoft
	}]

};

//using filestream to import stocks.csv
const filepath = './stocks.csv'
var csv_lines = fs.readFileSync(filepath, 'utf8').toString().split("\n");
var data = [];
for (i = 1; i < csv_lines.length; i++)
{
	data = csv_lines[i].split(",");
	if (data[1] == 'AMZN')
	{
		amazon.push({
			x: parseInt(data[0]),
			y: parseInt(data[2])
		});
	}
	else if (data[1] == 'FB')
	{
		facebook.push({
			x: parseInt(data[0]),
			y: parseInt(data[2])
		});
	}
	else if (data[1] == 'GOOGL')
	{
		google.push({
			x: parseInt(data[0]),
			y: parseInt(data[2])
		});
	}
	else if (data[1] == 'MSFT')
	{
		microsoft.push({
			x: parseInt(data[0]),
			y: parseInt(data[2])
		});
	}
	else 
	{
		console.log("Error getting data");
	}
}
//https://stackabuse.com/reading-and-writing-csv-files-with-node-js/
//https://dev.to/isalevine/parsing-csv-files-in-node-js-with-fs-createreadstream-and-csv-parser-koi
//create socket
var io = require('socket.io')(http);

//get index.html direction to plot chart
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});
//get stock.csv data 
app.get('/data', function(req, res) {
    res.sendFile(__dirname + '/stocks.csv');
  });

//socket io to emit chartoptions onto local host 
io.on('connection', function(socket){
  io.emit('dataMsg', chartOptions);
});

// nodejs 
http.listen(3000, function(){
  console.log('listening on *:3000');
});

function toogleDataSeries(e){
	if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
		e.dataSeries.visible = false;
	} else{
		e.dataSeries.visible = true;
	}
	chart.render();
}