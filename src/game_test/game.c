#include "api_engine.h"
#include "api_steam.h"
#include "api_debugtools.h"
#include "util_random.h"
#include "util_json.h"
#include "util_gltf.h"

#ifdef CONFIG_ENABLE_EMBED
IncludeBinary(g_music_ogg, "assets/music.ogg");
IncludeBinary(g_luigi_ogg, "assets/luigi.ogg");
IncludeBinary(g_font_ttf, "assets/Arial.ttf");
IncludeBinary(g_corset_model, "assets/corset.glb");
#endif

static G_GlobalData* game;
static E_GlobalData* engine;

struct G_SnakeState typedef G_SnakeState;
struct G_Scene3DState typedef G_Scene3DState;
struct G_StressState typedef G_StressState;

enum G_GlobalState
{
	G_GlobalState_MainMenu,
	G_GlobalState_Snake,
	G_GlobalState_Scene3D,
	G_GlobalState_Stress,
}
typedef G_GlobalState;

struct G_GlobalData
{
	G_GlobalState state;
	G_GlobalState previous_state;
	E_Font font;
	ArenaSavepoint persistent_arena_save;
	URng_State rng;
	
	E_SoundHandle music;
	E_SoundHandle sound_luigi;
	
	union
	{
		G_SnakeState* snake;
		G_Scene3DState* scene3d;
		G_StressState* stress;
	};
};

#include "game_snake.c"
#include "game_scene3d.c"
#include "game_stress.c"

//~ NOTE(ljre): Main menu implementation
static void
G_Init(void)
{
	Trace();
	
#ifdef CONFIG_ENABLE_STEAM
	if (S_Init(0))
		OS_DebugLog("Steam user nickname: %S\n", S_GetUserNickname());
#endif
	
	game = engine->game = ArenaPushStruct(engine->persistent_arena, G_GlobalData);
	game->previous_state = game->state = G_GlobalState_MainMenu;
	
	URng_Init(&game->rng, OS_CurrentPosixTime());
	
	E_FontDesc desc = {
		.arena = engine->persistent_arena,
		.ttf = { 0 },
		
		.char_height = 32.0f,
		.prebake_ranges = {
			{ 0x21, 0x7E },
			{ 0xA1, 0xFF },
		},
	};
	
#ifdef CONFIG_ENABLE_EMBED
	desc.ttf = BufRange(g_font_ttf_begin, g_font_ttf_end);
#else
	SafeAssert(OS_MapFile(Str("assets/Arial.ttf"), NULL, &desc.ttf));
#endif
	SafeAssert(E_MakeFont(&desc, &game->font));
	
	{
		Buffer ogg;
		bool ok;
		
#ifdef CONFIG_ENABLE_EMBED
		ogg = BufRange(g_music_ogg_begin, g_music_ogg_end);
		ok = true;
#else
		ok = OS_MapFile(Str("assets/music.ogg"), NULL, &ogg);
#endif
		
		ok = ok && E_LoadSound(ogg, &game->music, NULL);
		ok = ok && E_PlaySound(game->music, &(E_PlaySoundOptions) { .volume = 0.5f }, NULL);
		
#ifdef CONFIG_ENABLE_EMBED
		ogg = BufRange(g_luigi_ogg_begin, g_luigi_ogg_end);
		ok = true;
#else
		ok = OS_MapFile(Str("assets/luigi.ogg"), NULL, &ogg);
#endif
		
		ok = ok && E_LoadSound(ogg, &game->sound_luigi, NULL);
	}
	
	game->persistent_arena_save = ArenaSave(engine->persistent_arena);
}

