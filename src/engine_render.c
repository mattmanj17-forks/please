#define GL (*global_graphics->opengl)
#define D3D (*global_graphics->d3d)

#ifdef DEBUG
#   define D3DCall(x) do{HRESULT r=(x);if(FAILED(r)){Platform_DebugMessageBox("D3DCall Failed\nFile: " __FILE__ "\nLine: %i\nError code: %lx",__LINE__,r);exit(1);}}while(0)
#else
#   define D3DCall(x) (x)
#endif

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
} typedef Vertex;

struct Render_3DModel
{
    uint32 vbo, vao, ebo;
    int32 index_count;
} typedef Render_3DModel;

//~ Globals
internal const GraphicsContext* global_graphics;
internal const Vertex global_quad_vertices[] = {
    // Positions         // Normals           // Texcoords
    { 0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, },
    { 1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, },
    { 1.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, },
    { 0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, },
};

internal const uint32 global_quad_indices[] = {
    0, 1, 2,
    2, 3, 0
};

internal float32 global_width;
internal float32 global_height;
internal mat4 global_view;
internal uint32 global_quad_vbo, global_quad_ebo, global_quad_vao;
internal uint32 global_white_texture;

internal uint32 global_default_shader;
internal uint32 global_default_3dshader;
internal uint32 global_current_shader;
internal int32 global_uniform_color;
internal int32 global_uniform_view;
internal int32 global_uniform_model;
internal int32 global_uniform_texture;

internal const char* const global_vertex_shader =
"#version 330 core\n"
"layout (location = 0) in vec3 aPosition;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoord;\n"

"out vec2 vTexCoord;"

"uniform mat4 uView;\n"
"uniform mat4 uModel;\n"

"void main() {"
"    gl_Position = uView * uModel * vec4(aPosition, 1.0);"
"    vTexCoord = aTexCoord;"
"}"
;

internal const char* const global_fragment_shader =
"#version 330 core\n"

"in vec2 vTexCoord;"

"out vec4 oFragColor;"

"uniform vec4 uColor;"
"uniform sampler2D uTexture;"

"void main() {"
"    oFragColor = uColor * texture(uTexture, vTexCoord);"
"}"
;

internal const char* const global_vertex_3dshader =
"#version 330 core\n"
"layout (location = 0) in vec3 aPosition;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoord;\n"

"out vec2 vTexCoord;"
"out vec3 vNormal;"

"uniform mat4 uView;\n"
"uniform mat4 uModel;\n"

"void main() {"
"    gl_Position = uView * uModel * vec4(aPosition, 1.0);"
"    vNormal = vec3(uModel * vec4(aNormal, 1.0));"
"    vTexCoord = aTexCoord;"
"}"
;

internal const char* const global_fragment_3dshader =
"#version 330 core\n"

"in vec2 vTexCoord;"
"in vec3 vNormal;"

"out vec4 oFragColor;"

"uniform vec4 uColor;"
"uniform sampler2D uTexture;"

"void main() {"
"    vec4 color = vec4(vec3(uColor) * max(0.0, dot(normalize(vNormal), vec3(0.0, 0.0, -1.0))), uColor.a);"
"    oFragColor = color * texture(uTexture, vTexCoord);"
"}"
;

//~ Functions
#ifdef DEBUG
internal void APIENTRY
OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                           GLsizei length, const GLchar* message, const void* userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        Platform_DebugMessageBox("OpenGL Error:\n\nType = 0x%x\nID = %u\nSeverity = 0x%x\nMessage= %s",
                                 type, id, severity, message);
    }
    else
    {
        Platform_DebugLog("OpenGL Debug Callback:\n\tType = 0x%x\n\tID = %u\n\tSeverity = 0x%x\n\tmessage = %s\n",
                          type, id, severity, message);
    }
}
#endif

internal uint32
CompileShader(const char* vertex_source, const char* fragment_source)
{
    char info[512];
    int32 success;
    
    // Compile Vertex Shader
    uint32 vertex_shader = GL.glCreateShader(GL_VERTEX_SHADER);
    GL.glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    GL.glCompileShader(vertex_shader);
    
    // Check for errors
    GL.glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GL.glGetShaderInfoLog(vertex_shader, sizeof info, NULL, info);
        Platform_DebugMessageBox("Vertex Shader Error:\n\n%s", info);
        GL.glDeleteShader(vertex_shader);
        return 0;
    }
    
    // Compile Fragment Shader
    uint32 fragment_shader = GL.glCreateShader(GL_FRAGMENT_SHADER);
    GL.glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    GL.glCompileShader(fragment_shader);
    
    // Check for errors
    GL.glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GL.glGetShaderInfoLog(fragment_shader, sizeof info, NULL, info);
        Platform_DebugMessageBox("Fragment Shader Error:\n\n%s", info);
        GL.glDeleteShader(vertex_shader);
        GL.glDeleteShader(fragment_shader);
        return 0;
    }
    
    // Link
    uint32 program = GL.glCreateProgram();
    GL.glAttachShader(program, vertex_shader);
    GL.glAttachShader(program, fragment_shader);
    GL.glLinkProgram(program);
    
    // Check for errors
    GL.glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GL.glGetProgramInfoLog(program, sizeof info, NULL, info);
        Platform_DebugMessageBox("Shader Linking Error:\n\n%s", info);
        GL.glDeleteProgram(program);
        program = 0;
    }
    
    // Clean-up
    GL.glDeleteShader(vertex_shader);
    GL.glDeleteShader(fragment_shader);
    
    return program;
}

