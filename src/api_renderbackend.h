#ifndef API_RENDERBACKEND_H
#define API_RENDERBACKEND_H

enum
{
	RB_Limits_SamplersPerDrawCall = 4,
	RB_Limits_InputsPerShader = 8,
	RB_Limits_ColorAttachPerRenderTarget = 4,
};

union RB_Handle
{ uint32 id; }
typedef RB_Handle;

//~ ResourceCommand
enum RB_LayoutDescKind
{
	RB_LayoutDescKind_Null = 0,
	
	RB_LayoutDescKind_Float,
	RB_LayoutDescKind_Vec2,
	RB_LayoutDescKind_Vec3,
	RB_LayoutDescKind_Vec4,
	RB_LayoutDescKind_Mat2,
	RB_LayoutDescKind_Mat3,
	RB_LayoutDescKind_Mat4,
}
typedef RB_LayoutDescKind;

struct RB_LayoutDesc
{
	RB_LayoutDescKind kind;
	uint32 offset;
	
	uint8 divisor;
	uint8 vbuffer_index;
	uint8 gl_location;
}
typedef RB_LayoutDesc;

enum RB_ResourceCommandKind
{
	RB_ResourceCommandKind_Null = 0,
	
	RB_ResourceCommandKind_MakeTexture2D,
	RB_ResourceCommandKind_MakeVertexBuffer,
	RB_ResourceCommandKind_MakeIndexBuffer,
	RB_ResourceCommandKind_MakeUniformBuffer,
	RB_ResourceCommandKind_MakeShader,
	RB_ResourceCommandKind_MakeRenderTarget,
	//
	RB_ResourceCommandKind_UpdateVertexBuffer,
	RB_ResourceCommandKind_UpdateIndexBuffer,
	RB_ResourceCommandKind_UpdateUniformBuffer,
	RB_ResourceCommandKind_UpdateTexture2D,
	//
	RB_ResourceCommandKind_FreeTexture2D,
	RB_ResourceCommandKind_FreeVertexBuffer,
	RB_ResourceCommandKind_FreeIndexBuffer,
	RB_ResourceCommandKind_FreeUniformBuffer,
	RB_ResourceCommandKind_FreeShader,
	RB_ResourceCommandKind_FreeRenderTarget,
}
typedef RB_ResourceCommandKind;

struct RB_ResourceCommand typedef RB_ResourceCommand;
struct RB_ResourceCommand
{
	RB_ResourceCommand* next;
	RB_ResourceCommandKind kind;
	bool flag_dynamic : 1;
	bool flag_subregion : 1;
	
	RB_Handle* handle;
	
	union
	{
		struct
		{
			const void* pixels;
			int32 width, height;
			uint32 channels;
			
			bool flag_linear_filtering : 1;
			
			// used if 'flag_subregion' is set and not on init.
			int32 xoffset;
			int32 yoffset;
		}
		texture_2d;
		
		struct
		{
			Buffer d3d_vs_blob, d3d_ps_blob;
			String gl_vs_src, gl_fs_src;
			
			RB_LayoutDesc input_layout[RB_Limits_InputsPerShader];
		}
		shader;
		
		struct
		{
			const void* memory;
			uintsize size;
			
			// used if 'flag_subregion' is set and not on init.
			uintsize offset;
		}
		buffer;
		
		struct
		{
			RB_Handle* color_textures[RB_Limits_ColorAttachPerRenderTarget];
			RB_Handle* depth_stencil_texture;
		}
		render_target;
	};
};

//~ DrawCommand
struct RB_SamplerDesc
{
	RB_Handle* handle;
}
typedef RB_SamplerDesc;

enum RB_DrawCommandKind
{
	RB_DrawCommandKind_Null = 0,
	
	RB_DrawCommandKind_Clear,
	RB_DrawCommandKind_SetRenderTarget,
	RB_DrawCommandKind_ResetRenderTarget,
	RB_DrawCommandKind_DrawCall,
}
typedef RB_DrawCommandKind;

struct RB_DrawCommand typedef RB_DrawCommand;
struct RB_DrawCommand
{
	RB_DrawCommand* next;
	RB_ResourceCommand* resources_cmd;
	RB_DrawCommandKind kind;
	
	union
	{
		struct
		{
			float32 color[4];
			
			bool flag_color : 1;
			bool flag_depth : 1;
			bool flag_stencil : 1;
		}
		clear;
		
		struct
		{
			RB_Handle* handle;
		}
		set_target;
		
		struct
		{
			RB_Handle* shader;
			RB_Handle* ibuffer;
			RB_Handle* ubuffer;
			RB_Handle* vbuffers[4];
			uint32 vbuffer_strides[4];
			
			uint32 index_count;
			uint32 instance_count;
			
			RB_SamplerDesc samplers[RB_Limits_SamplersPerDrawCall];
		}
		drawcall;
	};
};

API void RB_Init(Arena* scratch_arena, const OS_WindowGraphicsContext* graphics_context);
API void RB_Deinit(Arena* scratch_arena);
API bool RB_Present(Arena* scratch_arena, int32 vsync_count);

API void RB_ExecuteResourceCommands(Arena* scratch_arena, RB_ResourceCommand* commands);
API void RB_ExecuteDrawCommands(Arena* scratch_arena, RB_DrawCommand* commands, int32 default_width, int32 default_height);

#endif //API_RENDERBACKEND_H
