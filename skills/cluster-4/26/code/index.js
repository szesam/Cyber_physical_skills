// Modules
var level = require('level')
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var fs = require('fs');
var parse = require('csv-parse');

// Create or open the underlying LevelDB store
var db = level('./mydb', {valueEncoding: 'json'});
// create parser to read csv text into database
var parser = parse({delimiter: '\t'}, function (err,data)
{
    var i = 0;
    data.forEach(function(line)
    {
      var key = i;
      var value = [{time:line[0], id:line[1], smoke:line[2], temp:line[3]}];
      console.log(key,value);
      db.put(key,value, function (err) 
      {
          if (err) return console.log("error at db.put)");
      })
      i++;
    });
});
fs.createReadStream('smoke.csv').pipe(parser)

db.get('30', function (err, value) {
  if (err) {
    if (err.notFound) {
      // handle a 'NotFoundError' here
      return
    }
    // I/O or other error, pass it up the callback chain
    return callback(err)
  }
  console.log(value);
  // .. handle `value` here
});

// // Points to index.html to serve webpage
// app.get('/', function(req, res){
//   res.sendFile(__dirname + '/index.html');
// });

// // Function to stream from database
// function readDB(arg) {
//   db.createReadStream()
//     .on('data', function (data) {
//       // Parsed the data into a structure but don't have to ...
//       var dataIn = {[data.key]: data.value};
//       console.log(dataIn);
//       // Stream data to client
//       io.emit('message', dataIn);
//     })
//     .on('error', function (err) {
//       console.log('Oh my!', err)
//     })
//     .on('close', function () {
//       console.log('Stream closed')
//     })
//     .on('end', function () {
//       console.log('Stream ended')
//     })
// }

// // When a new client connects
// var clientConnected = 0; // this is just to ensure no new data is recorded during streaming
// io.on('connection', function(socket){
//   console.log('a user connected');
//   clientConnected = 0;

//   // Call function to stream database data
//   readDB();
//   clientConnected = 1;
//   socket.on('disconnect', function(){
//     console.log('user disconnected');
//   });
// });

// // Listening on localhost:3000
// http.listen(3000, function() {
//   console.log('listening on *:3000');
// });

