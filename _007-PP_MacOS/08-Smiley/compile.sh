mkdir -p StaticSmiley.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o StaticSmiley.app/Contents/MacOS/StaticSmiley StaticSmiley.mm -framework Cocoa -framework QuartzCore -framework OpenGL
