struct VSIn
{
    uint vertexId : SV_VertexID;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 color : color;
};

struct PSOut
{
    float4 color : SV_Target;
};

VSOut VSMain(VSIn input)
{
    VSOut output;

    if (input.vertexId == 0)
        output.pos = float4(0.0, 0.5, 0.0, 1.0);
    else if (input.vertexId == 1)
        output.pos = float4(-0.5, -0.5, 0.0, 1.0);
    else if (input.vertexId == 2)
        output.pos = float4(0.5, -0.5, 0.0, 1.0);
    else
        output.pos = float4(0.0, 0.0, 0.0, 1.0);

    output.color = clamp(output.pos, 0, 1);

    return output;
}

PSOut PSMain(VSOut input)
{
    PSOut output;

    output.color = input.color;

    return output;
}