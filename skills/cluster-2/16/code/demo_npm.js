// download a package by npm install <module_name>
var http = require('http');
var uc = require('upper-case');
// Once the package is installed, it is ready to use.

// Include the "upper-case" package the same way you include any other module:
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write(uc.upperCase("Hello World!"));
  res.end();
}).listen(8080);