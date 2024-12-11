mkdir -p 2DAnimation.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o 2DAnimation.app/Contents/MacOS/2DAnimation 2DAnimation.mm -framework Cocoa -framework QuartzCore -framework OpenGL
