mkdir -p PerspectiveTriangle.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o PerspectiveTriangle.app/Contents/MacOS/PerspectiveTriangle PerspectiveTriangle.mm -framework Cocoa -framework QuartzCore -framework OpenGL
