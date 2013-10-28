# minimum version and module directory
if(WIN32)
    cmake_minimum_required(VERSION 2.8.3)
else(WIN32)
    cmake_minimum_required(VERSION 2.8)
endif(WIN32)

# define project
project(ALVAR)

# set project version
include(version.cmake OPTIONAL)

# set project packaging
if(ALVAR_VERSION) # compiling from src package
    if(ALVAR_PACKAGE) # force package
        set(_package ${ALVAR_PACKAGE})
    else(ALVAR_PACKAGE) # use default src package
        set(_package src)
    endif(ALVAR_PACKAGE)
    set(ALVAR_PACKAGE ${_package} CACHE STRING "ALVAR package type (src: only building source, sdk: building sdk package, bin: building bin package)")
endif(ALVAR_VERSION)

# set library type
if(ALVAR_PACKAGE)
    if(NOT DEFINED ALVAR_STATIC)
        set(ALVAR_STATIC FALSE)
    endif(NOT DEFINED ALVAR_STATIC)
    set(ALVAR_STATIC ${ALVAR_STATIC} CACHE BOOL "ALVAR library type")

    if(ALVAR_STATIC)
        set(BUILD_SHARED_LIBS FALSE)
        add_definitions(-DALVAR_STATIC)
    else(ALVAR_STATIC)
        set(BUILD_SHARED_LIBS TRUE)
    endif(ALVAR_STATIC)
endif(ALVAR_PACKAGE)

# determine compiler
if(ALVAR_PACKAGE)
    if(WIN32)
        file(WRITE "${CMAKE_BINARY_DIR}/return0.cpp"
            "#include <iostream>\n"
            "int main() {\n"
            "  std::cout << _MSC_VER << std::endl;\n"
            "  return 0;\n"
            "}\n"
        )
        try_run(_run_result _compile_result
            "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/return0.cpp"
            RUN_OUTPUT_VARIABLE _output)
        file(REMOVE "${CMAKE_BINARY_DIR}/return0.cpp")
        if(NOT _run_result AND _compile_result AND _output)
            string(REGEX REPLACE "^\([0-9][0-9][0-9][0-9]\).*" "\\1" _version "${_output}")
        endif(NOT _run_result AND _compile_result AND _output)
        if(_version MATCHES "^1400$")
            set(ALVAR_COMPILER vs2005)
        elseif(_version MATCHES "^1500$")
            set(ALVAR_COMPILER vs2008)
        elseif(_version MATCHES "^1600$")
            set(ALVAR_COMPILER vs2010)
        endif(_version MATCHES "^1400$")
    else(WIN32)
        execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpversion" RESULT_VARIABLE _result OUTPUT_VARIABLE _output)
        if(NOT _result AND _output)
            string(REGEX REPLACE "^\([0-9]+\)\\.\([0-9]+\)\\.[0-9]+[^0-9]*" "\\1\\2" _version "${_output}")
        endif(NOT _result AND _output)
        set(ALVAR_COMPILER "gcc${_version}")
    endif(WIN32)
endif(ALVAR_PACKAGE)

# find package dependencies
if($ENV{OPENCV_ROOT})
    set(OpenCV_ROOT_DIR $ENV{OPENCV_ROOT} CACHE PATH "The OpenCV root directory")
endif($ENV{OPENCV_ROOT})
find_package(OpenCV COMPONENTS calib3d core features2d flann highgui imgproc legacy ml video)
if(NOT OpenCV_FOUND)
    find_package(OpenCV COMPONENTS calib3d core features2d flann highgui imgproc legacy ml video
                 NO_MODULE NAMES OpenCV
                 PATHS ${OpenCV_ROOT_DIR} ${OpenCV_ROOT_DIR}/build ${OpenCV_ROOT_DIR}/opencv/build)
    list(GET OpenCV_LIB_DIR 0 OpenCV_ROOT_DIR)
    string(REGEX REPLACE "/lib$" "" OpenCV_ROOT_DIR ${OpenCV_ROOT_DIR})
    if(OpenCV_ROOT_DIR MATCHES x86)
        string(REGEX REPLACE "x86" "common/tbb/ia32" OpenCV_TBB_ROOT_DIR ${OpenCV_ROOT_DIR})
    else(OpenCV_ROOT_DIR MATCHES x86)
        string(REGEX REPLACE "x64" "common/tbb/intel64" OpenCV_TBB_ROOT_DIR ${OpenCV_ROOT_DIR})
    endif(OpenCV_ROOT_DIR MATCHES x86)
    if(NOT OpenCV_FOUND)
        mark_as_advanced(OpenCV_DIR)
        message(FATAL_ERROR "Could NOT find OpenCV")
    endif(NOT OpenCV_FOUND)
endif(NOT OpenCV_FOUND)
if(WIN32)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(WIN32)

