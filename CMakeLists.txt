cmake_minimum_required(VERSION 3.10)

#set project name
project(WebServer)

#set include dirs
include_directories(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/MyNet/Events/
    ${PROJECT_SOURCE_DIR}/MyHTTP/HTTP/
    ${PROJECT_SOURCE_DIR}/MyNet/Log/
    ${PROJECT_SOURCE_DIR}/MyHTTP/Thread/
    ${PROJECT_SOURCE_DIR}/Utils/
    ${PROJECT_SOURCE_DIR}/MyHTTP/DataBase/
    /usr/include/mysql/
)

# set src lists
aux_source_directory(${PROJECT_SOURCE_DIR}/MyNet/Events Events)
aux_source_directory(${PROJECT_SOURCE_DIR}/MyHTTP/HTTP HTTP)
aux_source_directory(${PROJECT_SOURCE_DIR}/MyNet/Log Log)
aux_source_directory(${PROJECT_SOURCE_DIR}/MyHTTP/Thread Thread)
aux_source_directory(${PROJECT_SOURCE_DIR}/Utils Utils)
aux_source_directory(${PROJECT_SOURCE_DIR}/MyHTTP/DataBase DataBase)

#add executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${Events}
    ${HTTP}
    ${Log}
    ${Thread}
    ${Utils}
    ${DataBase}
)

# set lib
target_link_libraries(${PROJECT_NAME} 
    -lpthread
    mysqlclient
)

#add options
add_definitions("-g")