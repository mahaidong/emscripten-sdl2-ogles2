#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL.h>
#include <SDL_opengles2.h>

// Window
SDL_Window *win;

GLuint initShader()
{

    const GLchar *vertexSource =
        "attribute vec3 position;                      \n"
        "void main()                                   \n"
        "{                                             \n"
        "    gl_Position = vec4(position.xyz, 1.0);    \n"
        "}                                             \n";

    // Fragment/pixel shader
    const GLchar *fragmentSource =
        "void main()                                  \n"
        "{                                            \n"
        "    gl_FragColor = vec4 ( 1.0,1.0,1.0, 1.0 );\n"
        "}                                            \n";

    // Create and compile vertex shader

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link vertex and fragment shader into shader program and use it
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    return shaderProgram;
}

void initGeometry(GLuint shaderProgram)
{
    // Create vertex buffer object and copy vertex data into it
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLfloat vertices[] =
        {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the shader vertex data (positions only, 3 floats)
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void redraw(SDL_Event &eventHandler)
{
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the vertex buffer
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(win);
}

void mainLoop(void *mainLoopArg)
{
    SDL_Event &eventHandler = *((SDL_Event *)mainLoopArg);
    SDL_PollEvent(&eventHandler);
    redraw(eventHandler);
}

int main(int argc, char **argv)
{

    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow("101",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         640, 480,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    SDL_GL_CreateContext(win);

    // Initialize shader and geometry
    GLuint shaderProgram = initShader();
    initGeometry(shaderProgram);

    // Start the main loop
    SDL_Event eventHandler;
    void *mainLoopArg = &eventHandler;
#ifdef __EMSCRIPTEN__
    int fps = 0; // Use browser's requestAnimationFrame
    emscripten_set_main_loop_arg(mainLoop, mainLoopArg, fps, true);
#else
    while (true)
        mainLoop(mainLoopArg);
#endif
    return 0;
}