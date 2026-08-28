cmake_minimum_required(VERSION 3.24)

project(scrcpy VERSION 4.1 LANGUAGES C RC)

if(NOT WIN32)
    message(FATAL_ERROR "This CMake build is only intended for Windows.")
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "scrcpy uses GNU C statement expressions. Open a Visual Studio Developer Command Prompt, then configure with: "
        "cmake -S . -B build-msvc -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release")
endif()

set(SCRCPY_FFMPEG_ROOT "F:/NRC/navigate/Inject/video/ffmpeg" CACHE PATH
    "FFmpeg development package root")
set(SCRCPY_SDL3_ROOT "D:/software/SDL3-3.4.14" CACHE PATH
    "SDL3 Visual C++ development package root")
set(SCRCPY_RUNTIME_DIR "F:/NRC/navigate/x64/Debug/androidData/scrcpy" CACHE PATH
    "Directory containing FFmpeg runtime DLLs and scrcpy-server")

foreach(required_path
        "${SCRCPY_FFMPEG_ROOT}/include/libavcodec/avcodec.h"
        "${SCRCPY_FFMPEG_ROOT}/lib/avcodec.lib"
        "${SCRCPY_SDL3_ROOT}/include/SDL3/SDL.h"
        "${SCRCPY_SDL3_ROOT}/lib/x64/SDL3.lib")
    if(NOT EXISTS "${required_path}")
        message(FATAL_ERROR "Required dependency not found: ${required_path}")
    endif()
endforeach()

set(SCRCPY_SOURCES
    app/src/main.c
    app/src/adb/adb.c
    app/src/adb/adb_device.c
    app/src/adb/adb_parser.c
    app/src/adb/adb_tunnel.c
    app/src/audio_player.c
    app/src/audio_regulator.c
    app/src/cli.c
    app/src/clock.c
    app/src/compat.c
    app/src/control_msg.c
    app/src/controller.c
    app/src/decoder.c
    app/src/demuxer.c
    app/src/device_msg.c
    app/src/disconnect.c
    app/src/events.c
    app/src/icon.c
    app/src/file_pusher.c
    app/src/fps_counter.c
    app/src/frame_buffer.c
    app/src/input_manager.c
    app/src/keyboard_sdk.c
    app/src/mouse_capture.c
    app/src/mouse_sdk.c
    app/src/opengl.c
    app/src/options.c
    app/src/packet_merger.c
    app/src/receiver.c
    app/src/recorder.c
    app/src/scrcpy.c
    app/src/screen.c
    app/src/sdl_hints.c
    app/src/server.c
    app/src/texture.c
    app/src/version.c
    app/src/video_regulator.c
    app/src/hid/hid_gamepad.c
    app/src/hid/hid_keyboard.c
    app/src/hid/hid_mouse.c
    app/src/trait/frame_source.c
    app/src/trait/packet_source.c
    app/src/uhid/gamepad_uhid.c
    app/src/uhid/keyboard_uhid.c
    app/src/uhid/mouse_uhid.c
    app/src/uhid/uhid_output.c
    app/src/util/acksync.c
    app/src/util/audiobuf.c
    app/src/util/average.c
    app/src/util/command.c
    app/src/util/env.c
    app/src/util/file.c
    app/src/util/getopt.c
    app/src/util/intmap.c
    app/src/util/intr.c
    app/src/util/log.c
    app/src/util/memory.c
    app/src/util/net.c
    app/src/util/net_intr.c
    app/src/util/process.c
    app/src/util/process_intr.c
    app/src/util/rand.c
    app/src/util/sdl.c
    app/src/util/str.c
    app/src/util/strbuf.c
    app/src/util/term.c
    app/src/util/thread.c
    app/src/util/tick.c
    app/src/util/timeout.c
    app/src/sys/win/file.c
    app/src/sys/win/process.c
    app/scrcpy-windows.rc
)

set(CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${CONFIG_DIR}")
file(WRITE "${CONFIG_DIR}/config.h" [=[
#ifndef SCRCPY_CONFIG_H
#define SCRCPY_CONFIG_H

#define SCRCPY_VERSION "4.1"
#define PREFIX ""
#define PORTABLE 1
#define DEFAULT_LOCAL_PORT_RANGE_FIRST 27183
#define DEFAULT_LOCAL_PORT_RANGE_LAST 27199
#define HAVE_STRDUP 1

#endif
]=])

add_executable(scrcpy ${SCRCPY_SOURCES})
set_target_properties(scrcpy PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(scrcpy PRIVATE
    "${CONFIG_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src"
    "${SCRCPY_FFMPEG_ROOT}/include"
    "${SCRCPY_SDL3_ROOT}/include"
)

target_compile_definitions(scrcpy PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _WIN32_WINNT=0x0600
    WINVER=0x0600
)

target_compile_options(scrcpy PRIVATE
    /W3
    /clang:-std=gnu11
    /clang:-Wno-gnu-statement-expression
    /clang:-Wno-deprecated-declarations
)

target_link_options(scrcpy PRIVATE /MANIFEST:NO)

target_link_libraries(scrcpy PRIVATE
    "${SCRCPY_FFMPEG_ROOT}/lib/avformat.lib"
    "${SCRCPY_FFMPEG_ROOT}/lib/avcodec.lib"
    "${SCRCPY_FFMPEG_ROOT}/lib/avutil.lib"
    "${SCRCPY_FFMPEG_ROOT}/lib/swresample.lib"
    "${SCRCPY_FFMPEG_ROOT}/lib/openh264.lib"
    "${SCRCPY_SDL3_ROOT}/lib/x64/SDL3.lib"
    ws2_32
    bcrypt
)

set(RUNTIME_FILES
    "${SCRCPY_SDL3_ROOT}/lib/x64/SDL3.dll"
    "${SCRCPY_RUNTIME_DIR}/avcodec-62.dll"
    "${SCRCPY_RUNTIME_DIR}/avformat-62.dll"
    "${SCRCPY_RUNTIME_DIR}/avutil-60.dll"
    "${SCRCPY_RUNTIME_DIR}/swresample-6.dll"
    "${SCRCPY_RUNTIME_DIR}/scrcpy-server"
    "${SCRCPY_RUNTIME_DIR}/adb.exe"
    "${SCRCPY_RUNTIME_DIR}/AdbWinApi.dll"
    "${SCRCPY_RUNTIME_DIR}/AdbWinUsbApi.dll"
)

foreach(runtime_file IN LISTS RUNTIME_FILES)
    if(EXISTS "${runtime_file}")
        add_custom_command(TARGET scrcpy POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${runtime_file}" "$<TARGET_FILE_DIR:scrcpy>"
            VERBATIM)
    endif()
endforeach()

add_custom_command(TARGET scrcpy POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/app/data/scrcpy.png"
            "${CMAKE_CURRENT_SOURCE_DIR}/app/data/disconnected.png"
            "$<TARGET_FILE_DIR:scrcpy>"
    VERBATIM)
