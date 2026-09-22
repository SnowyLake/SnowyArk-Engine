struct VertexInput
{
    [[vk::location(0)]] float2 position;
    [[vk::location(1)]] float3 color;
};

struct VertexOutput
{
    float4 sv_position : SV_Position;
    [[vk::location(0)]] float3 color;
};

[shader("vertex")]
VertexOutput MainVertex(VertexInput input)
{
    VertexOutput output;
    output.sv_position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 MainFragment(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
