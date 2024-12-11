mkdir -p DiffuseLightsOnCube.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o DiffuseLightsOnCube.app/Contents/MacOS/DiffuseLightsOnCube DiffuseLightsOnCube.mm -framework Cocoa -framework QuartzCore -framework OpenGL
