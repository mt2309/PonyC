add_executable(ponyC Parser.cc Lexer.cc Main.cc TypeChecker.cc CompilationUnit.cc Loader.cc)

FIND_PACKAGE(Boost COMPONENTS program_options filesystem system REQUIRED)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${Boost_LIBRARY_DIRS})

target_link_libraries(ponyC ${Boost_LIBRARIES})


set(Boost_DEBUG 1)
