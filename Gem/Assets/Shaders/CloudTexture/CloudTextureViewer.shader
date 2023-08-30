{
  "Source": "CloudTextureViewer.azsl",
  "AddBuildArguments": {
    "debug": true
  },
  "DepthStencilState": {
    "Depth": {
      "Enable": true,
      "CompareFunc": "GreaterEqual"
    }
  },
  //"GlobalTargetBlendState" : {
  //    "Enable" : true,
  //    "BlendSource" : "AlphaSource",
  //    "BlendDest" : "One",
  //    "BlendOp" : "Add"
  //},
  "DrawList": "cloudtexture",
  "ProgramSettings": {
    "EntryPoints": [
      {
        "name": "MainVS",
        "type": "Vertex"
      },
      {
        "name": "MainPS",
        "type": "Fragment"
      }
    ]
  }
}
