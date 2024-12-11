mkdir -p StaticIndia.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o StaticIndia.app/Contents/MacOS/StaticIndia StaticIndia.mm -framework Cocoa -framework QuartzCore -framework OpenGL
