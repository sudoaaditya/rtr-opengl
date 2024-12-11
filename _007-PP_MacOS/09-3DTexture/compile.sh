mkdir -p 3DTextures.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o 3DTextures.app/Contents/MacOS/3DTextures 3DTextures.mm -framework Cocoa -framework QuartzCore -framework OpenGL