internal void
ColorToVec4(ColorARGB color, vec4 out)
{
    out[0] = (float32)(color>>16 & 0xFF) / 255.0f;
    out[1] = (float32)(color>>8 & 0xFF) / 255.0f;
    out[2] = (float32)(color & 0xFF) / 255.0f;
    out[3] = (float32)(color>>24 & 0xFF) / 255.0f;
}

internal void
CalcBitmapSizeForText(const Render_Font* font, float32 scale, String text, int32* out_width, int32* out_height)
{
    int32 width = 0, line_width = 0, height = 0;
	int32 codepoint, it = 0;
	
	height += (int32)roundf((float32)(font->ascent - font->descent + font->line_gap) * scale);
	while (codepoint = String_Decode(text, &it), codepoint)
	{
		if (codepoint == '\n')
		{
			if (line_width > width)
				width = line_width;
			
			line_width = 0;
			height += (int32)roundf((float32)(font->ascent - font->descent + font->line_gap) * scale);
			continue;
		}
		
		int32 advance, bearing;
		stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance, &bearing);
		
		line_width += (int32)roundf((float32)advance * scale);
	}
    
    if (line_width > width)
		width = line_width;
	
    *out_width = width + 1;
    *out_height = height + 1;
}

//~ Temporary Internal API
internal void
Engine_InitRender(void)
{
    Trace("Render_Init");
    
    global_width = (float32)Platform_WindowWidth();
    global_height = (float32)Platform_WindowHeight();
    
#ifdef DEBUG
    GL.glEnable(GL_DEBUG_OUTPUT);
    GL.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    GL.glDebugMessageCallback(OpenGLDebugMessageCallback, NULL);
#endif
    
    GL.glEnable(GL_DEPTH_TEST);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL.glViewport(0, 0, (int32)global_width, (int32)global_height);
    
    uint32 vbo, vao, ebo;
    GL.glGenBuffers(1, &vbo);
    GL.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GL.glBufferData(GL_ARRAY_BUFFER, sizeof global_quad_vertices, global_quad_vertices, GL_STATIC_DRAW);
    
    GL.glGenVertexArrays(1, &vao);
    GL.glBindVertexArray(vao);
    
    GL.glEnableVertexAttribArray(0);
    GL.glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    GL.glEnableVertexAttribArray(1);
    GL.glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    GL.glEnableVertexAttribArray(2);
    GL.glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    
    GL.glGenBuffers(1, &ebo);
    GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof global_quad_indices, global_quad_indices, GL_STATIC_DRAW);
    
    global_quad_vbo = vbo;
    global_quad_vao = vao;
    global_quad_ebo = ebo;
    
    global_default_shader = CompileShader(global_vertex_shader, global_fragment_shader);
    global_default_3dshader = CompileShader(global_vertex_3dshader, global_fragment_3dshader);
    
    uint32 white_texture[] = {
        0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF,
    };
    
    GL.glGenTextures(1, &global_white_texture);
    GL.glBindTexture(GL_TEXTURE_2D, global_white_texture);
	GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_texture);
    GL.glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
Engine_DeinitRender(void)
{
    Trace("Render_Deinit");
}

//~ API
API void
Render_ClearBackground(ColorARGB color)
{
    vec4 color_vec;
    ColorToVec4(color, color_vec);
    
    GL.glClearColor(color_vec[0], color_vec[1], color_vec[2], color_vec[3]);
    GL.glClear(GL_COLOR_BUFFER_BIT);
}

