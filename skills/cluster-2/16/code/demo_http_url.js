var http = require('http');

// creates a server object:
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'}); //write a response to the client
  //If the response from the HTTP server is supposed to be displayed as HTML, you should include an HTTP header with the correct content type:
  //The first argument of the res.writeHead() method is the status code, 200 means that all is OK, the second argument is an object containing the response headers.
  res.write(req.url);
  //The function passed into the http.createServer() has a req argument that represents the request from the client, as an object (http.IncomingMessage object).
//This object has a property called "url" which holds the part of the url that comes after the domain name:
  res.end(); // end the response
}).listen(8080); //the server object listens on port 8080