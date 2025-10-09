class OccViewer {
  constructor(theCanvasId, theOutputId) {
    // EGL implementation in Emscripten SDK expects property 'canvas'
    this.canvas    = document.getElementById(theCanvasId);
    this._myOutput = document.getElementById(theOutputId);

    this.print     = this.print.bind(this);
    this.printErr  = this.printErr.bind(this);

    //let aGlCtx =                   this.canvas.getContext ('webgl2', { alpha: false, depth: true, antialias: false, preserveDrawingBuffer: true } );
    //if (aGlCtx == null) { aGlCtx = this.canvas.getContext ('webgl',  { alpha: false, depth: true, antialias: false, preserveDrawingBuffer: true } ); }
    //this.canvas.tabIndex = -1
    //this.canvas.onclick = (theEvent): void => { this.canvas.focus(); }
  }

  // Redirect WebAssembly standard output stream.
  print(theText) {
    this._myOutput.innerHTML += theText + "<br>";
  }

  // Redirect WebAssembly standard error stream.
  printErr(theText) {
    //this._myOutput.innerHTML += theText + "<br>";
    console.warn(theText);
  }

  onRuntimeInitialized() {
    //console.log(" @@ onRuntimeInitialized()" + Object.getOwnPropertyNames(OccViewerModule));
  }
};

var OccViewerModule = new OccViewer('occViewerCanvas', 'output');

const OccViewerModuleInitialized = createOccViewerModule(OccViewerModule);
OccViewerModuleInitialized.then(function(Module) {
  //OccViewerModule.setCubemapBackground ("cubemap.jpg");
  OccViewerModule.openFromUrl ("ball", "samples/Ball.brep");
});
