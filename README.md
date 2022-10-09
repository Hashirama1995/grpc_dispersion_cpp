# grpc_dispersion_cpp
An example of distributed variance calculation. Protocol grpc. C++ language

## 1. Pre-Environment Setup
This project requires `vcpkg` a package manager to be installed. VCPKG is used to manage your packages dependencies for projects like Protobufs and gRPC. If you already have vcpkg manager installed then continue to [2. Compile](https://github.com/advra/grpc-vcpkg-boilerplate-example/blob/main/README.md#2-compile), otherwise continuing reading.

## 2. Steps
1. git clone https://github.com/microsoft/vcpkg
2. cd vcpkg/
3. ./vcpkg install grpc
4. ./vcpkg install protobuf   (Once installed you can run `./vcpkg list` to ensure all the packages have successfully installed.)
5. ./vcpkg integrate install
6. ./vcpkg install protobuf[zlib] protobuf[zlib]:x64-windows
7. In CMakeLists.txt , which lies in the root folder, you must specify the path to the folder where you have vcpkg.exe 
(**example** - set (VCPKG_HOME "C:/Users/UserName/Desktop/vcpkg/vcpkg") #IMPORTANT: change this to where your vcpkg exists.
8. mkdir build      (create build in the root folder create folder "build" )
9. cd build
10. cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/aa/vcpkg/scripts/buildsystems/vcpkg.cmake (generate solution)
11. cmake --build . (build project)
12. NICE! Almost ready! Go to folder ..\build\src\Debug
13. The folder should contain  .exe files of the server and client (greeter_server.exe, greeter_client.exe)
14.  ./greeter_server (Start the server first)
15.  ./greeter_client (Them start first client)
16.  When starting a client, give this client a new ID.

Ready! Launch new clients, the variance will be automatically recalculated.
Shards have 5 values. Values are hardcoded. Everything can be changed in the source code.
Values are generated randomly.
