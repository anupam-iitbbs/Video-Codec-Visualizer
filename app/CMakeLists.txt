# ivcv_app: Qt 6 desktop shell.
#
# Stage 1 scope: establishes the docked main window shell described in
# docs/ARCHITECTURE.md, section 5. Module views are wired in starting with
# Stage 2 (RGB to YUV).

qt_standard_project_setup()

add_executable(ivcv_app
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_link_libraries(ivcv_app
    PRIVATE
        ivcv::core
        Qt6::Widgets
)

set_target_properties(ivcv_app PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