API void
Render_Begin2D(const Render_Camera* camera)
{
    mat4 proj;
    glm_ortho(0.0f, global_width, global_height, 0.0f, -1.0f, 1.0f, proj);
    
    glm_mat4_identity(global_view);
    if (camera)
    {
        glm_translate(global_view, (vec3) { camera->pos[0], camera->pos[1], 0.0f });
        glm_rotate(global_view, camera->angle, (vec3) { 0.0f, 0.0f, 1.0f });
        glm_scale(global_view, (vec3) { camera->scale[0], camera->scale[1], 0.0f });
    }
    
    glm_mat4_mul(proj, global_view, global_view);
    
    GL.glDisable(GL_DEPTH_TEST);
    
    uint32 shader = global_default_shader;
    global_uniform_color = GL.glGetUniformLocation(shader, "uColor");
    global_uniform_view = GL.glGetUniformLocation(shader, "uView");
    global_uniform_model = GL.glGetUniformLocation(shader, "uModel");
    global_uniform_texture = GL.glGetUniformLocation(shader, "uTexture");
    
    global_current_shader = shader;
}

API void
Render_Begin3D(const Render_Camera* camera)
{
    mat4 proj;
    glm_perspective(90.0f, global_width / global_height, 0.01f, 100.0f, proj);
    
    glm_mat4_identity(global_view);
    if (camera)
    {
        glm_look((float32*)camera->pos, (float32*)camera->dir, (float32*)camera->up, global_view);
    }
    
    glm_mat4_mul(proj, global_view, global_view);
    
    GL.glEnable(GL_DEPTH_TEST);
    GL.glClear(GL_DEPTH_BUFFER_BIT);
    
    uint32 shader = global_default_3dshader;
    global_uniform_color = GL.glGetUniformLocation(shader, "uColor");
    global_uniform_view = GL.glGetUniformLocation(shader, "uView");
    global_uniform_model = GL.glGetUniformLocation(shader, "uModel");
    global_uniform_texture = GL.glGetUniformLocation(shader, "uTexture");
    
    global_current_shader = shader;
}

API void
Render_DrawRectangle(vec4 color, vec3 pos, vec3 size)
{
    mat4 matrix = GLM_MAT4_IDENTITY;
    glm_translate(matrix, pos);
    glm_scale(matrix, size);
    glm_translate(matrix, (vec3) { -0.5f, -0.5f }); // NOTE(ljre): Center
    
    GL.glUseProgram(global_current_shader);
    GL.glUniform4fv(global_uniform_color, 1, color);
    GL.glUniformMatrix4fv(global_uniform_view, 1, false, (float32*)global_view);
    GL.glUniformMatrix4fv(global_uniform_model, 1, false, (float32*)matrix);
    GL.glUniform1i(global_uniform_texture, 0);
    
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D, global_white_texture);
    
    GL.glBindVertexArray(global_quad_vao);
    GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, global_quad_ebo);
    GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    GL.glBindTexture(GL_TEXTURE_2D, 0);
}

API bool32
Render_LoadFontFromFile(String path, Render_Font* out_font)
{
    Render_Font result;
    result.data = Platform_ReadEntireFile(path, &result.data_size);
    
    if (result.data)
    {
        if (stbtt_InitFont(&result.info, result.data, 0))
        {
            stbtt_GetFontVMetrics(&result.info, &result.ascent, &result.descent, &result.line_gap);
            *out_font = result;
            return true;
        }
        else
        {
            Platform_FreeFileMemory(result.data, result.data_size);
            return false;
        }
    }
    else
    {
        return false;
    }
}

API void
Render_DrawText(const Render_Font* font, String text, vec3 pos, float32 char_height, ColorARGB color)
{
    float32 scale = stbtt_ScaleForPixelHeight(&font->info, char_height);
    vec4 color_vec;
    ColorToVec4(color, color_vec);
    
    int32 bitmap_width, bitmap_height;
    CalcBitmapSizeForText(font, scale, text, &bitmap_width, &bitmap_height);
    
    uint8* bitmap = Engine_PushMemory((uintsize)(bitmap_width * bitmap_height));
    
    float32 xx = 0, yy = 0;
    int32 codepoint, it = 0;
    while (codepoint = String_Decode(text, &it), codepoint)
    {
        if (codepoint == '\n')
		{
			xx = 0;
			yy += (float32)(font->ascent - font->descent + font->line_gap) * scale;
			continue;
		}
		
		int32 advance, bearing;
		stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance, &bearing);
		
		int32 char_x1, char_y1;
		int32 char_x2, char_y2;
		stbtt_GetCodepointBitmapBox(&font->info, codepoint, scale, scale,
									&char_x1, &char_y1, &char_x2, &char_y2);
		
		int32 char_width = char_x2 - char_x1;
		int32 char_height = char_y2 - char_y1;
		
		int32 end_x = (int32)roundf(xx + (float32)bearing * scale) + char_x1;
		int32 end_y = (int32)roundf(yy + (float32)font->ascent * scale) + char_y1;
		
		int32 offset = end_x + (end_y * bitmap_width);
		stbtt_MakeCodepointBitmap(&font->info, bitmap + offset, char_width, char_height, bitmap_width, scale, scale, codepoint);
		
		xx += (float32)advance * scale;
    }
    
    //Platform_DebugDumpBitmap("test.ppm", bitmap, bitmap_width, bitmap_height, 1);
    
    mat4 matrix = GLM_MAT4_IDENTITY;
    glm_translate(matrix, (vec3) { pos[0], pos[1], pos[2] });
    glm_scale(matrix, (vec3) { (float32)bitmap_width, (float32)bitmap_height });
    
    GL.glActiveTexture(GL_TEXTURE0);
    
    uint32 texture;
    GL.glGenTextures(1, &texture);
    GL.glBindTexture(GL_TEXTURE_2D, texture);
	GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL.glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, (int32[]) { GL_ONE, GL_ONE, GL_ONE, GL_RED });
    GL.glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap_width, bitmap_height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    
    GL.glUseProgram(global_current_shader);
    GL.glUniform4fv(global_uniform_color, 1, color_vec);
    GL.glUniformMatrix4fv(global_uniform_view, 1, false, (float32*)global_view);
    GL.glUniformMatrix4fv(global_uniform_model, 1, false, (float32*)matrix);
    GL.glUniform1i(global_uniform_texture, 0);
    
    GL.glBindVertexArray(global_quad_vao);
    GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, global_quad_ebo);
    GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    GL.glBindTexture(GL_TEXTURE_2D, 0);
    GL.glDeleteTextures(1, &texture);
    
    Engine_PopMemory(bitmap);
}

