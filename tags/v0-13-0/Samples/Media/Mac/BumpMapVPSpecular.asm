!!ARBvp1.0
# ARB_vertex_program generated by NVIDIA Cg compiler
# cgc version 1.1.0003, build date Jul  7 2003  12:21:41
# command line args: -profile arbvp1 -entry specular_vp
# nv30vp backend compiling 'specular_vp' program
PARAM c6 = { 0, 1, 2, 0 };
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbvp1
#program specular_vp
#semantic specular_vp.lightPosition
#semantic specular_vp.eyePosition
#semantic specular_vp.worldViewProj
#var float4 position : $vin.POSITION : POSITION : 0 : 1
#var float3 normal : $vin.NORMAL : NORMAL : 1 : 1
#var float2 uv : $vin.TEXCOORD0 : TEXCOORD0 : 2 : 1
#var float3 tangent : $vin.TEXCOORD1 : TEXCOORD1 : 3 : 1
#var float4 oPosition : $vout.POSITION : POSITION : 4 : 1
#var float2 oUv : $vout.TEXCOORD0 : TEXCOORD0 : 5 : 1
#var float3 oTSLightDir : $vout.TEXCOORD1 : TEXCOORD1 : 6 : 1
#var float3 oTSHalfAngle : $vout.TEXCOORD2 : TEXCOORD2 : 7 : 1
#var float3 lightPosition :  : c[0] : 8 : 1
#var float3 eyePosition :  : c[1] : 9 : 1
#var float4x4 worldViewProj :  : c[2], 4 : 10 : 1
TEMP R0, R1, R2, R3, R4;
ATTRIB v25 = vertex.texcoord[1];
ATTRIB v24 = vertex.texcoord[0];
ATTRIB v18 = vertex.normal;
ATTRIB v16 = vertex.position;
PARAM c1 = program.local[1];
PARAM c0 = program.local[0];
PARAM c2[4] = { program.local[2..5] };
	MOV result.texcoord[0].xy, v24;
	DP4 result.position.x, c2[0], v16;
	DP4 result.position.y, c2[1], v16;
	DP4 result.position.z, c2[2], v16;
	DP4 result.position.w, c2[3], v16;
	ADD R4.xyz, c0.xyzx, -v16.xyzx;
	MOV R3.xyz, v25;
	DP3 R2.x, R3.xyzx, R4.xyzx;
	MUL R0.xyz, R3.zxyz, v18.yzxy;
	MAD R1.xyz, R3.yzxy, v18.zxyz, -R0.xyzx;
	DP3 R2.y, R1.xyzx, R4.xyzx;
	DP3 R2.z, v18.xyzx, R4.xyzx;
	DP3 R0.x, R2.xyzx, R2.xyzx;
	RSQ R0.x, R0.x;
	MUL result.texcoord[1].xyz, R0.x, R2.xyzx;
	ADD R0.xyz, c1.xyzx, -v16.xyzx;
	ADD R0.yzw, R0.xxyz, R4.xxyz;
	DP3 R0.x, R0.yzwy, R0.yzwy;
	RSQ R0.x, R0.x;
	MUL R0.xyz, R0.x, R0.yzwy;
	DP3 result.texcoord[2].x, R3.xyzx, R0.xyzx;
	DP3 result.texcoord[2].y, R1.xyzx, R0.xyzx;
	DP3 result.texcoord[2].z, v18.xyzx, R0.xyzx;
END
# 23 instructions
# 5 temp registers
# End of program
