const electron = require('electron')

const os = require('os')
const url = require('url')
const path = require('path')
const child_process = require('child_process')


/******************************************************************************/
/*****************      this part for our color picker          ***************/
/********                                                              ********/
/**                                                                        ****/

let color_picker_path = null
console.log(os.arch())
if( os.type() == "Darwin" ){
  color_picker_path = './dst/macOS/ColorPicker4MoDao.app/Contents/MacOS/ColorPicker4MoDao';
} else {
  if( os.arch() == 'x64') {
    color_picker_path = './dst/Windows/x64/ColorPicker4MoDao.exe';
  } else {
    color_picker_path = './dst/Windows/x32/ColorPicker4MoDao.exe';
  }
}

let color_picker_callbacker = null;
let color_picker_child_process = null;

function bind_color_picker_callbacks(the_process) {
  the_process.stdout.on('data', function(data) {
    // this is where the picked color
    process.stdout.write(data);
    color_picker_callbacker.send('onColorPicked', data.toString())
  });

  the_process.on('error', function () {
    console.log("ColorPicker Start Failed");
  });

  the_process.on('close', function (code) {
    console.log('ColorPicker Exit ' + code);
  });

  the_process.stdout.on('end', function () {
    console.log('ColorPicker STDOUT Done');
  });
}

color_picker_child_process = child_process.spawn(color_picker_path, ['--daemon'])
console.log("ColorPicker PID: " + color_picker_child_process.pid)

bind_color_picker_callbacks(color_picker_child_process)

/**                                                                        ****/
/********                                                              ********/
/***************                                                ***************/
/**************************                       *****************************/
/******************************************************************************/


let mainWindow = null

function createWindow () {
  mainWindow = new electron.BrowserWindow({width: 800, height: 600})

  mainWindow.setMenu(null)

  mainWindow.loadURL(url.format({
    pathname: path.join(__dirname, 'index.html'),
    protocol: 'file:',
    slashes: true
  }))

  mainWindow.on('closed', () => {
    win = null
  })

  mainWindow.webContents.openDevTools()
}

electron.app.on('ready', createWindow)

electron.app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
     electron.app.quit()
  }
})

electron.app.on('activate', () => {
  if (mainWindow === null) {
    createWindow()
  }
})

/******************************************************************************/
/*****************      this part for our color picker          ***************/
/********                                                              ********/
/**                                                                        ****/

electron.ipcMain.on('showColorPicker', (event, arg) => {
  let contents = new Buffer(4);
  contents[0] = 0x01;
  contents[1] = 0x02;
  contents[2] = 0x03;
  contents[3] = 0x04;

  color_picker_callbacker = event.sender;
  color_picker_child_process.stdin.write(contents);
})

/**                                                                        ****/
/********                                                              ********/
/***************                                                ***************/
/**************************                       *****************************/
/******************************************************************************/