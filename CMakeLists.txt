# We require CMake version >= 2.8
CMake_Minimum_Required(VERSION 2.8)

set(CMAKE_USE_RELATIVE_PATHS ON)

# Project name
Project(XPMaster)

# declare final executable name
Set( prog_Binary "xpmaster.bin" )

# setup gcc biased compiler flags
Set( CMAKE_CXX_FLAGS_DEBUG "-Wall -g" )
Set( CMAKE_CXX_FLAGS_RELEASE  "-Wall -O2" )

If( CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" )
	Message(STATUS "Info: Debug build" )

	# for debug type builds, turn on verbose makefiles
#	Set(CMAKE_VERBOSE_MAKEFILE ON)
	
	# Tell other CMake files that we're doing a debug build
	Set(DEBUG_BUILD 1)

	# Tell C/C++ that we're doing a debug build
	Add_Definitions( "-DDEBUG" )
	
Else()
	Message(STATUS "Info: Release build" )
	
	# Tell C/C++ that we're doing a release build
	Add_Definitions( "-DRELEASE" )
	
EndIf()

# set additional gcc compiler flags specific to the OMAP44xx(Pandaboard ES) and iMX.6
#    Target is an ARM Cortex A9
#    Tune code performance same as the compiler was built for
#    Use NEON hardware for floating point
#Set( CMAKE_CXX_FLAGS "-mcpu=cortex-a9 -mtune=native -mfpu=neon" )
Set( CMAKE_CXX_FLAGS "-std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti -mcpu=cortex-a9 -mfpu=neon -Wall" )

Set( CMAKE_EXE_LINKER_FLAGS "-fno-exceptions -fno-rtti" )

# ToDo: add external libs pull in code here


# set list of library dependencies this project requires
Set(prog_Libs
	"-lpthread"
	"-ldl"
	"-lutil"
	"-lm"
	"-lpython2.7"
    "-lusb-1.0"
)

# bring in all source files under our project's src/ directory
File(GLOB_RECURSE sourceC   "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
File(GLOB_RECURSE sourceCPP "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

Set(prog_Sources
    ${sourceC}
    ${sourceCPP}
)

# set headers path
Include_Directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
	"/usr/include/python2.7"
#    ${FREETYPE_INCLUDE_DIRS}
#    "${CMAKE_CURRENT_SOURCE_DIR}/libs/ftgles/src"
)

# create executable for project
Add_Executable( ${prog_Binary} ${prog_Sources} )
Target_Link_Libraries( ${prog_Binary} ${prog_Libs})

# Done
