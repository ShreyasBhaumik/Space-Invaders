cmake_minimum_required(VERSION 3.16)
project(SpaceInvaders VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find SFML ────────────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# ── Executable ───────────────────────────────────────────────
add_executable(space_invaders space_invaders.cpp)

target_link_libraries(space_invaders PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# ── Windows: no console window, copy DLLs next to .exe ───────
if(WIN32)
    set_target_properties(space_invaders PROPERTIES WIN32_EXECUTABLE TRUE)

    # Copy SFML DLLs to output directory after build
    add_custom_command(TARGET space_invaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:space_invaders>
        COMMENT "Copying SFML DLLs..."
    )
endif()

# ── Copy font asset if present ───────────────────────────────
if(EXISTS "${CMAKE_SOURCE_DIR}/PressStart2P-Regular.ttf")
    add_custom_command(TARGET space_invaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/PressStart2P-Regular.ttf"
            $<TARGET_FILE_DIR:space_invaders>
        COMMENT "Copying font..."
    )
endif()

# ── Install rules (for packaging) ────────────────────────────
install(TARGETS space_invaders DESTINATION bin)
if(EXISTS "${CMAKE_SOURCE_DIR}/PressStart2P-Regular.ttf")
    install(FILES PressStart2P-Regular.ttf DESTINATION bin)
endif()

# ── CPack packaging ───────────────────────────────────────────
set(CPACK_PACKAGE_NAME              "SpaceInvaders")
set(CPACK_PACKAGE_VERSION           "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SHORT "Classic Space Invaders in C++ / SFML")
set(CPACK_PACKAGE_VENDOR            "YourName")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Space Invaders")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\Space Invaders\\\\Space Invaders.lnk' '$INSTDIR\\\\bin\\\\space_invaders.exe'")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsfml-dev (>= 2.5)")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "YourName <you@example.com>")
endif()

include(CPack)
