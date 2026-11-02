// Research executable only: mount the caller's checkout for tensor replay.
Module['preRun'] = [function() {
  FS.mkdir('/project');
  FS.mount(NODEFS, { root: process.cwd() }, '/project');
}];
