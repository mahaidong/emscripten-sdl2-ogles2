# Requires Emscripten to build: https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html
emcc -std=c++11 101.cpp -s USE_SDL=2 -s FULL_ES2=1 -s WASM=1 -o ../101.js
emcc -std=c++11 hello_triangle.cpp events.cpp camera.cpp -s USE_SDL=2 -s FULL_ES2=1 -s WASM=1 -o ../hello_triangle.js
emcc -std=c++11 hello_texture.cpp events.cpp camera.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS="[""png""]" -s FULL_ES2=1 -s WASM=1 --preload-file media/texmap.png -o ../hello_texture.js
emcc -std=c++11 hello_text_ttf.cpp events.cpp camera.cpp -s USE_SDL=2 -s USE_SDL_TTF=2 -s FULL_ES2=1 -s WASM=1 --preload-file media/LiberationSansBold.ttf -o ../hello_text_ttf.js
emcc -std=c++11 hello_text_txf.cpp events.cpp camera.cpp texfont.cpp -s USE_SDL=2 -s FULL_ES2=1 -s WASM=1 --preload-file media/rockfont.txf -o ../hello_text_txf.js
emcc -std=c++11 hello_image.cpp events.cpp camera.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s FULL_ES2=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -o ../hello_image.js
