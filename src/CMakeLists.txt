add_executable(ponyC main.cpp)

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${Boost_LIBRARY_DIRS})

target_link_libraries(ponyC ${Boost_LIBRARIES}) 


set(Boost_DEBUG 1)
