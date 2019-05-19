const os = require('os')
const http = require('http')
const child_process = require('child_process');

const port = 3000

const webpage =
`
<html>

<head>

<script>

function buttonClicked() {
  var request = new XMLHttpRequest();
  request.open("GET", "ShowColorPicker", true);
  request.send();
}

</script>

</head>

<body>

<button onclick="buttonClicked()" > Test </button>
</body>

</html>

`

var color_picker_path = '';

if( os.type() == "Darwin" ){
  color_picker_path = './dst/macOS/ColorPicker4MoDao.app/Contents/MacOS/ColorPicker4MoDao';
} else {
  if( os.arch() == 'x64') {
    color_picker_path = './dst/Windows/x64/ColorPicker4MoDao.exe';
  } else {
    color_picker_path = './dst/Windows/x32/ColorPicker4MoDao.exe';
  }
}

var color_picker = child_process.spawn(color_picker_path, ['--daemon']);

console.log("ColorPicker PID: " + color_picker.pid);

color_picker.stdout.on('data', function(data) {
  process.stdout.write('\033[32m'); // green
  process.stdout.write(data);
  process.stdout.write('\033[39m');
});

color_picker.stderr.on('data', function(data) {
  process.stdout.write('\033[31m'); // red
  process.stdout.write(data);
  process.stdout.write('\033[39m');
});

color_picker.on('error', function () {
  console.log("Start ColorPicker Failed");
});

color_picker.on('close', function (code) {
  console.log('ColorPicker Exit ' + code);
});

color_picker.stdout.on('end', function () {
  console.log('ColorPicker STDOUT Done');
});


var contents = new Buffer.alloc(4);
contents[0] = 0x01;
contents[1] = 0x02;
contents[2] = 0x03;
contents[3] = 0x04;


const requestHandler = (request, response) => {
  console.log(request.url)
  switch(request.url)
  {
    case '/':
      response.writeHead(200, { 'Content-Type': 'text/html' });
      response.end(webpage);
    break;
    case '/ShowColorPicker':
      response.writeHead(200, { 'Content-Type': 'text/html' });
      response.end();
      color_picker.stdin.write(contents);
    break;
    default:
      response.statusCode = 404;
      response.statusMessage = 'Not Found';
      response.end();
    break;
  }
}

const server = http.createServer(requestHandler)

server.listen(port, (err) => {
  if (err) {
    return console.log('something bad happened', err)
  }

  console.log(`server is listening on ${port}`)
})
