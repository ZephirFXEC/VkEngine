import os
import pathlib
import subprocess

GCC_c = '"C:/Users/zphrfx/AppData/Local/Programs/CLion Nova/bin/mingw/bin/gcc.exe"'
GCC_cpp = '"C:/Users/zphrfx/AppData/Local/Programs/CLion Nova/bin/mingw/bin/g++.exe"'
MSVC = '"C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Tools/MSVC/14.39.33428/bin/Hostx64/x64/cl.exe"'
VCPKG = '"C:/vcpkg/scripts/buildsystems/vcpkg.cmake"'
CMAKE = '"C:/Users/zphrfx/AppData/Local/Programs/CLion Nova/bin/cmake/win/x64/bin/cmake.exe"'

# Define the configurations
configurations = [
    {
        "PROFILE_NAME": "Debug-MinGW",
        "GENERATION_DIR": "build/debug_gcc",
        "GENERATOR": "Ninja",
        "GENERATION_OPTIONS": f"-DCMAKE_TOOLCHAIN_FILE={VCPKG} -DCMAKE_CXX_FLAGS_DEBUG:STRING=\"-g -Wall -std=c++20\""
    },
    {
        "PROFILE_NAME": "RelWithDebInfo-MinGW",
        "GENERATION_DIR": "build/release_gcc",
        "GENERATOR": "Ninja",
        "GENERATION_OPTIONS": f"-DCMAKE_TOOLCHAIN_FILE={VCPKG} -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=\"-O3 -g -DNDEBUG -std=c++20\""
    },
    {
        "PROFILE_NAME": "Debug-Visual Studio",
        "GENERATION_DIR": "build/debug_msvc",
        "GENERATOR": '"Visual Studio 17 2022"',
        "GENERATION_OPTIONS": f"-DCMAKE_TOOLCHAIN_FILE={VCPKG} -DCMAKE_CXX_FLAGS_DEBUG:STRING=\"/MDd /Zi /Ob0 /Od /RTC1 /std:c++20 /W4\""
    },
    {
        "PROFILE_NAME": "RelWithDebInfo-Visual Studio",
        "GENERATION_DIR": "build/release_msvc",
        "GENERATOR": '"Visual Studio 17 2022"',
        "GENERATION_OPTIONS": f"-DCMAKE_TOOLCHAIN_FILE={VCPKG} -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=\"/MD /Zi /O2 /Ob1 /DNDEBUG /std:c++20\""
    }
]


# Function to run the command
def run_command(cmd):
    try:
        subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while executing: {cmd}")
        print(f"Error: {e}")


# Function to compile the project
def compile_project(build_dir, generator):
    compile_command = ""

    if "Visual Studio" in generator:
        mode = config["PROFILE_NAME"].split("-")[0]
        compile_command = f"{CMAKE} --build {build_dir} --target ALL_BUILD --config {mode}"

    if "Ninja" in generator:
        compile_command = f"{CMAKE} --build {build_dir} --target all -j 10"

    print(f"Running compile command: {compile_command}")
    run_command(compile_command)


# Iterate over configurations and execute commands
for config in configurations:
    current_path = pathlib.Path(__file__).parent.resolve()
    generation_dir = current_path / config["GENERATION_DIR"]
    toolchain = config["GENERATOR"]
    options = config["GENERATION_OPTIONS"]

    # Create the generation directory if it doesn't exist
    os.makedirs(generation_dir, exist_ok=True)

    # Compile the project
    compile_project(generation_dir, toolchain)
