mkdir -p PerFragmentLight.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o PerFragmentLight.app/Contents/MacOS/PerFragmentLight PerFragmentLight.mm -framework Cocoa -framework QuartzCore -framework OpenGL
