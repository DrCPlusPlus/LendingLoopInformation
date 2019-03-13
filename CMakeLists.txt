cmake_minimum_required (VERSION 2.6)
project (LendingLoopInformation)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_BUILD_TYPE Debug)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)


set(CMAKE_VERBOSE_MAKEFILE ON)
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
#include_directories("${PROJECT_SOURCE_DIR}/../SocketLibrary/SocketLib")
#link_directories("${PROJECT_SOURCE_DIR}/../SocketLibrary/build")

#http://www.voidcn.com/article/p-ollmkxzl-ya.html
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(LendingLoopInformation src/main.cpp src/WebStuff/WebConnector.cpp src/LendingLoop/LoopConnector.cpp src/LendingLoop/LenderManipulator.cpp 
                                    src/Displayables/LineItem.cpp src/Displayables/DisplayLineItem.cpp src/Base64.cpp src/BrowserLaunch.cpp
                                    src/Windows/Settings.cpp src/Windows/LogIn.cpp src/GuiComponents/AutoCompleteTextBox.cpp
                                    src/GuiComponents/MessageBox.cpp
                                    src/GuiComponents/DialogComponents.cpp
                                    src/LendingLoopInformation.cpp
                                    src/AllPaymentsList.cpp src/Windows/PopUpSummary.cpp src/Windows/CalendarPopUp.cpp src/GuiComponents/ContextMenu.cpp
                                    src/Windows/uiAndIcon.cpp)
target_link_libraries(LendingLoopInformation curl)
target_link_libraries(LendingLoopInformation pthread)
target_link_libraries(LendingLoopInformation ${GTK3_LIBRARIES})
#target_link_libraries(LendingLoopInformation SocketLib)

set_target_properties(LendingLoopInformation PROPERTIES COMPILE_FLAGS "-std=c++11 -D_FILE_OFFSET_BITS=64" LINK_FLAGS "-std=c++11 -D_FILE_OFFSET_BITS=64")
