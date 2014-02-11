# REQUIREMENT: Assuming the 'astyle' version 2.04 executable is on the PATH
#   You can check this by typing
#       astyle --version
#
# USAGE: Call this script by
#          sh format-cpp-code.sh
#        or
#          chmod u+rwx format-cpp-code.sh
#          ./format-cpp-code.sh
#
# Refer to ../sailsmart/.. so that we are sure we apply it to the
# _right_ src directory
astyle --style=google --indent=spaces=2 --recursive ../sailsmart/src/*.cpp ../sailsmart/src/*.h
