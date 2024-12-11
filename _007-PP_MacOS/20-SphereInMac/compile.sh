mkdir -p Sphere.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o Sphere.app/Contents/MacOS/Sphere Sphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
