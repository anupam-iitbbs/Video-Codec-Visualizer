# ivcv_ui: Qt 6 module views (RgbYuvView, and future DctView,
# QuantizationView, and so on).
#
# Kept as its own library, separate from ivcv_app, so module views can be
# unit-tested (e.g. with QTest) independently of the main window shell,
# and so ivcv_app stays a thin composition root (see
# docs/ARCHITECTURE.md, section 1).

add_library(ivcv_ui
    RgbYuvView.cpp
    ChromaSubsamplingView.cpp
    BlockGridPanel.cpp
    BlockPartitioningView.cpp
)

add_library(ivcv::ui ALIAS ivcv_ui)

target_include_directories(ivcv_ui
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(ivcv_ui
    PUBLIC
    ivcv::core
    Qt6::Widgets
)

target_compile_features(ivcv_ui PUBLIC cxx_std_20)

set_target_properties(ivcv_ui PROPERTIES
    CXX_EXTENSIONS OFF
)
