#includes
include_directories($ENV{CACTUSINCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(/opt/cactus/include/amc13)
#library directory
link_directories(/opt/cactus/lib)

set(LIBS ${LIBS} cactus_amc13_amc13)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__AMC13__")

#find source files
file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cc)

#add the library
add_library(Ph2_Amc13 SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(Ph2_Amc13 ${LIBS})
