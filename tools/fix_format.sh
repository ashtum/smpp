find ../include/ -iname *.hpp | xargs clang-format -i
find ../test/ -iname *.cpp | xargs clang-format -i
find ../example/ -iname *.cpp | xargs clang-format -i
