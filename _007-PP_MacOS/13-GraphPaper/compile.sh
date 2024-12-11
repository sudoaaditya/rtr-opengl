mkdir -p Graph.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o Graph.app/Contents/MacOS/Graph Graph.mm -framework Cocoa -framework QuartzCore -framework OpenGL
