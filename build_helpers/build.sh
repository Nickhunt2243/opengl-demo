# Step 1: Configure the project
cmake -S . -B build

# Step 2: Build the project
cmake --build build --target OpenGLDemo --config Debug

# Move to Debug directory and run program
cd Debug
OpenGLDemo.exe
