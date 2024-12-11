mkdir -p DynamicIndia.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o DynamicIndia.app/Contents/MacOS/DynamicIndia DynamicIndia.mm -framework Cocoa -framework QuartzCore -framework OpenGL
