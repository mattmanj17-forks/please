#ifndef INTERNAL_API_RENDER_H
#define INTERNAL_API_RENDER_H

struct Engine_RendererCamera
{
	vec3 pos;
	
	union
	{
		// NOTE(ljre): 2D Stuff
		struct { vec2 size; float32 zoom; float32 angle; };
		
		// NOTE(ljre): 3D Stuff
		struct { vec3 dir; vec3 up; };
	};
}
typedef Engine_RendererCamera;

struct Engine_Renderer3DPointLight
{
	vec3 position;
	
	float32 constant, linear, quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}
typedef Engine_Renderer3DPointLight;

struct Engine_Renderer3DFlashlight
{
	vec3 position;
	vec3 direction;
	vec3 color;
	
	float32 constant, linear, quadratic;
	
	float32 inner_cutoff;
	float32 outer_cutoff;
}
typedef Engine_Renderer3DFlashlight;

struct Engine_Renderer3DModel
{
	mat4 transform;
	Asset_3DModel* asset;
	vec4 color;
}
typedef Engine_Renderer3DModel;

struct Engine_Renderer3DScene
{
	Asset_3DModel* light_model;
	vec3 dirlight;
	vec3 dirlight_color;
	
	uint32 shadow_fbo, shadow_depthmap;
	uint32 gbuffer, gbuffer_pos, gbuffer_norm, gbuffer_albedo, gbuffer_depth;
	
	int32 model_count;
	int32 point_light_count;
	int32 flashlights_count;
	
	Engine_Renderer3DModel* models;
	Engine_Renderer3DPointLight* point_lights;
	Engine_Renderer3DFlashlight* flashlights;
}
typedef Engine_Renderer3DScene;

struct Engine_Renderer2DSprite
{
	mat4 transform;
	vec4 texcoords;
	vec4 color;
}
typedef Engine_Renderer2DSprite;

struct Engine_Renderer2DLayer
{
	Asset_Texture* texture;
	
	int32 sprite_count;
	Engine_Renderer2DSprite* sprites;
}
typedef Engine_Renderer2DLayer;

API void Engine_CalcViewMatrix2D(const Engine_RendererCamera* camera, mat4 out_view);
API void Engine_CalcViewMatrix3D(const Engine_RendererCamera* camera, mat4 out_view, float32 fov, float32 aspect);
API void Engine_CalcModelMatrix2D(const vec2 pos, const vec2 scale, float32 angle, mat4 out_view);
API void Engine_CalcModelMatrix3D(const vec3 pos, const vec3 scale, const vec3 rot, mat4 out_view);
API void Engine_CalcPointInCamera2DSpace(const Engine_RendererCamera* camera, const vec2 pos, vec2 out_pos);

struct Engine_RendererApi
{
	bool (*load_font_from_file)(String path, Asset_Font* out_font);
	bool (*load_3dmodel_from_file)(String path, Asset_3DModel* out_model);
	bool (*load_texture_from_file)(String path, Asset_Texture* out_texture);
	bool (*load_texture_array_from_file)(String path, Asset_Texture* out_texture, int32 cell_width, int32 cell_height);
	
	void (*clear_background)(float32 r, float32 g, float32 b, float32 a);
	void (*begin)(void);
	void (*draw_rectangle)(vec4 color, vec3 pos, vec3 size, vec3 alignment);
	void (*draw_texture)(const Asset_Texture* texture, const mat4 transform, const mat4 view, const vec4 color);
	void (*draw_text)(const Asset_Font* font, String text, const vec3 pos, float32 char_height, const vec4 color, const vec3 alignment);
	
	void (*draw_3dscene)(Engine_Renderer3DScene* scene, const Engine_RendererCamera* camera);
	void (*draw_2dlayer)(const Engine_Renderer2DLayer* layer, const Engine_RendererCamera* camera);
}
typedef Engine_RendererApi;

#endif //INTERNAL_API_RENDER_H
