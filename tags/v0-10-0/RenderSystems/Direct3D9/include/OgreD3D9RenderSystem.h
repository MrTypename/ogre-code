#ifndef __D3D9RENDERSYSTEM_H__
#define __D3D9RENDERSYSTEM_H__

// Precompiler options
#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"

#include "OgreRenderSystem.h"

// Include D3D files
#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	class D3D9DriverList;
	class D3D9Driver;

	struct HardwareVertexBuffer
	{
		LPDIRECT3DVERTEXBUFFER9 buffer;
		UINT count;
	};

	struct HardwareIndexBuffer
	{
		LPDIRECT3DINDEXBUFFER9 buffer;
		UINT count;
	};

	/**
	Implementation of DirectX9 as a rendering system.
	*/
	class D3D9RenderSystem : public RenderSystem
	{
	private:
		// Direct3D rendering device
		// Only created after top-level window created
		LPDIRECT3D9			mpD3D;
		LPDIRECT3DDEVICE9	mpD3DDevice;
		
		//full-screen multisampling, anti aliasing quality
		DWORD mMultiSampleQuality;
		//external window handle ;)
		HWND mExternalHandle;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D9DriverList* mDriverList;
		// Currently active driver
		D3D9Driver* mActiveD3DDriver;

		D3DCAPS9 mCaps;

		BYTE* mpRenderBuffer;
		DWORD mRenderBufferSize;

		// Vertex buffers.  Currently for rendering we need to place all the data
		// that we receive into one of the following vertex buffers (one for each 
		// component type).
		HardwareVertexBuffer mpXYZBuffer;
		HardwareVertexBuffer mpNormalBuffer;
		HardwareVertexBuffer mpDiffuseBuffer;
		HardwareVertexBuffer mpSpecularBuffer;
		HardwareVertexBuffer mpTextures[OGRE_MAX_TEXTURE_LAYERS][4]; // max 8 textures with max 4 units per texture
		UINT mStreamsInUse;
		HardwareIndexBuffer mpIndicies;

		// With a quick bit of adding up, I cannot see our vertex shader declaration being larger then 26 items
#define D3D_MAX_DECLSIZE 26
		D3DVERTEXELEMENT9 mCurrentDecl[D3D_MAX_DECLSIZE];
		
		LPDIRECT3DVERTEXDECLARATION9 mpCurrentVertexDecl;

		// Array of up to 8 lights, indexed as per API
		// Note that a null value indeicates a free slot
#define MAX_LIGHTS 8
		Light* mLights[MAX_LIGHTS];

		HINSTANCE mhInstance;

		D3D9DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);

		inline bool compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, int size );

		// Matrix conversion
		D3DXMATRIX makeD3DXMatrix( const Matrix4& mat );
		Matrix4 convertD3DXMatrix( const D3DXMATRIX& mat );

		void initConfigOptions(void);
		void initInputDevices(void);
		void processInputDevices(void);
		void setD3D9Light( int index, Light* light );
		
#ifdef _DEBUG
		void DumpBuffer( BYTE* pBuffer, DWORD vertexFormat, unsigned int numVertices, unsigned int stride );
#endif

		D3DCMPFUNC convertCompareFunction(CompareFunction func);
		D3DSTENCILOP convertStencilOp(StencilOperation op);

		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

	public:
		// method for resizing/repositing the render window
 		virtual ResizeRepositionWindow(HWND wich);
		// method for setting external window hwnd
		void SetExternalWindowHandle(HWND externalHandle){mExternalHandle = externalHandle;};
		// method for setting fullscreen multisampling quality
		// must be used before window creation
		void setFullScreenMultiSamplingPasses(DWORD numPasses);

		D3D9RenderSystem( HINSTANCE hInstance );
		~D3D9RenderSystem();

		// ------------------------------------------
		// Overridden RenderSystem functions
		// ------------------------------------------
		const String& getName(void) const;
		ConfigOptionMap& getConfigOptions(void);
		void setConfigOption( const String &name, const String &value );
		String validateConfigOptions(void);
		RenderWindow* initialise( bool autoCreateWindow );
		void reinitialise();
		void shutdown();
		void startRendering();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setTextureFiltering( TextureFilterOptions fo );
		void setLightingEnabled( bool enabled );
		RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth,
			bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
			RenderWindow* parentWindowHandle = 0);
		RenderTexture * createRenderTexture( const String & name, int width, int height );
		void destroyRenderWindow( RenderWindow* pWin );
		String getErrorDescription( long errorNumber );
		void convertColourValue( const ColourValue& colour, unsigned long* pDest );
		// ------------------------------------------
		// Low-level overridden members
		// ------------------------------------------
		void _addLight( Light* lt );
		void _removeLight( Light* lt );
		void _modifyLight( Light* lt );
		void _removeAllLights(void);
		void _pushRenderState(void);
		void _popRenderState(void);
		void _setWorldMatrix( const Matrix4 &m );
		void _setViewMatrix( const Matrix4 &m );
		void _setProjectionMatrix( const Matrix4 &m );
		void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse, const ColourValue &specular,
			const ColourValue &emissive, Real shininess );
		unsigned short _getNumTextureUnits(void);
		void _setTexture( int unit, bool enabled, const String &texname );
		void _setTextureCoordSet( int stage, int index );
		void _setTextureCoordCalculation( int stage, TexCoordCalcMethod m );
		void _setTextureBlendMode( int stage, const LayerBlendModeEx& bm );
		void _setTextureAddressingMode( int stage, Material::TextureLayer::TextureAddressingMode tam );
		void _setTextureMatrix( int stage, const Matrix4 &xform );
		void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor );
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value );
		void _setViewport( Viewport *vp );
		void _beginFrame(void);
		void _render( RenderOperation &op );
		void _endFrame(void);
		void _setCullingMode( CullingMode mode );
		void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
		void _setDepthBufferCheckEnabled( bool enabled = true );
		void _setDepthBufferWriteEnabled(bool enabled = true);
		void _setDepthBufferFunction( CompareFunction func = CMPF_LESS_EQUAL );
		void _setMultiSampleAntiAlias( BOOL set );
	/** See
		RenderSystem
		*/
		void _setDepthBias(ushort bias);
		void _setFog( FogMode mode = FOG_NONE, ColourValue colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
		void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest);
		void _setRasterisationMode(SceneDetailLevel level);
		/** See
		RenderSystem
		*/
		void setStencilCheckEnabled(bool enabled);
		/** See
		RenderSystem
		*/
		bool hasHardwareStencil(void);
		/** See
		RenderSystem
		*/
		ushort getStencilBufferBitDepth(void);
		/** See
		RenderSystem
		*/
		void setStencilBufferFunction(CompareFunction func);
		/** See
		RenderSystem
		*/
		void setStencilBufferReferenceValue(ulong refValue);
		/** See
		RenderSystem
		*/
		void setStencilBufferMask(ulong mask);
		/** See
		RenderSystem
		*/
		void setStencilBufferFailOperation(StencilOperation op);
		/** See
		RenderSystem
		*/
		void setStencilBufferDepthFailOperation(StencilOperation op);
		/** See
		RenderSystem
		*/
		void setStencilBufferPassOperation(StencilOperation op);
	};
}
#endif