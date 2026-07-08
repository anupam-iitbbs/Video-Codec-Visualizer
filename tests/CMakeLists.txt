# Unit tests mirroring the core/ library structure (see docs/SRS.md, NFR-5:
# minimum 80% coverage target on core/).

add_executable(ivcv_core_tests
    core/PipelineContextTest.cpp
)

target_link_libraries(ivcv_core_tests
    PRIVATE
        ivcv::core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ivcv_core_tests)
