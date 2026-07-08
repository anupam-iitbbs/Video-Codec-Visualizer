# ivcv_core: pure C++20 algorithm library, no Qt dependency.
#
# Every pipeline stage's algorithm lives here so it can be unit-tested and
# reused independently of the Qt UI (see docs/ARCHITECTURE.md, section 1).

add_library(ivcv_core
src/PipelineContext.cpp
src/ColorSpaceConverter.cpp
src/ImageLoader.cpp
)

add_library(ivcv::core ALIAS ivcv_core)

target_include_directories(ivcv_core
PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:include>
)

target_link_libraries(ivcv_core
PUBLIC
Eigen3::Eigen
opencv_core
opencv_imgproc
opencv_imgcodecs
fmt::fmt
spdlog::spdlog
)

target_compile_features(ivcv_core PUBLIC cxx_std_20)

set_target_properties(ivcv_core PROPERTIES
CXX_EXTENSIONS OFF
)
