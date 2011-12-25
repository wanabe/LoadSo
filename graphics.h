#include "d3d9.h"
#include "d3dx9.h"

#define FVF_VERTEX   (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
typedef struct
{
  float x, y, z;
  D3DCOLOR diffuse;
  float u, v;
} VERTEX;

extern void Init_ExtGraphics();
