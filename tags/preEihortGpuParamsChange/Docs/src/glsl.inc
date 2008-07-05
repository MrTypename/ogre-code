@node GLSL
@subsection OpenGL GLSL
OpenGL GLSL has a similar language syntax to HLSL but is tied to the OpenGL API. The are a few benefits over Cg in that it only requires the OpenGL render system plugin, not any additional plugins. Declaring a OpenGL GLSL program is similar to Cg but simpler. Here's an example:
@example
vertex_program myGLSLVertexProgram glsl
{
    source myGLSLVertexProgram.txt
}
@end example
In GLSL, no entry point needs to be defined since it is always 'main()' and there is no target definition since GLSL source is compiled into native GPU code and not intermediate assembly. @*@*

GLSL supports the use of modular shaders.  This means you can write GLSL external functions that can be used in multiple shaders.

@example
vertex_program myExteranalGLSLFunction1 glsl
{
    source myExternalGLSLfunction1.txt
}

vertex_program myExteranalGLSLFunction2 glsl
{
    source myExternalGLSLfunction2.txt
}

vertex_program myGLSLVertexProgram1 glsl
{
    source myGLSLfunction.txt
    attach myExteranalGLSLFunction1 myExteranalGLSLFunction2
}

vertex_program myGLSLVertexProgram2 glsl
{
    source myGLSLfunction.txt
    attach myExteranalGLSLFunction1
}
@end example

External GLSL functions are attached to the program that needs them by using 'attach' and including the names of all external programs required on the same line seperated by spaces.  This can be done for both vertex and fragment programs.

@subheading GLSL Texture Samplers
To pass texture unit index values from the material script to texture samplers in glsl use 'int' type named parameters.  See the example below:@*

excerpt from GLSL example.frag source:
@example
varying vec2 UV;
uniform sampler2D diffuseMap;

void main(void)
{
	gl_FragColor = texture2D(diffuseMap, UV);
}
@end example

In material script:
@example
fragment_program myFragmentShader glsl
{
  source example.frag
}

material exampleGLSLTexturing
{
  technique
  {
    pass
    {
      fragment_program_ref myFragmentShader
      { 
        param_named diffuseMap int 0
      }

      texture_unit 
      {
        texture myTexture.jpg 2d
      }
    }
  }
}
@end example

An index value of 0 refers to the first texture unit in the pass, an index value of 1 refers to the second unit in the pass and so on.@*@*

@subheading Matrix parameters

Here are some examples of passing matrices to GLSL mat2, mat3, mat4 uniforms:

@example
material exampleGLSLmatixUniforms
{
  technique matrix_passing
  {
    pass examples
    {
      vertex_program_ref myVertexShader
      { 
        // mat4 uniform
        param_named OcclusionMatrix matrix4x4 1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 0 
        // or
        param_named ViewMatrix float16 0 1 0 0  0 0 1 0  0 0 0 1  0 0 0 0 
        
        // mat3
        param_named TextRotMatrix float9 1 0 0  0 1 0  0 0 1  
      }
      
      fragment_program_ref myFragmentShader
      { 
        // mat2 uniform
        param_named skewMatrix float4 0.5 0 -0.5 1.0
      }

    }
  }
}
@end example

@subheading Accessing OpenGL states in GLSL
GLSL can access most of the GL states directly so you do not need to pass these states through @ref{param_named_auto} in the material script.  This includes lights, material state, and all the matrices used in the openGL state ie model view matrix, worldview projection matrix etc. @*@*

@subheading Binding vertex attributes
GLSL natively supports automatic binding of the most common incoming per-vertex attributes (e.g. gl_Vertex, gl_Normal, gl_MultiTexCoord0 etc). However, there are some which are not automatically bound, which must be declared in the shader using the 'attribute <type> <name>' syntax, and the vertex data bound to it by Ogre. @*@*

In addition to the built in attributes described in section 7.3 of the GLSL manual, Ogre supports the following automatically bound attributes:
@table @asis
@item tangent
If you declare an attribute as 'attribute vec3 tangent;' at the top of your GLSL vertex program, Ogre will automatically bind vertex elements of type VES_TANGENT to it. Note that for maximum compatibility with shader versions, by default tangents are placed in a spare texture coordinate by default rather than a tangent element, but you can control this using OgreXmlConverter / OgreMeshUpgrader, or via your exporter.
@item binormal
If you declare an attribute as 'attribute vec3 binormal;' at the top of your GLSL vertex program, Ogre will automatically bind vertex elements of type VES_BINORMAL to it. Note that none of the Ogre tools produce binormals since they can be easily calculated from normals and tangents in a vertex program.
@item blendIndices
If you declare an attribute as 'attribute vec4 blendIndices;' at the top of your GLSL vertex program, Ogre will automatically bind vertex elements of type VES_BLEND_INDICES to it. 
@item blendWeights
If you declare an attribute as 'attribute vec4 blendWeights;' at the top of your GLSL vertex program, Ogre will automatically bind vertex elements of type VES_BLEND_WEIGHTS to it. 


