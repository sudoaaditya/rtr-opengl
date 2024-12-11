mkdir -p HwlloWorld.app/Contents/MacOS

clang -o HelloWorld.app/Contents/MacOS HelloWorld.m -framework Cocoa
