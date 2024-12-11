mkdir -p DeathlyHallows.app/Contents/MacOS

clang++ -Wno-deprecated-declarations -o DeathlyHallows.app/Contents/MacOS/DeathlyHallows DeathlyHallows.mm -framework Cocoa -framework QuartzCore -framework OpenGL
