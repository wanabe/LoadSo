float4x4 matWVP;

float4 FirstVertexShader(float4 pos :POSITION) :POSITION {
   return mul(pos, matWVP);
}
float4 FirstPixelShader(float4 color :COLOR0) :COLOR0 {
   return color;
}

technique ExtRgssTeq
{
   pass P0
   {
      VertexShader = compile vs_2_0 FirstVertexShader();
      PixelShader = compile ps_2_0 FirstPixelShader();
   }
}
