cmake_minimum_required(VERSION 3.10)

project(cutils LANGUAGES C)

enable_testing()

find_program(LCOV lcov)
find_program(GENHTML genhtml)
add_custom_target(cutils_coverage
	COMMAND ${LCOV} -q -c -d ${CMAKE_BINARY_DIR} -o ${CMAKE_BINARY_DIR}/lcov.info
	COMMAND ${GENHTML} -q ${CMAKE_BINARY_DIR}/lcov.info -o ${CMAKE_SOURCE_DIR}/report/coverage
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(NOT MSVC)
	if(ARCH STREQUAL "x64")
		set(CMAKE_C_FLAGS "-m64")
	elseif(ARCH STREQUAL "x86")
		set(CMAKE_C_FLAGS "-m32")
	endif()
endif()

if(WIN32)
    set(LIB_EXT ".lib")
else()
	set(LIB_EXT ".a")
endif()

include(FetchContent)
FetchContent_Declare(ctest
  GIT_REPOSITORY    https://github.com/cgware/ctest.git
  GIT_TAG           main
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/deps/ctest
)
FetchContent_MakeAvailable(ctest)

file(GLOB_RECURSE cutils_src src/*.h src/*.c)
add_library(cutils ${cutils_src})
target_link_libraries(cutils PUBLIC cbase)
target_include_directories(cutils PUBLIC include ${CMAKE_SOURCE_DIR}/deps/ctest/deps/cbase/include)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cutils PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cutils PRIVATE -coverage)
	else()
		target_compile_options(cutils PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cutils PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cutils/${ARCH}-Debug/
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cutils/${ARCH}-Release/
	PREFIX ""
)

file(GLOB_RECURSE cutils_test_src test/*.h test/*.c)
add_executable(cutils_test ${cutils_test_src})
target_link_libraries(cutils_test cutils ctest)
target_include_directories(cutils PUBLIC include ${CMAKE_SOURCE_DIR}/deps/ctest/include)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cutils_test PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cutils_test PRIVATE -coverage)
	else()
		target_compile_options(cutils_test PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cutils_test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cutils_test/${ARCH}-Debug/
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cutils_test/${ARCH}-Release/
)
add_test(cutils_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target cutils_test)
add_test(cutils ${CMAKE_SOURCE_DIR}/bin/cutils_test/${ARCH}-${CMAKE_BUILD_TYPE}/cutils_test)
set_tests_properties(cutils PROPERTIES DEPENDS cutils_build)
