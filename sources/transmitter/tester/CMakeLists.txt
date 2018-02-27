########################################################################
#
# GPIO-Wire for Cheap RF communications
# Copyright (C) 2016-2018  Antonio Petricca <antonio.petricca@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
######################################################################## 

# Define project

cmake_minimum_required(VERSION 2.8)
project(Tester CXX)

# Globals

set(_TARGET_NAME "gpiowire-tester")
set(_TARGET_VERSION "\"0.0.1-DEBUG\"")

# Project files

include_directories(../library)

file(GLOB _LIBRARY_TESTER_SOURCES ../library/*.cpp)
file(GLOB _TESTER_SOURCES *.cpp)

# Build executable

add_executable(
  ${_TARGET_NAME} 
  ${_LIBRARY_TESTER_SOURCES}
  ${_TESTER_SOURCES}
)

target_link_libraries(
  ${_TARGET_NAME}
  ${_GLOBAL_CLIENT_TARGET_NAME}
)
