mkdir -p ToggledLight.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o ToggledLight.app/Contents/MacOS/ToggledLight ToggledLight.mm -framework Cocoa -framework QuartzCore -framework OpenGL
