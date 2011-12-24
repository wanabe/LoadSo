float4x4 matWVP;

struct FIRST_VS_OUTPUT {
  float4 pos :POSITION;
  float4 diffuse :COLOR0;
};

FIRST_VS_OUTPUT FirstVertexShader(float4 pos :POSITION, float4 diffuse :COLOR0) {
  FIRST_VS_OUTPUT outVS;
  outVS.pos = mul(pos, matWVP);
  outVS.diffuse = diffuse;
  return outVS;
}
float4 FirstPixelShader(float4 color :COLOR0) :COLOR0 {
  return color;
}

technique ExtRgssTec {
  pass P0 {
    VertexShader = compile vs_2_0 FirstVertexShader();
    PixelShader = compile ps_2_0 FirstPixelShader();
  }
}
