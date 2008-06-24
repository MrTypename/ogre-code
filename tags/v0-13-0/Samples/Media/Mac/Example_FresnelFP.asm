!!ARBfp1.0
# ARB_fragment_program generated by NVIDIA Cg compiler
# cgc version 1.1.0003, build date Jul  7 2003  12:21:41
# command line args: -entry main_fp -profile arbfp1
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbfp1
#program main_fp
#semantic main_fp.distortionRange
#semantic main_fp.tintColour
#semantic main_fp.noiseMap
#semantic main_fp.reflectMap
#semantic main_fp.refractMap
#var float fresnel : $vin.COLOR : COLOR : 0 : 1
#var float3 noiseCoord : $vin.TEXCOORD0 : TEXCOORD0 : 1 : 1
#var float4 projectionCoord : $vin.TEXCOORD1 : TEXCOORD1 : 2 : 1
#var float4 col : $vout.COLOR : COLOR : 3 : 1
#var float distortionRange :  : c[0] : 4 : 1
#var float4 tintColour :  : c[1] : 5 : 1
#var sampler3D noiseMap :  : texunit 0 : 6 : 1
#var sampler2D reflectMap :  : texunit 1 : 7 : 1
#var sampler2D refractMap :  : texunit 2 : 8 : 1
PARAM u0 = program.local[0];
PARAM u1 = program.local[1];
PARAM c0 = {2, 1, 0, 0};
PARAM c1 = {0.31, 0.57999998, 0.23, 0};
TEMP R0;
TEMP R1;
TEX R0.x, fragment.texcoord[0], texture[0], 3D;
ADD R1.xyz, fragment.texcoord[0], c1;
TEX R1.x, R1, texture[0], 3D;
MOV R0.y, R1.x;
MAD R0.xy, R0, c0.x, -c0.y;
MUL R0.xy, R0, u0.x;
RCP R0.w, fragment.texcoord[1].w;
MAD R0.xy, fragment.texcoord[1], R0.w, R0;
TEX R1, R0, texture[2], 2D;
TEX R0, R0, texture[1], 2D;
ADD R1, R1, u1;
ADD R0, R0, -R1;
MAD result.color, fragment.color.primary.x, R0, R1;
END
# 13 instructions, 2 R-regs, 0 H-regs.
# End of program