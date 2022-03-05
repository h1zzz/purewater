// MIT License Copyright (c) 2022, h1zzz

'use strict';

const { app, BrowserWindow } = require('electron');

const createWindow = function() {
  const win = new BrowserWindow({
    width: 800,
    height: 500,
  });
}

app.on('ready', function() {
  createWindow()
  app.on('activate', function() {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', function() {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
