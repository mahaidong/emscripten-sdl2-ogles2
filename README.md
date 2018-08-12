# emscripten-sdl2-ogles2
*C++/SDL2/OpenGLES2 to Javascript/WebGL using Emscripten*

This project is a collection of C++/SDL2/OpenGL samples that run in the browser via Emscripten.  These samples can serve as building blocks to aid in porting C++ graphics apps to the browser.

The long-term goal with this work is to preserve old graphics demos, particularly those developed in the 1990s by Silicon Graphics.  This work is inspired in part by the [preservation of classic arcade games in the browser](https://archive.org/details/internetarcade), which also utilizes Emscripten.


## Samples

### [Run Hello Triangle](https://erik-larsen.github.io/emscripten-sdl2-ogles2/hello_triangle.html)

![Hello Triangle](media/hello_triangle.png)

Demonstrates a colorful triangle using shaders, with support for mouse and touch input:
 * Pan using left mouse or finger drag.
 * Zoom using mouse wheel or pinch gesture.

### [Run Hello Texture](https://erik-larsen.github.io/emscripten-sdl2-ogles2/hello_texture.html)

![Hello Texture](media/hello_texture.png)

Demonstrates a textured triangle, using SDL to load an image from a file.

### [Run Hello Text](https://erik-larsen.github.io/emscripten-sdl2-ogles2/hello_text_ttf.html)

![Hello Text](media/hello_text_ttf.png)

Demonstrates TrueType text, using SDL to render a string into a texture and apply it to a quad.

### [Run Hello Texture Atlas](https://erik-larsen.github.io/emscripten-sdl2-ogles2/hello_text_txf.html)

![Hello Texture Atlas](media/hello_text_txf.png)

Demonstrates Texfont text, loading a font texture atlas from a .txf file and applying it to a quad. 

## Motivation

### Why Emscripten?  

Running an app in the browser is the ultimate convenience for the user.  No manual download/install is necessary and the app can run equally well on desktop, tablet, and phone.  Better to have Emscripten do the work to produce optimal Javascript/WASM, than doing the boring and error-prone work of hand porting C++ code.

### Why SDL2? 

These demos require OS-dependent stuff (keyboard, mouse, touch, text, audio, networking, etc.). SDL provides a cross-platform library to access this, and Emscripten supports SDL.

### Why OpenGLES2?  

These demos require GPU accelerated graphics which is WebGL in the browser.  And Emscripten generates WebGL by transpiling OpenGLES 2 code.
