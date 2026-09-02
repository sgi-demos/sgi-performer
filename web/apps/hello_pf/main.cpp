/*
 * hello_pf - milestone 0 bring-up app.
 *
 * Proves the platform layer the whole port sits on: an SDL2 window, an
 * ES2-subset GL context, a #version-100-compatible shader pipeline, VBO
 * drawing, and the Emscripten main-loop shape.  No Performer API yet.
 *
 * Context strategy (matches the port plan):
 *   1. ask SDL for a real OpenGL ES 2.0 context (Linux/Windows drivers,
 *      ANGLE, Emscripten/WebGL1),
 *   2. fall back to desktop GL 2.1 (macOS) and stay inside the ES2 subset.
 *
 * Usage: hello_pf [--frames N]   (exit after N frames; for smoke tests)
 */
#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLES2_MIN_IMPLEMENTATION
#include "gles2_min.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static const char* kVertexBody =
    "attribute vec2 a_pos;\n"
    "attribute vec3 a_color;\n"
    "uniform float u_angle;\n"
    "varying vec3 v_color;\n"
    "void main() {\n"
    "    float c = cos(u_angle), s = sin(u_angle);\n"
    "    gl_Position = vec4(c*a_pos.x - s*a_pos.y, s*a_pos.x + c*a_pos.y, 0.0, 1.0);\n"
    "    v_color = a_color;\n"
    "}\n";

static const char* kFragmentBody =
    "varying vec3 v_color;\n"
    "void main() { gl_FragColor = vec4(v_color, 1.0); }\n";

static GLuint compileShader(GLenum type, int isES, const char* body)
{
    /* ES contexts want #version 100 + default precision; desktop GL 2.1
     * compiles the same bodies as GLSL 1.20 with mediump defined away. */
    const char* prelude = isES
        ? "#version 100\nprecision mediump float;\n"
        : "#version 120\n#define mediump\n#define highp\n#define lowp\n";
    const char* sources[2] = { prelude, body };

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 2, sources, NULL);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof log, NULL, log);
        fprintf(stderr, "shader compile failed: %s\n", log);
        exit(1);
    }
    return shader;
}

struct AppState {
    SDL_Window* window;
    GLint uAngle;
    int frame;
    int maxFrames;   /* <= 0 means run until quit */
    int running;
};

static void frame(void* arg)
{
    AppState* st = (AppState*)arg;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT ||
            (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE))
            st->running = 0;
    }

    glClearColor(0.15f, 0.17f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1f(st->uAngle, (float)st->frame * 0.02f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    SDL_GL_SwapWindow(st->window);

    st->frame++;
    if (st->maxFrames > 0 && st->frame >= st->maxFrames)
        st->running = 0;
}

int main(int argc, char** argv)
{
    int maxFrames = 0;
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(argv[i], "--frames") == 0)
            maxFrames = atoi(argv[i + 1]);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* Preferred: a genuine ES 2.0 context. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    int isES = 1;
    SDL_Window* window = SDL_CreateWindow("hello_pf",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext ctx = window ? SDL_GL_CreateContext(window) : NULL;

    if (!ctx) {
        /* Fallback: desktop GL 2.1 (macOS), same ES2-subset code. */
        if (window) SDL_DestroyWindow(window);
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        isES = 0;
        window = SDL_CreateWindow("hello_pf",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        ctx = window ? SDL_GL_CreateContext(window) : NULL;
    }
    if (!ctx) {
        fprintf(stderr, "no usable GL context: %s\n", SDL_GetError());
        return 1;
    }
    if (!gles2_min_load()) {
        fprintf(stderr, "failed to resolve GL entry points\n");
        return 1;
    }

    printf("context : %s\n", isES ? "OpenGL ES 2.0" : "desktop GL 2.1 (ES2 subset)");
    printf("vendor  : %s\n", (const char*)glGetString(GL_VENDOR));
    printf("renderer: %s\n", (const char*)glGetString(GL_RENDERER));
    printf("version : %s\n", (const char*)glGetString(GL_VERSION));

    GLuint vs = compileShader(GL_VERTEX_SHADER, isES, kVertexBody);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, isES, kFragmentBody);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint linked = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof log, NULL, log);
        fprintf(stderr, "program link failed: %s\n", log);
        return 1;
    }
    glUseProgram(prog);

    /* interleaved x, y, r, g, b */
    static const GLfloat verts[] = {
         0.0f,  0.6f,  1.0f, 0.2f, 0.2f,
        -0.6f, -0.5f,  0.2f, 1.0f, 0.2f,
         0.6f, -0.5f,  0.2f, 0.4f, 1.0f,
    };
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

    GLint aPos = glGetAttribLocation(prog, "a_pos");
    GLint aColor = glGetAttribLocation(prog, "a_color");
    glEnableVertexAttribArray((GLuint)aPos);
    glVertexAttribPointer((GLuint)aPos, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray((GLuint)aColor);
    glVertexAttribPointer((GLuint)aColor, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(GLfloat), (const void*)(2 * sizeof(GLfloat)));

    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    AppState st = { window, glGetUniformLocation(prog, "u_angle"), 0, maxFrames, 1 };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(frame, &st, 0, 1);
#else
    SDL_GL_SetSwapInterval(1);
    while (st.running)
        frame(&st);
#endif

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("rendered %d frames, clean exit\n", st.frame);
    return 0;
}
