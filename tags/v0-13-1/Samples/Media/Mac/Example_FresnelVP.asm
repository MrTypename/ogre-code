!!ARBvp1.0
# ARB_vertex_program generated by NVIDIA Cg compiler
# cgc version 1.1.0003, build date Jul  7 2003  12:21:41
# command line args: -entry main_vp -profile arbvp1
# nv30vp backend compiling 'main_vp' program
PARAM c13 = { 0.5, 0, 0, 0.5 };
PARAM c14 = { 0, -0.5, 0, 0.5 };
PARAM c12 = { 0, 1, 2, 3 };
PARAM c15 = { 0, 0, 0, 1 };
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbvp1
#program main_vp
#semantic main_vp.worldViewProjMatrix
#semantic main_vp.eyePosition
#semantic main_vp.fresnelBias
#semantic main_vp.fresnelScale
#semantic main_vp.fresnelPower
#semantic main_vp.timeVal
#semantic main_vp.scale
#semantic main_vp.scroll
#semantic main_vp.noise
#var float4 pos : $vin.POSITION : POSITION : 0 : 1
#var float4 normal : $vin.NORMAL : NORMAL : 1 : 1
#var float2 tex : $vin.TEXCOORD0 : TEXCOORD0 : 2 : 1
#var float4 oPos : $vout.POSITION : POSITION : 3 : 1
#var float fresnel : $vout.COLOR : COLOR : 4 : 1
#var float3 noiseCoord : $vout.TEXCOORD0 : TEXCOORD0 : 5 : 1
#var float4 projectionCoord : $vout.TEXCOORD1 : TEXCOORD1 : 6 : 1
#var float4x4 worldViewProjMatrix :  : c[0], 4 : 7 : 1
#var float3 eyePosition :  : c[4] : 8 : 1
#var float fresnelBias :  : c[5] : 9 : 1
#var float fresnelScale :  : c[6] : 10 : 1
#var float fresnelPower :  : c[7] : 11 : 1
#var float timeVal :  : c[8] : 12 : 1
#var float scale :  : c[9] : 13 : 1
#var float scroll :  : c[10] : 14 : 1
#var float noise :  : c[11] : 15 : 1
TEMP R0;
ATTRIB v24 = vertex.texcoord[0];
ATTRIB v18 = vertex.normal;
ATTRIB v16 = vertex.position;
PARAM c6 = program.local[6];
PARAM c5 = program.local[5];
PARAM c7 = program.local[7];
PARAM c4 = program.local[4];
PARAM c11 = program.local[11];
PARAM c9 = program.local[9];
PARAM c10 = program.local[10];
PARAM c8 = program.local[8];
PARAM c0[4] = { program.local[0..3] };
	DP4 R0.x, c0[0], v16;
	DP4 R0.y, c0[1], v16;
	DP4 R0.z, c0[2], v16;
	DP4 R0.w, c0[3], v16;
	MOV result.position, R0;
	DP4 result.texcoord[1].x, c13, R0;
	DP4 result.texcoord[1].y, c14, R0;
	DP4 result.texcoord[1].z, c13.yyxx, R0;
	DP4 result.texcoord[1].w, c15, R0;
	MOV R0.z, c8.xxxy;
	MAD R0.xy, R0.z, c10.x, v24.xyxx;
	MUL result.texcoord[0].xy, R0.xyxx, c9.x;
	MUL result.texcoord[0].z, c11.x, R0.z;
	ADD R0.yzw, v16.xxyz, -c4.xxyz;
	DP3 R0.x, R0.yzwy, R0.yzwy;
	RSQ R0.x, R0.x;
	MUL R0.xyz, R0.x, R0.yzwy;
	DP3 R0.x, R0.xyzx, v18.xyzx;
	ADD R0.xy, c12.y, R0.x;
	MOV R0.zw, c7.x;
	LIT R0.z, R0;
	MOV R0.x, c6;
	MAD result.color.front.primary.x, R0.x, R0.z, c5.x;
END
# 23 instructions
# 1 temp registers
# End of program
