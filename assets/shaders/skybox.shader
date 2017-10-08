{
    "Shaders":
    {
        "Vertex":{"Source":"assets/shaders/Skybox.vert"},
        "Fragment":{"Source":"assets/shaders/Skybox.frag"}
    },
    "DescriptorSetLayouts":
    [
        [
            {"Binding":0, "Count":1, "Type":"UniformBuffer"},
            {"Binding":1, "Count":1, "Type":"CombinedImageSampler"}
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
        "DepthTestEnable":true,
        "DepthWriteEnable":true
    },
    "VertexInputState":
    {
        "InputBindings":
        [
            {"Binding":0,"InputRate":"Vertex","Stride":12}
        ],
        "InputAttributes":
        [
            {"Binding": 0, "Format":"R32G32B32Sfloat", "Location":0, "Offset":0}
        ]
    }
}
