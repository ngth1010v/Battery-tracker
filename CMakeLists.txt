cmake_minimum_required(VERSION 3.10)

project(BatteryTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
set(APP_ICON_RESOURCE ${CMAKE_SOURCE_DIR}/resource/resource.rc)

file(GLOB SOURCES
    "src/*.cpp"
)

# =========================
# BUILD TARGET (PHẢI Ở TRƯỚC)
# =========================
add_executable(BatteryTracker 
    ${SOURCES}
    resource/resource.rc
)

target_include_directories(BatteryTracker PRIVATE 
    include
    resource
)

# =========================
# LINK LIBRARIES (SAU KHI TẠO TARGET)
# =========================
target_link_libraries(BatteryTracker PRIVATE 
    gdiplus
    shell32
    msimg32
)

# =========================
# COMPILE OPTIONS
# =========================
target_compile_options(BatteryTracker PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)
target_compile_definitions(BatteryTracker PRIVATE UNICODE _UNICODE)

# =========================
# STATIC RUNTIME (MSYS2 / MINGW)
# =========================
if (MINGW)
    target_link_options(BatteryTracker PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
        -Wl,--allow-multiple-definition
    )
endif()