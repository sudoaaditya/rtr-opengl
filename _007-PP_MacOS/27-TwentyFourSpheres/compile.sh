mkdir -p 24Spheres.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o 24Spheres.app/Contents/MacOS/24Spheres 24Spheres.mm -framework Cocoa -framework QuartzCore -framework OpenGL
