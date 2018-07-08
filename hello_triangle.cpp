//
// Minimal C++/SDL2/OpenGLES2 sample that Emscripten transpiles into Javascript/WebGL.
//
// Setup:
//     Install emscripten: http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html
//
// Build:
//     emcc hello_triangle.cpp -s USE_SDL=2 -s FULL_ES2=1 -s WASM=0 -o hello_triangle.js
//
// Run:
//     emrun hello_triangle.html
//     emrun hello_triangle_debug.html
//
// Result:
//     A colorful triangle.  Left mouse pans, mouse wheel zooms in/out.  Window is resizable.
//
#include <exception>
#include <algorithm>

#define GL_GLEXT_PROTOTYPES 1

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL.h>
#include <SDL_opengles2.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#endif

// Window
SDL_Window* window = nullptr;
Uint32 windowID = 0;
int windowWidth = 640, windowHeight = 480;

// Mouse input
const float MOUSE_WHEEL_ZOOM_DELTA = 0.05f;
bool mouseButtonDown = false;
int mouseButtonDownX = 0, mouseButtonDownY = 0;
int mousePositionX = 0, mousePositionY = 0;

// Finger input
bool fingerDown = false;
float fingerDownX = 0.0f, fingerDownY = 0.0f;
long long fingerDownId = 0;

// Pinch input
const float PINCH_ZOOM_THRESHOLD = 0.001f;
const float PINCH_SCALE = 8.0f;
bool pinch = false;

// Shader vars
const GLfloat ZOOM_MIN = 0.1f, ZOOM_MAX = 10.0f;
GLint shaderPan, shaderZoom, shaderAspect;
GLfloat pan[2] = {0.0f, 0.0f}, zoom = 1.0f, aspect = 1.0f;
GLfloat basePan[2] = {0.0f, 0.0f};

// Vertex shader
const GLchar* vertexSource =
    "uniform vec2 pan;                             \n"
    "uniform float zoom;                           \n"
    "uniform float aspect;                         \n"
    "attribute vec4 position;                      \n"
    "varying vec3 color;                           \n"
    "void main()                                   \n"
    "{                                             \n"
    "    gl_Position = vec4(position.xyz, 1.0);    \n"
    "    gl_Position.xy += pan;                    \n"
    "    gl_Position.xy *= zoom;                   \n"
    "    gl_Position.y *= aspect;                  \n"
    "    color = gl_Position.xyz + vec3(0.5);      \n"
    "}                                             \n";

// Fragment/pixel shader
const GLchar* fragmentSource =
    "precision mediump float;                     \n"
    "varying vec3 color;                          \n"
    "void main()                                  \n"
    "{                                            \n"
    "    gl_FragColor = vec4 ( color, 1.0 );      \n"
    "}                                            \n";

float clamp (float val, float lo, float hi)
{
    return std::max(lo, std::min(val, hi));
}

void updateShader()
{
    glUniform2fv(shaderPan, 1, pan);
    glUniform1f(shaderZoom, zoom); 
    glUniform1f(shaderAspect, aspect);
}

