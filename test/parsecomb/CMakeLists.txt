add_executable(parsecomb_test parsecomb_test.cpp)
target_link_libraries(parsecomb_test PRIVATE parsecomb)

add_executable(json_test json_test.cpp)
target_link_libraries(json_test PRIVATE parsecomb)

AddTests(parsecomb_test)
AddTests(json_test)

