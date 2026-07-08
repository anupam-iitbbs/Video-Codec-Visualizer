# ivcv_app: Qt 6 desktop shell.
#
# Stage 1 established the docked main window shell described in
# docs/ARCHITECTURE.md, section 5. Stage 2 links in ivcv::ui so MainWindow
# can host real module views (starting with RgbYuvView) instead of only
# the placeholder canvas.

qt_standard_project_setup()

add_executable(ivcv_app
main.cpp
MainWindow.cpp
MainWindow.h
)

target_link_libraries(ivcv_app
PRIVATE
ivcv::core
ivcv::ui
Qt6::Widgets
)

set_target_properties(ivcv_app PROPERTIES
WIN32_EXECUTABLE ON
MACOSX_BUNDLE ON
)
