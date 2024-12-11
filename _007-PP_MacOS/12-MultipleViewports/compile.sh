mkdir -p MultipleViewports.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o MultipleViewports.app/Contents/MacOS/MultipleViewports MultipleViewports.mm -framework Cocoa -framework QuartzCore -framework OpenGL
