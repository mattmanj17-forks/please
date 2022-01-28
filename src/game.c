#include "internal.h"

#if 0
#   include "game_jam.c"
#else
enum Game_State
{
	Game_State_Menu,
	Game_State_3DDemo,
}
typedef Game_State;

struct Game_Data
{
	Asset_Font font;
	Game_State state;
	
	//~ NOTE(ljre): Game_3DDemoScene
	Asset_3DModel model;
	Asset_3DModel tree_model;
	Asset_3DModel corset_model;
	
	Asset_SoundBuffer sound_music;
	Asset_SoundBuffer sound_music3;
	
	Engine_PlayingAudio playing_audios[16];
	int32 playing_audio_count;
	
	int32 controller_index;
	
	float32 camera_yaw;
	float32 camera_pitch;
	float32 sensitivity;
	float32 camera_total_speed;
	float32 camera_height;
	float32 moving_time;
	vec2 camera_speed;
	
	Render_Camera render_camera;
	
	// NOTE(ljre): Models to render
	Render_3DModel* render_models;
	Render_3DPointLight lights[2];
	Render_3DModel* render_rotating_cubes;
	int32 rotating_cube_count;
	
	Render_3DScene render_scene;
};

internal void
ColorToVec4(uint32 color, vec4 out)
{
	out[0] = (float32)(color>>16 & 0xFF) / 255.0f;
	out[1] = (float32)(color>>8 & 0xFF) / 255.0f;
	out[2] = (float32)(color & 0xFF) / 255.0f;
	out[3] = (float32)(color>>24 & 0xFF) / 255.0f;
}

