include (UrhoCommon)

file (GLOB CPP_FILES 
        *.cpp
        Machines/*.cpp
        Resource/*.cpp
        Satellites/*.cpp
        Terrain/*.cpp)

file (GLOB H_FILES *.h)


set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

setup_main_executable ()
