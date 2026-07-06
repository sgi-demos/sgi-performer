add_executable(pr_math_test main.cpp)
# include dirs, the pfcompat force-include, and OpenGL come transitively
# from pr_math's PUBLIC usage requirements
target_link_libraries(pr_math_test PRIVATE pr_math)