internal void
DrawGamepadLayout(const Input_Gamepad* gamepad, float32 x, float32 y, float32 width, float32 height)
{
	vec3 alignment = { -0.5f, -0.5f };
	vec4 black = { 0.1f, 0.1f, 0.1f, 1.0f };
	vec4 colors[2] = {
		{ 0.5f, 0.5f, 0.5f, 1.0f }, // Disabled
		{ 0.9f, 0.9f, 0.9f, 1.0f }, // Enabled
	};
	
	float32 xscale = width / 600.0f;
	float32 yscale = height / 600.0f;
	
	// NOTE(ljre): Draw Axes
	{
		Render_DrawRectangle(black,
							 (vec3) { x + width / 2.0f - 100.0f * xscale, y + height / 2.0f + 100.0f * yscale },
							 (vec3) { 50.0f * xscale, 50.0f * yscale }, alignment);
		
		Render_DrawRectangle(colors[Input_IsDown(*gamepad, Input_GamepadButton_LS)],
							 (vec3) { x + width / 2.0f - 100.0f * xscale + gamepad->left[0] * 20.0f * xscale,
								 y + height / 2.0f + 100.0f * yscale + gamepad->left[1] * 20.0f * yscale },
							 (vec3) { 20.0f * xscale, 20.0f * yscale }, alignment);
		
		Render_DrawRectangle(black,
							 (vec3) { x + width / 2.0f + 100.0f * xscale, y + height / 2.0f + 100.0f * yscale },
							 (vec3) { 50.0f * xscale, 50.0f * yscale }, alignment);
		
		Render_DrawRectangle(colors[Input_IsDown(*gamepad, Input_GamepadButton_RS)],
							 (vec3) { x + width / 2.0f + 100.0f * xscale + gamepad->right[0] * 20.0f * xscale, y + height / 2.0f + 100.0f * yscale + gamepad->right[1] * 20.0f * yscale },
							 (vec3) { 20.0f * xscale, 20.0f * yscale }, alignment);
	}
	
	// NOTE(ljre): Draw Buttons
	{
		struct Pair
		{
			vec3 pos;
			vec3 size;
		} buttons[] = {
			[Input_GamepadButton_Y] = { { width * 0.84f, height * 0.44f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_B] = { { width * 0.90f, height * 0.50f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_A] = { { width * 0.84f, height * 0.56f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_X] = { { width * 0.78f, height * 0.50f }, { 30.0f, 30.0f } },
			
			[Input_GamepadButton_LB] = { { width * 0.20f, height * 0.30f }, { 60.0f, 20.0f } },
			[Input_GamepadButton_RB] = { { width * 0.80f, height * 0.30f }, { 60.0f, 20.0f } },
			
			[Input_GamepadButton_Up]= { { width * 0.16f, height * 0.44f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_Right] = { { width * 0.22f, height * 0.50f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_Down]  = { { width * 0.16f, height * 0.56f }, { 30.0f, 30.0f } },
			[Input_GamepadButton_Left]  = { { width * 0.10f, height * 0.50f }, { 30.0f, 30.0f } },
			
			[Input_GamepadButton_Back]  = { { width * 0.45f, height * 0.48f }, { 25.0f, 15.0f } },
			[Input_GamepadButton_Start] = { { width * 0.55f, height * 0.48f }, { 25.0f, 15.0f } },
		};
		
		for (int32 i = 0; i < ArrayLength(buttons); ++i)
		{
			buttons[i].pos[0] += x;
			buttons[i].pos[1] += y;
			buttons[i].size[0] *= xscale;
			buttons[i].size[1] *= yscale;
			
			Render_DrawRectangle(colors[Input_IsDown(*gamepad, i)], buttons[i].pos, buttons[i].size, alignment);
		}
	}
	
	// NOTE(ljre): Draw Triggers
	{
		vec4 color;
		
		glm_vec4_lerp(colors[0], colors[1], gamepad->lt, color);
		Render_DrawRectangle(color,
							 (vec3) { x + width * 0.20f, y + height * 0.23f },
							 (vec3) { 60.0f * xscale, 30.0f * yscale }, alignment);
		
		glm_vec4_lerp(colors[0], colors[1], gamepad->rt, color);
		Render_DrawRectangle(color,
							 (vec3) { x + width * 0.80f, y + height * 0.23f },
							 (vec3) { 60.0f * xscale, 30.0f * yscale }, alignment);
	}
}

internal void
Game_3DDemoScene(Engine_Data* g, bool32 needs_init)
{
	Trace("Game_3DDemoScene");
	
	if (needs_init)
	{
		if (!Render_Load3DModelFromFile(Str("./assets/cube.glb"), &g->game->model))
			Platform_ExitWithErrorMessage(Str("Could not load cube located in 'assets/cube.sobj'. Are you running this program on the build directory or repository directory? Make sure it's on the repository one."));
		if (!Render_Load3DModelFromFile(Str("./assets/first_tree.glb"), &g->game->tree_model))
			Platform_ExitWithErrorMessage(Str("Could not load tree model :("));
		if (!Render_Load3DModelFromFile(Str("./assets/corset.glb"), &g->game->corset_model))
			Platform_ExitWithErrorMessage(Str("Could not load corset model :("));
		
		// NOTE(ljre): Load Audio
		Engine_LoadSoundBuffer(Str("music.ogg"), &g->game->sound_music);
		Engine_LoadSoundBuffer(Str("music3.ogg"), &g->game->sound_music3);
		
		if (g->game->sound_music.samples)
		{
			g->game->playing_audios[0] = (Engine_PlayingAudio) {
				.sound = &g->game->sound_music,
				.frame_index = -1,
				.loop = true,
				.volume = 1.0f,
				.speed = 1.0f,
			};
			
			g->game->playing_audio_count = 1;
		}
		
		g->game->controller_index = 0;
		
		g->game->camera_yaw = 0.0f;
		g->game->camera_pitch = 0.0f;
		g->game->sensitivity = 0.05f;
		g->game->camera_total_speed = 0.075f;
		g->game->camera_height = 1.9f;
		g->game->moving_time = 0.0f;
		glm_vec2_zero(g->game->camera_speed);
		
		g->game->render_camera = (Render_Camera) {
			.pos = { 0.0f, g->game->camera_height, 0.0f },
			.dir = { 0.0f, 0.0f, -1.0f },
			.up = { 0.0f, 1.0f, 0.0f },
		};
		
		g->game->render_models = Arena_Push(g->persistent_arena, sizeof(*g->game->render_models) * 42);
		
		//- Ground
		Render_3DModel* ground = &g->game->render_models[0];
		
		glm_mat4_identity(ground->transform);
		glm_scale(ground->transform, (vec3) { 20.0f, 2.0f, 20.0f });
		glm_translate(ground->transform, (vec3) { 0.0, -1.0f, 0.0f });
		ground->color[0] = 1.0f;
		ground->color[1] = 1.0f;
		ground->color[2] = 1.0f;
		ground->color[3] = 1.0f;
		ground->asset = &g->game->model;
		
		//- Tree
		Render_3DModel* tree = &g->game->render_models[1];
		
		glm_mat4_identity(tree->transform);
		glm_translate(tree->transform, (vec3) { -1.0f });
		tree->asset = &g->game->tree_model;
		ColorToVec4(0xFFFFFFFF, tree->color);
		
		//- Corset
		for (int32 i = 0; i < 10; ++i) {
			Render_3DModel* corset = &g->game->render_models[2 + i];
			
			vec3 pos = {
				Engine_RandomF32Range(-5.0f, 5.0f),
				0.1f,
				Engine_RandomF32Range(-5.0f, 5.0f),
			};
			
			glm_mat4_identity(corset->transform);
			glm_translate(corset->transform, pos);
			
			corset->asset = &g->game->corset_model;
			ColorToVec4(0xFFFFFFFF, corset->color);
		}
		
		//- Rotating Cubes
		Render_3DModel* rotating_cubes = &g->game->render_models[12];
		g->game->rotating_cube_count = 30;
		g->game->render_rotating_cubes = rotating_cubes;
		for (int32 i = 0; i < g->game->rotating_cube_count; ++i)
		{
			Render_3DModel* cube = &rotating_cubes[i];
			
			vec3 pos = {
				Engine_RandomF32Range(-20.0f, 20.0f),
				Engine_RandomF32Range(3.0f, 10.0f),
				Engine_RandomF32Range(-20.0f, 20.0f),
			};
			
			glm_mat4_identity(cube->transform);
			glm_translate(cube->transform, pos);
			cube->asset = &g->game->model;
			
			static const uint32 colors[] = {
				0xFFFF0000, 0xFFFF3E00, 0xFFFF8100, 0xFFFFC100, 0xFFFFFF00, 0xFFC1FF00,
				0xFF81FF00, 0xFF3EFF00, 0xFF00FF00, 0xFF00FF3E, 0xFF00FF81, 0xFF00FFC1,
				0xFF00FFFF, 0xFF00C1FF, 0xFF0081FF, 0xFF003EFF, 0xFF0000FF, 0xFF3E00FF,
				0xFF8100FF, 0xFFC100FF, 0xFFFF00FF, 0xFFFF00C1, 0xFFFF0081, 0xFFFF003E,
			};
			
			ColorToVec4(colors[Engine_RandomU64() % ArrayLength(colors)], cube->color);
		}
		
		//- Lights
		g->game->lights[0] = (Render_3DPointLight) {
			.position[0] = 1.0f,
			.position[1] = 2.0f,
			.position[2] = 5.0f,
			.ambient[0] = 0.2f,
			.ambient[1] = 0.0f,
			.ambient[2] = 0.0f,
			.diffuse[0] = 0.8f,
			.diffuse[1] = 0.0f,
			.diffuse[2] = 0.0f,
			.specular[0] = 1.0f,
			.specular[1] = 0.0f,
			.specular[2] = 0.0f,
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		};
		
		g->game->lights[1] = (Render_3DPointLight) {
			.position[0] = 1.0f,
			.position[1] = 2.0f,
			.position[2] = -5.0f,
			.ambient[0] = 0.0f,
			.ambient[1] = 0.0f,
			.ambient[2] = 1.0f,
			.diffuse[0] = 0.0f,
			.diffuse[1] = 0.0f,
			.diffuse[2] = 1.0f,
			.specular[0] = 0.0f,
			.specular[1] = 0.0f,
			.specular[2] = 0.8f,
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		};
		
		// NOTE(ljre): Render Command
		g->game->render_scene = (Render_3DScene) {
			.dirlight[0] = 1.0f,
			.dirlight[1] = 2.0f, // TODO(ljre): discover why tf this needs to be positive
			.dirlight[2] = 0.5f,
			
			.dirlight_color[0] = 0.2f,
			.dirlight_color[1] = 0.2f,
			.dirlight_color[2] = 0.3f,
			
			.light_model = &g->game->model,
			.point_lights = g->game->lights,
			.models = g->game->render_models,
			.point_light_count = ArrayLength(g->game->lights),
			.model_count = 42,
		};
		
		glm_vec3_normalize(g->game->render_scene.dirlight);
	}
	
	//~ NOTE(ljre): 3D Scene
	Trace("Game Loop");
	void* memory_state = Arena_End(g->temp_arena);
	
	if (Platform_WindowShouldClose() || Input_KeyboardIsPressed(Input_KeyboardKey_Escape))
		g->running = false;
	
	if (Input_KeyboardIsPressed(Input_KeyboardKey_Left))
		g->game->controller_index = (g->game->controller_index - 1) & 15;
	else if (Input_KeyboardIsPressed(Input_KeyboardKey_Right))
		g->game->controller_index = (g->game->controller_index + 1) & 15;
	
	if (g->game->sound_music3.samples &&
		g->game->playing_audio_count < ArrayLength(g->game->playing_audios) &&
		Input_KeyboardIsPressed(Input_KeyboardKey_Space))
	{
		g->game->playing_audios[g->game->playing_audio_count++] = (Engine_PlayingAudio) {
			.sound = &g->game->sound_music3,
			.frame_index = -1,
			.loop = false,
			.volume = 1.0f,
			.speed = 1.5f,
		};
	}
	
	Input_Gamepad gamepad;
	bool32 is_connected = Input_GetGamepad(g->game->controller_index, &gamepad);
	
	float32 dt = g->delta_time;
	
	if (is_connected)
	{
		//~ NOTE(ljre): Camera Direction
		if (gamepad.right[0] != 0.0f)
		{
			g->game->camera_yaw += gamepad.right[0] * g->game->sensitivity * dt;
			g->game->camera_yaw = fmodf(g->game->camera_yaw, PI32 * 2.0f);
		}
		
		if (gamepad.right[1] != 0.0f)
		{
			g->game->camera_pitch += -gamepad.right[1] * g->game->sensitivity * dt;
			g->game->camera_pitch = glm_clamp(g->game->camera_pitch, -PI32 * 0.49f, PI32 * 0.49f);
		}
		
		float32 pitched = cosf(g->game->camera_pitch);
		
		g->game->render_camera.dir[0] = cosf(g->game->camera_yaw) * pitched;
		g->game->render_camera.dir[1] = sinf(g->game->camera_pitch);
		g->game->render_camera.dir[2] = sinf(g->game->camera_yaw) * pitched;
		glm_vec3_normalize(g->game->render_camera.dir);
		
		vec3 right;
		glm_vec3_cross((vec3) { 0.0f, 1.0f, 0.0f }, g->game->render_camera.dir, right);
		
		glm_vec3_cross(g->game->render_camera.dir, right, g->game->render_camera.up);
		
		//~ Movement
		vec2 speed;
		
		speed[0] = -gamepad.left[1];
		speed[1] =  gamepad.left[0];
		
		glm_vec2_rotate(speed, g->game->camera_yaw, speed);
		glm_vec2_scale(speed, g->game->camera_total_speed, speed);
		glm_vec2_lerp(g->game->camera_speed, speed, 0.2f * dt, g->game->camera_speed);
		
		g->game->render_camera.pos[0] += g->game->camera_speed[0] * dt;
		g->game->render_camera.pos[2] += g->game->camera_speed[1] * dt;
		
		if (Input_IsDown(gamepad, Input_GamepadButton_A))
			g->game->camera_height += 0.05f * dt;
		else if (Input_IsDown(gamepad, Input_GamepadButton_B))
			g->game->camera_height -= 0.05f * dt;
		
		//~ Bump
		float32 d = glm_vec2_distance2(g->game->camera_speed, GLM_VEC2_ZERO);
		if (d > 0.0001f)
			g->game->moving_time += 0.3f * dt;
		
		g->game->render_camera.pos[1] = g->game->camera_height + sinf(g->game->moving_time) * 0.04f;
	}
	
	if (is_connected)
		Render_ClearBackground(0.1f, 0.15f, 0.3f, 1.0f);
	else
		Render_ClearBackground(0.0f, 0.0f, 0.0f, 1.0f);
	
	//~ NOTE(ljre): 3D World
	float32 t = (float32)Platform_GetTime();
	
	for (int32 i = 0; i < g->game->rotating_cube_count; ++i)
	{
		vec4 pos;
		glm_vec4_copy(g->game->render_rotating_cubes[i].transform[3], pos);
		//glm_vec4_copy((vec4) { [3] = 1.0f }, rotating_cubes[i].transform[3]);
		
		//glm_mat4_transpose(rotating_cubes[i].transform);
		//glm_mat4_mulv(rotating_cubes[i].transform, pos, pos);
		
		glm_mat4_identity(g->game->render_rotating_cubes[i].transform);
		glm_translate(g->game->render_rotating_cubes[i].transform, pos);
		glm_rotate(g->game->render_rotating_cubes[i].transform, t + (float32)i, (vec3) { 0.0f, 1.0f, 0.0f });
	}
	
	Render_Draw3DScene(&g->game->render_scene, &g->game->render_camera);
	
	//~ NOTE(ljre): 2D User Interface
	Render_Begin2D();
	
	//- NOTE(ljre): Draw Controller Index
	const vec4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	char buf[128];
	snprintf(buf, sizeof buf, "Controller Index: %i", g->game->controller_index);
	Render_DrawText(&g->game->font, Str(buf), (vec3) { 10.0f, 10.0f }, 32.0f, white, (vec3) { 0 });
	
	Render_DrawText(&g->game->font, Str("Press X, A, or B!"), (vec3) { 30.0f, 500.0f }, 24.0f, white, (vec3) { 0 });
	
	if (is_connected)
		DrawGamepadLayout(&gamepad, 0.0f, 0.0f, 300.0f, 300.0f);
	
	//~ NOTE(ljre): Finish Frame
	Engine_PlayAudios(g->game->playing_audios, &g->game->playing_audio_count, 0.075f);
	Engine_FinishFrame();
	
	Arena_Pop(g->temp_arena, memory_state);
}

struct Game_MenuButton
{
	vec3 position;
	vec3 size;
	String text;
}
typedef Game_MenuButton;

internal bool32
IsMouseOverButton(Input_Mouse* mouse, Game_MenuButton* button)
{
	return ((mouse->pos[0] >= button->position[0] && mouse->pos[0] <= button->position[0] + button->size[0]) &&
			(mouse->pos[1] >= button->position[1] && mouse->pos[1] <= button->position[1] + button->size[1]));
}

internal void
DrawMenuButton(Game_Data* game, Game_MenuButton* button, Input_Mouse* mouse)
{
	vec4 highlight_none = { 0.1f, 0.1f, 0.1f, 1.0f };
	vec4 highlight_over = { 0.2f, 0.2f, 0.2f, 1.0f };
	vec4 highlight_pressed = { 0.4f, 0.4f, 0.4f, 1.0f };
	
	float32* color = highlight_none;
	if (IsMouseOverButton(mouse, button))
	{
		color = highlight_over;
		
		if (Input_IsDown(*mouse, Input_MouseButton_Left))
		{
			color = highlight_pressed;
		}
	}
	
	Render_DrawRectangle(color, button->position, button->size, (vec3) { 0 });
	
	vec3 pos = {
		button->position[0] + button->size[0] / 2.0f,
		button->position[1] + button->size[1] / 2.0f,
	};
	
	vec3 alignment = { -0.5f, -0.5f, 0.0f };
	
	Render_DrawText(&game->font, button->text, pos, 24.0f, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, alignment);
}

API void
Game_Main(Engine_Data* g)
{
	Trace("Game_MainScene");
	
	if (!g->game)
	{
		g->game = Arena_Push(g->persistent_arena, sizeof(Game_Data));
		g->game->state = Game_State_Menu;
		
		bool32 font_loaded = Render_LoadFontFromFile(Str("./assets/FalstinRegular-XOr2.ttf"), &g->game->font);
		if (!font_loaded)
			Platform_ExitWithErrorMessage(Str("Could not load the default font :("));
	}
	
	switch (g->game->state)
	{
		case Game_State_Menu:
		{
			// NOTE(ljre): Menu Buttons
			Game_MenuButton button_3dscene = {
				.position = { 100.0f, 200.0f },
				.size = { 200.0f, 50.0f },
				.text = StrI("Run 3D Scene"),
			};
			
			Game_MenuButton button_quit = {
				.position = { 100.0f, 400.0f },
				.size = { 200.0f, 50.0f },
				.text = StrI("Quit"),
			};
			
			//~ NOTE(ljre): Update
			void* memory_state = Arena_End(g->temp_arena);
			
			Input_Mouse mouse;
			Input_GetMouse(&mouse);
			
			if (Platform_WindowShouldClose() || Input_KeyboardIsPressed(Input_KeyboardKey_Escape))
				g->running = false;
			
			if (Input_IsReleased(mouse, Input_MouseButton_Left))
			{
				if (IsMouseOverButton(&mouse, &button_3dscene))
				{
					g->game->state = Game_State_3DDemo;
					Game_3DDemoScene(g, true);
					break;
				}
				else if (IsMouseOverButton(&mouse, &button_quit))
					g->running = false;
			}
			
			//~ NOTE(ljre): Render Menu
			{
				Render_Begin2D();
				
				Render_ClearBackground(0.0f, 0.0f, 0.0f, 1.0f);
				Render_DrawText(&g->game->font, Str("Welcome!\nThose are buttons"), (vec3) { 10.0f, 10.0f }, 32.0f, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, (vec3) { 0 });
				
				DrawMenuButton(g->game, &button_3dscene, &mouse);
				DrawMenuButton(g->game, &button_quit, &mouse);
			}
			
			//~ NOTE(ljre): Finish Frame
			Arena_Pop(g->temp_arena, memory_state);
			Engine_FinishFrame();
		} break;
		
		case Game_State_3DDemo: Game_3DDemoScene(g, false); break;
	}
}
#endif
