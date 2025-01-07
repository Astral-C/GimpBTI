cmake_minimum_required(VERSION 3.8)
project(file-bti)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/gctoolsplusplus)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIMP REQUIRED
    gimp-2.0>=2.8
    gimpui-2.0>=2.8
)


file(GLOB_RECURSE BTI_SRC
    "src/dialog.cpp"
    "src/plugin-bti.cpp"
    "include/*.hpp"
    "include/*.h"
)

file(GLOB_RECURSE TPL_SRC
    "src/dialog.cpp"
    "src/plugin-tpl.cpp"
    "include/*.hpp"
    "include/*.h"
)

add_executable(file-bti ${BTI_SRC})
target_include_directories(file-bti PUBLIC include lib/gctoolsplusplus/lib/bStream lib/gctoolsplusplus/include ${GIMP_INCLUDE_DIRS})
target_link_libraries(file-bti ${GIMP_LIBRARIES} gctools++)
link_directories(${GIMP_LIBRARY_DIRS})
execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=gimplibdir gimp-2.0 RESULT_VARIABLE PKG_CONFIG_RESULT OUTPUT_VARIABLE GIMP_LIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(PKG_CONFIG_RESULT)
    message(WARNING "gimplibdir not set, install target unavailable")
else()
    install(TARGETS file-bti RUNTIME DESTINATION "${GIMP_LIB_DIR}/plug-ins")
endif()


add_executable(file-tpl ${TPL_SRC})
target_include_directories(file-tpl PUBLIC include lib/gctoolsplusplus/lib/bStream lib/gctoolsplusplus/include ${GIMP_INCLUDE_DIRS})
target_link_libraries(file-tpl ${GIMP_LIBRARIES} gctools++)


execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=gimplibdir gimp-2.0 RESULT_VARIABLE PKG_CONFIG_RESULT OUTPUT_VARIABLE GIMP_LIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(PKG_CONFIG_RESULT)
    message(WARNING "gimplibdir not set, install target unavailable")
else()
    install(TARGETS file-tpl RUNTIME DESTINATION "${GIMP_LIB_DIR}/plug-ins")
endif()