API void
Render_Draw3DModel(const Render_3DModel* model, const mat4 where, ColorARGB color)
{
    vec4 color_vec;
    ColorToVec4(color, color_vec);
    
    GL.glBindVertexArray(model->vao);
    GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    
    GL.glUseProgram(global_current_shader);
    
    GL.glUniform4fv(global_uniform_color, 1, color_vec);
    GL.glUniformMatrix4fv(global_uniform_view, 1, false, (const float32*)global_view);
    GL.glUniformMatrix4fv(global_uniform_model, 1, false, (const float32*)where);
    GL.glUniform1i(global_uniform_texture, 0);
    
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D, global_white_texture);
    
    GL.glDrawElements(GL_TRIANGLES, model->index_count, GL_UNSIGNED_INT, 0);
}

API bool32
Render_Load3DModelFromFile(String path, Render_3DModel* out)
{
    // TODO(ljre): Bad data error handling
    
    static const char header[] = "simple obj\n";
    bool32 result = true;
    
    uintsize size;
    char* data = Platform_ReadEntireFile(path, &size);
    if (!data)
        return false;
    
    char* head = data;
    
    if (strncmp(head, header, sizeof(header) - 1) == 0)
    {
        head += sizeof(header) - 1;
        
        uintsize vertex_count = strtoul(head, &head, 10);
        Vertex* vertices = Engine_PushMemory(sizeof(Vertex) * vertex_count);
        
        for (int32 i = 0; i < vertex_count; ++i)
        {
            Vertex* v = vertices + i;
            
            v->position[0] = strtof(head, &head);
            v->position[1] = strtof(head, &head);
            v->position[2] = strtof(head, &head);
            v->normal[0] = strtof(head, &head);
            v->normal[1] = strtof(head, &head);
            v->normal[2] = strtof(head, &head);
            v->texcoord[0] = strtof(head, &head);
            v->texcoord[1] = strtof(head, &head);
        }
        
        uintsize index_count = strtoul(head, &head, 10);
        uint32* indices = Engine_PushMemory(sizeof(uint32) * index_count);
        for (int32 i = 0; i < index_count; ++i)
        {
            indices[i] = (uint32)strtoul(head, &head, 10);
        }
        
        GL.glGenBuffers(1, &out->vbo);
        GL.glBindBuffer(GL_ARRAY_BUFFER, out->vbo);
        GL.glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(Vertex) * vertex_count), vertices, GL_STATIC_DRAW);
        
        GL.glGenBuffers(1, &out->ebo);
        GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ebo);
        GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(sizeof(uint32) * index_count), indices, GL_STATIC_DRAW);
        
        GL.glGenVertexArrays(1, &out->vao);
        GL.glBindVertexArray(out->vao);
        
        GL.glEnableVertexAttribArray(0);
        GL.glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        GL.glEnableVertexAttribArray(1);
        GL.glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        GL.glEnableVertexAttribArray(2);
        GL.glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
        
        out->index_count = (int32)index_count;
        
        Engine_PopMemory(indices);
        Engine_PopMemory(vertices);
    }
    else
    {
        result = false;
    }
    
    Platform_FreeFileMemory(data, size);
    return result;
}