static bool
G_MenuButton(E_RectBatch* batch, float32* inout_y, String text)
{
	Trace();
	
	const float32 button_width = 300.0f;
	const float32 button_height = 100.0f;
	const float32 button_x = (engine->os->window.width - button_width) / 2;
	const float32 mouse_x = engine->os->mouse.pos[0];
	const float32 mouse_y = engine->os->mouse.pos[1];
	
	float32 button_y = *inout_y;
	
	bool is_mouse_pressing = OS_IsDown(engine->os->mouse, OS_MouseButton_Left);
	bool is_mouse_over = true
		&& (mouse_x >= button_x && button_x + button_width  >= mouse_x)
		&& (mouse_y >= button_y && button_y + button_height >= mouse_y);
	
	vec4 color = { 0.3f, 0.3f, 0.3f, 1.0f };
	
	if (is_mouse_over)
	{
		if (is_mouse_pressing)
			color[0] = color[1] = color[2] = 0.2f;
		else
			color[0] = color[1] = color[2] = 0.4f;
	}
	
	E_PushRect(batch, &(E_RectBatchElem) {
		.pos = { button_x, button_y },
		.scaling[0][0] = button_width,
		.scaling[1][1] = button_height,
		.tex_kind = 1,
		.texcoords = { 0, 0, INT16_MAX, INT16_MAX },
		.color = { color[0], color[1], color[2], color[3] },
	});
	
	float32 scale = sinf((float32)OS_GetTimeInSeconds()) * 0.2f + 1.5f;
	
	E_PushText(batch, &game->font, text, vec2(button_x, button_y + button_height*0.25f), vec2(scale, scale), GLM_VEC4_ONE);
	
	button_y += button_height * 1.5f;
	
	*inout_y = button_y;
	return is_mouse_over && OS_IsReleased(engine->os->mouse, OS_MouseButton_Left);
}