if(NOT ALVAR_PACKAGE)
    set(ALVAR_ROOT_DIR ${CMAKE_SOURCE_DIR})
    find_package(ALVAR REQUIRED)
endif(NOT ALVAR_PACKAGE)

if(ALVAR_PACKAGE)
    # add debug definition on unix
    if(UNIX)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            add_definitions(-D_DEBUG)
        endif(CMAKE_BUILD_TYPE MATCHES Debug)
    endif(UNIX)
    
    # do not include manifests for plugins (workaround for windows xp sp2)
    if(MSVC)
        if(MSVC_VERSION GREATER 1310)
            set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
        endif(MSVC_VERSION GREATER 1310)
    endif(MSVC)
endif(ALVAR_PACKAGE)

# set install rpath to current directory such that applications can find libraries that are located in the same directory
SET(CMAKE_INSTALL_RPATH ".")

# disable warning C4251 and C4275 on windows
if(WIN32)
    set(_build_types
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
    )
    foreach(_type ${_build_types})
        string(TOUPPER ${_type} _type)
        set(CMAKE_C_FLAGS_${_type} "${CMAKE_C_FLAGS_${_type}} /wd4251 /wd4275" CACHE STRING "Force static linking of runtime libraries" FORCE)
        set(CMAKE_CXX_FLAGS_${_type} "${CMAKE_CXX_FLAGS_${_type}} /wd4251 /wd4275" CACHE STRING "Force static linking of runtime libraries" FORCE)
    endforeach(_type ${CMAKE_CONFIGURATION_TYPES})
endif(WIN32)

# install runtime libraries
if(ALVAR_PACKAGE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION lib)
endif(ALVAR_PACKAGE)

# hack to setup environment runtime path
set(ALVAR_RUNTIME)
list(APPEND ALVAR_RUNTIME
    ${OpenCV_ROOT_DIR}/bin\;
    ${GLUT_ROOT_PATH}\;
    ${OSG_ROOT_DIR}/bin\;
)
if(OpenCV_TBB_ROOT_DIR)
    list(APPEND ALVAR_RUNTIME
        ${OpenCV_TBB_ROOT_DIR}\;
    )
endif(OpenCV_TBB_ROOT_DIR)
if(NOT ALVAR_PACKAGE)
    list(APPEND ALVAR_RUNTIME
        ${ALVAR_ROOT_DIR}/bin\;
    )
endif(NOT ALVAR_PACKAGE)

# attempt to enable packaging support
if(ALVAR_PACKAGE MATCHES sdk|bin)
    include(CPackConfig.cmake)
    include(CPack)
endif(ALVAR_PACKAGE MATCHES sdk|bin)

# process sub directories
if(ALVAR_PACKAGE)
    add_subdirectory(src)
endif(ALVAR_PACKAGE)

# hack macro to help copy runtime dependencies
macro(copy_runtime _libraries _runtimes _prefixes)
    set(_type none)
    foreach(_library ${_libraries})
        if(_library MATCHES "^optimized$")
            set(_add FALSE)
            set(_type optimized)
        elseif(_library MATCHES "^debug$")
            set(_add FALSE)
            set(_type debug)
        else(_library MATCHES "^optimized$")
            if(_type MATCHES none|optimized)

                string(REGEX REPLACE "^.*/([a-zA-Z0-9_\\-]+)\\.lib$" "\\1" _name "${_library}")
                set(_runtime _runtime-NOTFOUND)
                find_file(_runtime ${_name}.dll PATHS ${_runtimes} NO_DEFAULT_PATH)
                foreach(_prefix ${_prefixes})
                    find_file(_runtime ${_prefix}${_name}.dll PATHS ${_runtimes} NO_DEFAULT_PATH)
                endforeach(_prefix)

                install(FILES ${_runtime} DESTINATION lib)

            endif(_type MATCHES none|optimized)
            set(_type none)
        endif(_library MATCHES "^optimized$")
    endforeach(_library)
endmacro(copy_runtime)

# hack to copy runtime dependencies
if(ALVAR_PACKAGE MATCHES bin AND WIN32)
    copy_runtime("${OpenCV_LIBRARIES}" "${ALVAR_RUNTIME}" "")
    copy_runtime("${GLUT_LIBRARIES}" "${ALVAR_RUNTIME}" "")
    set(_prefixes osg55- osg74- ot11-)
    copy_runtime("${OPENSCENEGRAPH_LIBRARIES}" "${ALVAR_RUNTIME}" "${_prefixes}")
    find_file(_osgplugin osgdb_osg.dll PATHS ${ALVAR_RUNTIME} PATH_SUFFIXES osgPlugins-${OPENSCENEGRAPH_VERSION} NO_DEFAULT_PATH)
    install(FILES ${_osgplugin} DESTINATION bin)
endif(ALVAR_PACKAGE MATCHES bin AND WIN32)
