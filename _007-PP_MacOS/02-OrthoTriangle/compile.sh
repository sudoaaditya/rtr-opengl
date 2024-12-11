mkdir -p OrthoTriangle.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o OrthoTriangle.app/Contents/MacOS/OrthoTriangle OrthoTriangle.mm -framework Cocoa -framework QuartzCore -framework OpenGL
