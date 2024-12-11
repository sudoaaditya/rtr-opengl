mkdir -p DiffuseLightOnSphere.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o DiffuseLightOnSphere.app/Contents/MacOS/DiffuseLightOnSphere DiffuseLightOnSphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
