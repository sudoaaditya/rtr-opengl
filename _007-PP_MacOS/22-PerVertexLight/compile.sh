mkdir -p PerVertexLight.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o PerVertexLight.app/Contents/MacOS/PerVertexLight PerVertexLight.mm -framework Cocoa -framework QuartzCore -framework OpenGL
