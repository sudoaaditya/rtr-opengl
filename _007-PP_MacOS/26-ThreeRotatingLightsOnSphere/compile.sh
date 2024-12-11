mkdir -p ThreeLights.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o ThreeLights.app/Contents/MacOS/ThreeLights 3Lights.mm -framework Cocoa -framework QuartzCore -framework OpenGL