static void
G_UpdateAndRender(void)
{
	Trace();
	
#ifdef CONFIG_ENABLE_STEAM
	S_Update();
#endif
	
	if (engine->os->window.should_close || OS_IsPressed(engine->os->keyboard, OS_KeyboardKey_Escape))
		engine->running = false;
	
	RB_BeginCmd(engine->renderbackend, &(RB_BeginDesc) {
		.viewport_width = engine->os->window.width,
		.viewport_height = engine->os->window.height,
	});
	RB_CmdClear(engine->renderbackend, &(RB_ClearDesc) {
		.flag_color = true,
		.flag_depth = true,
		.color = { 0.2f, 0.0f, 0.4f, 1.0f },
	});
	
	if (game->previous_state != game->state)
	{
		switch (game->state)
		{
			case G_GlobalState_MainMenu: ArenaRestore(game->persistent_arena_save); break;
			case G_GlobalState_Snake: G_SnakeInit(); break;
			case G_GlobalState_Scene3D: G_Scene3DInit(); break;
			case G_GlobalState_Stress: G_StressInit(); break;
		}
		
		game->previous_state = game->state;
	}
	
	switch (game->state)
	{
		case G_GlobalState_MainMenu:
		{
			float32 ui_y = 100.0f;
			E_RectBatch batch = {
				.arena = engine->frame_arena,
				.textures[0] = E_WhiteTexture(),
				.textures[1] = game->font.texture,
				.count = 0,
				.elements = ArenaEndAligned(engine->frame_arena, alignof(E_RectBatchElem)),
			};
			
			//- Squares
			static bool hundreds_enabled = false;
			for (int32 i = 0; i < (hundreds_enabled ? 100*1000 : 100); ++i)
			{
				uint64 random = HashInt64(i);
				
				float32 time = (float32)OS_GetTimeInSeconds() * 0.25f + (float32)i;
				float32 xoffset = cosf(time * 5.0f) * 100.0f;
				float32 yoffset = sinf(time * 1.4f) * 100.0f;
				
				float32 x = (random & 511) - 256.0f + xoffset;
				float32 y = (random>>9 & 511) - 256.0f + yoffset;
				float32 angle = (random>>18 & 511) / 512.0f * (float32)Math_PI*2 + time;
				float32 size = 32.0f;
				vec3 color = {
					(random>>27 & 255) / 255.0f,
					(random>>35 & 255) / 255.0f,
					(random>>43 & 255) / 255.0f,
				};
				
				E_PushRect(&batch, &(E_RectBatchElem) {
					.pos = { engine->os->window.width*0.5f + x, engine->os->window.height*0.5f + y },
					.scaling = {
						{ size*cosf(angle), size*-sinf(angle) },
						{ size*sinf(angle), size* cosf(angle) },
					},
					.tex_index = 0,
					.tex_kind = 1,
					.texcoords = { 0, 0, INT16_MAX, INT16_MAX },
					.color = { color[0], color[1], color[2], 1.0f },
				});
			}
			
			//- Buttons
			if (G_MenuButton(&batch, &ui_y, Str("Play Snake")))
				game->state = G_GlobalState_Snake;
			if (G_MenuButton(&batch, &ui_y, Str("Play Scene 3D")))
				game->state = G_GlobalState_Scene3D;
			if (G_MenuButton(&batch, &ui_y, Str("Play Stress")))
				game->state = G_GlobalState_Stress;
			if (G_MenuButton(&batch, &ui_y, Str("Quit")))
				engine->running = false;
			
			float32 scl = 0.5f;
			DBG_UIState debugui = DBG_UIBegin(engine, &batch, &game->font, vec2(50.0f, 50.0f), vec2(scl, scl));
			
			static bool unfolded = false;
			if (DBG_UIPushFoldable(&debugui, Str("Options"), &unfolded))
			{
				if (DBG_UIPushButton(&debugui, Str("Stop music")))
					E_StopAllSounds(&game->music);
				if (DBG_UIPushButton(&debugui, Str("luigi")))
					E_PlaySound(game->sound_luigi, &(E_PlaySoundOptions) { 0 }, NULL);
				if (DBG_UIPushButton(&debugui, Str("test OS_MessageBox")))
					OS_MessageBox(Str("Title Title Title"), Str("Message Message message!!!"));
				if (DBG_UIPushButton(&debugui, hundreds_enabled ? Str("Disable stress") : Str("Enable stress")))
					hundreds_enabled ^= 1;
				
				static uint8 buffer[128] = "Click me!";
				static intsize size = 9;
				static bool selected = false;
				
				DBG_UIPushTextF(&debugui, "Text Input:");
				DBG_UIPushTextField(&debugui, buffer, sizeof(buffer), &size, &selected, 200.0f);
				
				static bool caps_unfolded = false;
				if (DBG_UIPushFoldable(&debugui, Str("RenderBackend Capabilities"), &caps_unfolded))
				{
					RB_Capabilities caps = RB_QueryCapabilities(engine->renderbackend);
					
					const char* shader_type = "";
					switch (caps.shader_type)
					{
						case 0: shader_type = ""; break;
						case RB_ShaderType_Glsl: shader_type = "GLSL 3.3 / ES 3.0"; break;
						case RB_ShaderType_Hlsl40Level91: shader_type = "HLSL vs/ps_4_0_level_9_1"; break;
						case RB_ShaderType_Hlsl40Level93: shader_type = "HLSL vs/ps_4_0_level_9_3"; break;
						case RB_ShaderType_Hlsl40: shader_type = "HLSL vs/ps_4_0"; break;
					}
					
					DBG_UIPushTextF(&debugui, "API: %S\nDriver Renderer: %S\nDriver Vendor: %S\nDriver Version: %S", caps.backend_api, caps.driver_renderer, caps.driver_vendor, caps.driver_version);
					DBG_UIPushTextF(&debugui, "shader_type: %s", shader_type);
					
#define _CapInt(Field) DBG_UIPushTextF(&debugui, #Field ": %i", caps.Field)
#define _CapBool(Field) if (caps.Field) DBG_UIPushTextF(&debugui, "Supported: " #Field)
					
					_CapInt(max_texture_size);
					_CapInt(max_render_target_textures);
					_CapInt(max_textures_per_drawcall);
					_CapBool(has_instancing);
					_CapBool(has_32bit_index);
					_CapBool(has_separate_alpha_blend);
					_CapBool(has_compute_shaders);
					_CapBool(has_f16_formats);
					_CapBool(has_f16_shader_ops);
					
#undef _CapInt
#undef _CapBool
					
					DBG_UIPopFoldable(&debugui);
				}
				
				static bool arenas_unfolded = false;
				if (DBG_UIPushFoldable(&debugui, Str("Arenas Info"), &arenas_unfolded))
				{
					DBG_UIPushArenaInfo(&debugui, engine->persistent_arena, Str("Persistent"));
					DBG_UIPushArenaInfo(&debugui, engine->scratch_arena, Str("Scratch"));
					DBG_UIPushArenaInfo(&debugui, engine->frame_arena, Str("Frame"));
					DBG_UIPushArenaInfo(&debugui, engine->audio_thread_arena, Str("Audio Thread"));
					
					DBG_UIPopFoldable(&debugui);
				}
				
				static bool audiodev_unfolded = false;
				if (DBG_UIPushFoldable(&debugui, Str("Audio Devices"), &audiodev_unfolded))
				{
					DBG_UIPushTextF(&debugui, "Current playing device ID: %u", engine->os->audio.current_device_id);
					DBG_UIPushTextF(&debugui, "Sample Rate: %i", engine->os->audio.mix_sample_rate);
					DBG_UIPushTextF(&debugui, "Channels: %i", engine->os->audio.mix_channels);
					DBG_UIPushTextF(&debugui, "Frame Pull Rate: %i", engine->os->audio.mix_frame_pull_rate);
					DBG_UIPushVerticalSpacing(&debugui, 24.0f);
					DBG_UIPushTextF(&debugui, "Devices:");
					
					static bool devs_unfolded[32];
					for (int32 i = 0; i < Min(engine->os->audio.device_count, ArrayLength(devs_unfolded)); ++i)
					{
						const OS_AudioDeviceInfo* dev = &engine->os->audio.devices[i];
						String text = StringPrintfLocal(256, "ID %u", dev->id);
						
						if (DBG_UIPushFoldable(&debugui, text, &devs_unfolded[i]))
						{
							DBG_UIPushTextF(&debugui, "Name: %S\nDescription: %S\nInterface: %S", dev->name, dev->description, dev->interface_name);
							
							DBG_UIPopFoldable(&debugui);
						}
					}
					
					DBG_UIPopFoldable(&debugui);
				}
				
				static bool timing_unfolded = false;
				if (DBG_UIPushFoldable(&debugui, Str("Timings"), &timing_unfolded))
				{
					uint64 frequency;
					OS_CurrentTick(&frequency);
					
					DBG_UIPushTextF(&debugui, "Raw CPU frame delta (ms): %.2f", (float64)engine->raw_frame_tick_delta / (frequency/1000));
					DBG_UIPushTextF(&debugui, "Delta time: %f", engine->delta_time);
					DBG_UIPushVerticalSpacing(&debugui, 24.0f);
					DBG_UIPushTextF(&debugui, "Frame Snap History:");
					
					for (intsize i = 0; i < ArrayLength(engine->frame_snap_history); ++i)
						DBG_UIPushTextF(&debugui, " - %i", engine->frame_snap_history[i]);
					
					DBG_UIPopFoldable(&debugui);
				}
				
				DBG_UIPopFoldable(&debugui);
			}
			
			DBG_UIEnd(&debugui);
			
			E_DrawRectBatch(&batch, NULL);
		} break;
		
		case G_GlobalState_Snake: G_SnakeUpdateAndRender(); break;
		case G_GlobalState_Scene3D: G_Scene3DUpdateAndRender(); break;
		case G_GlobalState_Stress: G_StressUpdateAndRender(); break;
	}
	
	if (!engine->running)
	{
		E_UnloadSound(game->sound_luigi);
		E_UnloadSound(game->music);
	}
	
	RB_EndCmd(engine->renderbackend);
	E_FinishFrame();
}

API void
G_Main(E_GlobalData* data)
{
	Trace();
	
	engine = data;
	game = data->game;
	
	if (!game)
		G_Init();
	
	for ArenaTempScope(data->scratch_arena)
		G_UpdateAndRender();
}
