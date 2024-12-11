mkdir -p Geometry.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o Geometry.app/Contents/MacOS/Geometry Geometry.mm -framework Cocoa -framework QuartzCore -framework OpenGL
