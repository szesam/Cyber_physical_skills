var http = require('http');

// creates a server object:
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'}); //write a response to the client
  //If the response from the HTTP server is supposed to be displayed as HTML, you should include an HTTP header with the correct content type:
  //The first argument of the res.writeHead() method is the status code, 200 means that all is OK, the second argument is an object containing the response headers.
  res.end('Hello World!'); // end the response
}).listen(8080); //the server object listens on port 8080