GLuint initShader()
{
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

    // Get shader variables and initalize them
	shaderPan = glGetUniformLocation(shaderProgram, "pan");
	shaderZoom = glGetUniformLocation(shaderProgram, "zoom");    
	shaderAspect = glGetUniformLocation(shaderProgram, "aspect");
    updateShader();

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
        0.5f, -0.5f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the shader vertex data (positions only, 3 floats)
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

// Convert from normalized window coords (x,y) in ([0.0, 1.0], [1.0, 0.0]) to device coords ([-1.0, 1.0], [-1.0,1.0])
void normWindowToDeviceCoords (float normWinX, float normWinY, float& deviceX, float& deviceY)
{
    deviceX = (normWinX - 0.5f) * 2.0f;
    deviceY = (1.0f - normWinY - 0.5f) * 2.0f;
}

// Convert from window coords (x,y) in ([0, windowWidth], [windowHeight, 0]) to device coords ([-1.0, 1.0], [-1.0,1.0])
void windowToDeviceCoords (int winX, int winY, float& deviceX, float& deviceY)
{
    normWindowToDeviceCoords(winX / (float)windowWidth,  winY / (float)windowHeight, deviceX, deviceY);
}

// Convert from device coords ([-1.0, 1.0], [-1.0,1.0]) to world coords ([-inf, inf], [-inf, inf])
void deviceToWorldCoords (float deviceX, float deviceY, float& worldX, float& worldY)
{
    worldX = deviceX / zoom - pan[0];
    worldY = deviceY / aspect / zoom - pan[1];
}

// Convert from window coords (x,y) in ([0, windowWidth], [windowHeight, 0]) to world coords ([-inf, inf], [-inf, inf])
void windowToWorldCoords(int winX, int winY, float& worldX, float& worldY)
{
    float deviceX, deviceY;
    windowToDeviceCoords(winX, winY, deviceX, deviceY);   
    deviceToWorldCoords(deviceX, deviceY, worldX, worldY);
}

// Convert from normalized window coords (x,y) in in ([0.0, 1.0], [1.0, 0.0]) to world coords ([-inf, inf], [-inf, inf])
void normWindowToWorldCoords(float normWinX, float normWinY, float& worldX, float& worldY)
{
    float deviceX, deviceY;
    normWindowToDeviceCoords(normWinX, normWinY, deviceX, deviceY);
    deviceToWorldCoords(deviceX, deviceY, worldX, worldY);
}

void windowResizeEvent(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    // Update viewport and aspect ratio
    glViewport(0, 0, windowWidth, windowHeight);
    aspect = windowWidth / (float)windowHeight; 
    updateShader();
}

void zoomEventMouse(bool mouseWheelDown, int x, int y)
{                
    float preZoomWorldX, preZoomWorldY;
    windowToWorldCoords(mousePositionX, mousePositionY, preZoomWorldX, preZoomWorldY);

    // Zoom by scaling up/down in 0.05 increments 
    float zoomDelta = mouseWheelDown ? -MOUSE_WHEEL_ZOOM_DELTA : MOUSE_WHEEL_ZOOM_DELTA;
    zoom += zoomDelta;

    // Limit zooming to finite range
    zoom = clamp(zoom, ZOOM_MIN, ZOOM_MAX);

    float postZoomWorldX, postZoomWorldY;
    windowToWorldCoords(mousePositionX, mousePositionY, postZoomWorldX, postZoomWorldY);

    // Zoom to point: Keep the world coords under mouse position the same before and after the zoom
    float deltaWorldX = postZoomWorldX - preZoomWorldX, deltaWorldY = postZoomWorldY - preZoomWorldY;
    pan[0] += deltaWorldX;
    pan[1] += deltaWorldY;

    updateShader();
}

void zoomEventPinch (float pinchDist, float pinchX, float pinchY)
{
    float preZoomWorldX, preZoomWorldY;
    normWindowToWorldCoords(pinchX, pinchY, preZoomWorldX, preZoomWorldY);

    // Zoom in/out by positive/negative pinch distance
    float zoomDelta = pinchDist * PINCH_SCALE;
    zoom += zoomDelta;

    // Limit zooming to finite range
    zoom = clamp(zoom, ZOOM_MIN, ZOOM_MAX);

    float postZoomWorldX, postZoomWorldY;
    normWindowToWorldCoords(pinchX, pinchY, postZoomWorldX, postZoomWorldY);

    // Zoom to point: Keep the world coords under pinch position the same before and after the zoom
    float deltaWorldX = postZoomWorldX - preZoomWorldX, deltaWorldY = postZoomWorldY - preZoomWorldY;
    pan[0] += deltaWorldX;
    pan[1] += deltaWorldY;

    updateShader();
}

void panEventMouse(int x, int y)
{ 
     int deltaX = windowWidth / 2 + (x - mouseButtonDownX),
         deltaY = windowHeight / 2 + (y - mouseButtonDownY);

    float deviceX, deviceY;
    windowToDeviceCoords(deltaX,  deltaY, deviceX, deviceY);

    pan[0] = basePan[0] + deviceX / zoom;
    pan[1] = basePan[1] + deviceY / zoom / aspect;
    
    updateShader();
}

void panEventFinger(float x, float y)
{ 
    float deltaX = 0.5f + (x - fingerDownX),
          deltaY = 0.5f + (y - fingerDownY);

    float deviceX, deviceY;
    normWindowToDeviceCoords(deltaX,  deltaY, deviceX, deviceY);

    pan[0] = basePan[0] + deviceX / zoom;
    pan[1] = basePan[1] + deviceY / zoom / aspect;

    updateShader();
}

void handleEvents()
{
    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                std::terminate();
                break;

            case SDL_WINDOWEVENT:
            {
                if (event.window.windowID == windowID
                    && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int width = event.window.data1, height = event.window.data2;
                    windowResizeEvent(width, height);
                }
                break;
            }

            case SDL_MOUSEWHEEL: 
            {
                SDL_MouseWheelEvent *m = (SDL_MouseWheelEvent*)&event;
                bool mouseWheelDown = (m->y < 0);
                zoomEventMouse(mouseWheelDown, mousePositionX, mousePositionY);
                break;
            }
            
            case SDL_MOUSEMOTION: 
            {
                SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
                mousePositionX = m->x;
                mousePositionY = m->y;
                if (mouseButtonDown && !fingerDown && !pinch)
                    panEventMouse(mousePositionX, mousePositionY);
                break;
            }

            case SDL_MOUSEBUTTONDOWN: 
            {
                SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
                if (m->button == SDL_BUTTON_LEFT && !fingerDown && !pinch)
                {
                    mouseButtonDown = true;
                    mouseButtonDownX = m->x;
                    mouseButtonDownY = m->y;
                    basePan[0] = pan[0]; 
                    basePan[1] = pan[1];
                }
                break;
            }

            case SDL_MOUSEBUTTONUP: 
            {
                SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
                if (m->button == SDL_BUTTON_LEFT)
                    mouseButtonDown = false;
                break;
            }

            case SDL_FINGERMOTION:
                if (fingerDown)
                {
                    SDL_TouchFingerEvent *m = (SDL_TouchFingerEvent*)&event;

                    // Finger down and finger moving must match
                    if (m->fingerId == fingerDownId)
                        panEventFinger(m->x, m->y);
                }
                break;

            case SDL_FINGERDOWN:
                if (!pinch)
                {
                    // Finger already down means multiple fingers, which is handled by multigesture event
                    if (fingerDown)
                        fingerDown = false;
                    else
                    {
                        SDL_TouchFingerEvent *m = (SDL_TouchFingerEvent*)&event;

                        fingerDown = true;
                        fingerDownX = m->x;
                        fingerDownY = m->y;
                        fingerDownId = m->fingerId;
                        basePan[0] = pan[0]; 
                        basePan[1] = pan[1];
                    }
                }
                break;

            case SDL_MULTIGESTURE:
            {
                SDL_MultiGestureEvent *m = (SDL_MultiGestureEvent*)&event;
                if (m->numFingers == 2 && fabs(m->dDist) >= PINCH_ZOOM_THRESHOLD)
                {
                    pinch = true;
                    fingerDown = false;
                    mouseButtonDown = false;
                    zoomEventPinch(m->dDist, m->x, m->y);
                }
                break;
            }

            case SDL_FINGERUP:
                fingerDown = false;
                pinch = false;
                break;
        }

        // Debugging
        printf ("event=%d mousePos=%d,%d mouseButtonDown=%d fingerDown=%d pinch=%d aspect=%f window=%dx%d\n", 
                 event.type, mousePositionX, mousePositionY, mouseButtonDown, fingerDown, pinch, aspect, windowWidth, windowHeight);      
        printf ("    zoom=%f pan=%f,%f\n", zoom, pan[0], pan[1]);
    }
}

void redraw()
{
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the vertex buffer
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap front/back framebuffers
    SDL_GL_SwapWindow(window);
}

void mainLoop() 
{    
    handleEvents();
    redraw();
}

int main(int argc, char** argv)
{
    // Create SDL window
    window = SDL_CreateWindow("hello_triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE| SDL_WINDOW_SHOWN);
    windowID = SDL_GetWindowID(window);

    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(window);

    // Set clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Initialize shader
    GLuint shaderProgram = initShader();

    // Initialize geometry
    initGeometry(shaderProgram);

    // Start the main loop
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, true);
#else
    while(true) 
        mainLoop();
#endif

    return 0;
}