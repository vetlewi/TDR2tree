##############################################
# Include dependencies
#include(ParseAndAddCatchTests)

#add_executable(${CMAKE_PROJECT_NAME}_test
#        src/TDRparser.cpp
#        src/main.cpp)

#target_include_directories(${CMAKE_PROJECT_NAME}_test
#    PRIVATE
#        ${CMAKE_SOURCE_DIR}/external)

#target_link_libraries(${CMAKE_PROJECT_NAME}_test Sort::Buffer Sort::Parameter Sort::Parser)
#ParseAndAddCatchTests(${CMAKE_PROJECT_NAME}_test)