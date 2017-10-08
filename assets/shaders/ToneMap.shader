{
    "Shaders":
    {
        "Vertex":{"Source":"assets/shaders/ToneMap.vert", "EntryPoint":"main", "Language":"GLSL"},
        "Fragment":{"Source":"assets/shaders/ToneMap.frag", "EntryPoint":"main", "Language":"GLSL"}
    },
    "DescriptorSetLayouts":
    [
        [
            {"Binding":0, "Count":1, "Type":"CombinedImageSampler"}
        ]
    ],
    "ColorBlendState":
    {
        "LogicOp": "Copy",
        "logicOpEnable": false,
        "BlendConstants": [0,0,0,0],
        "ColorBlendAttachmentStates":
        [
            {
                "blendEnable": false,
                "SrcColorBlendFactor":"Zero",
                "DstColorBlendFactor":"Zero",
                "ColorBlendOp":"Add",
                "SrcAlphaBlendFactor":"Zero",
                "DstAlphaBlendFactor":"Zero",
                "AlphaBlendOp":"Add"
            }
        ]
    },
    "RasterizationState":
    {
        "CullMode":"None"
    },
    "DepthStencilState":
    {
        "Back":"Zero",
        "Front":"Keep",
        "DepthBoundsTestEnable":false,
        "DepthCompareOp":"LessOrEqual",
        "DepthTestEnable":false,
        "DepthWriteEnable":false
    }
}