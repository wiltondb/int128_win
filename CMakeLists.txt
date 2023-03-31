# Copyright 2023 alex@staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 3.0 )

project ( int128_win C )

enable_testing()

add_executable ( int128_win_test
    test.c
    uint128_win.h
    int128_win.h )

target_include_directories ( int128_win_test BEFORE PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR} )

add_test ( int128_win_test
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/int128_win_test.exe )
