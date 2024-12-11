mkdir -p BWShapes.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o BWShapes.app/Contents/MacOS/BWShapes BWShapes.mm -framework Cocoa -framework QuartzCore -framework OpenGL
