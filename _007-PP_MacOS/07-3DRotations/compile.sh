mkdir -p 3DAnimation.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o 3DAnimation.app/Contents/MacOS/3DAnimation 3DAnimation.mm -framework Cocoa -framework QuartzCore -framework OpenGL
