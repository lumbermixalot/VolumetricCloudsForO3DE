{
  "Source": "Cloudscape.azsl",
  "AddBuildArguments": {
    "debug": false
  },
  "DepthStencilState" : 
  {
      "Depth" : 
      { 
          "Enable" : false 
      },
      "Stencil" :
      {
          "Enable" : false
      }
  },
  "GlobalTargetBlendState" : {
    "Enable" : true,
    "BlendSource" : "One",
    "BlendDest" : "AlphaSourceInverse",
    "BlendOp" : "Add"
},
  "DrawList": "cloudscape",
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
