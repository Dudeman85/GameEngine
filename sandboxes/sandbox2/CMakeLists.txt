add_executable(sandbox2 OpenGL_TestScene.cpp SFML_Tiled_TestScene.cpp GLCheck.hpp main.cpp TextRendering_TestScene.cpp ENetTest_Client.cpp ENetTest_Server.cpp)
target_link_libraries(sandbox2 engine)