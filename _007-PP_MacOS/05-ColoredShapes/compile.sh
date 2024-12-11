mkdir -p ColoredShapes.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o ColoredShapes.app/Contents/MacOS/ColoredShapes ColoredShapes.mm -framework Cocoa -framework QuartzCore -framework OpenGL
