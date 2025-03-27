### Introduction
PUSimulator is a device simulator project based on BVCSP.
Purpose:
* Open source code is used for BVCSP users to understand how to develop device-side.
* Used to test BVCSP functions.
* Used to simulate device test platform performance.

### Directory Description
```
bin Generate/debug directory
build sln project file/compilation temporary file directory
include BVCSP library related header file directory
lib BVCSP library related dll/lib directory
src simulator source code directory
```
> Normally, you only need to modify the code in src according to the situation, and do not need to modify the code in the subdirectory of src, which is the encapsulation of the bvcsp interface.

### Compilation Instructions

The project is a Windows Visual Studio 2010 project, and it is recommended to use Visual Studio 2010 to compile.
The project project files are in the build directory.

### Run

1. Modify the pusimulator.ini configuration file in the bin directory. Key points: id (device ID number) ip (online server address) port (online server port)
2. Automatically copy the file to run: Double-click run.bat in the bin directory to run.
3. Manually copy the file to run: Copy the dll file in the lib directory to the bin directory. Double-click PUSimulator.exe to run.
> BVCSP library depends on msvcr100.dll and msvcp100.dll (if there is no vs2010 development environment, you can install the operating environment).
PUSimulator.exe is open source, and the specific dependent operating environment needs to be determined according to your development environment.
BVCSP is a paid library and requires authentication before it can be put on the server. For authentication, you need to contact the salesperson (auth_code must be provided, see the printout).
### Development process
1. Set the log callback interface.
2. Initialize the library.
3. Authentication.
4. Implement virtual functions of related objects such as commands and channels.
5. Set device information and channel objects.
6. Go online to the server.
7. Go offline.
8. Release the library.

### Initialize the library
* If you use the library's internal thread, the caller does not need to call the BVCSP_HandleEvent interface.
> Note that the library callback comes from the library's internal thread, and be careful to handle synchronization issues to prevent deadlock.

* If you use the library's external thread, the caller needs to call the BVCSP_HandleEvent interface "in a fixed thread".
> Note that you must wait until BVCSP_HandleEvent is called (the library determines that the working thread has started) before calling other interfaces, otherwise you will receive a reply of "-65532".

### Authentication
BVCSP is a paid library and requires authentication before going online to the server. You need to contact the salesperson for authentication (auth_code is required).
The authentication-related code is in auth.cpp, and you need to make the following changes:

* In the hardware information callback interface, fill in your real device hardware information.
* In the Auth interface, fill in the developer key information you applied for (need to contact the sales staff for allocation).
> The same hardware information will be considered as the same device, the authentication information is bound to the hardware information, and only one valid hardware information platform is allowed at the same time.

### Set device information, channel object
The device/channel information is set in loginout.cpp, and you need to make the following changes:
* Modify config.cpp to implement the configuration save and read functions (the default is the .ini configuration file under Windows).
* The login interface in loginout.cpp implements device/channel information registration!!!!

### Online/offline server
It has been implemented in loginout.cpp and can be modified according to needs.

### GPS
GPS related functions are implemented in gps.*. According to the comments, the virtual function interface of the relevant class is implemented, which can support the following functions.
* GPS channel support.
* Setting GPS channel name support.
* Query GPS positioning data command support.
* Query/set GPS configuration support.

### Serial port (TSP)
Serial port related functions are implemented in tsp.*. According to the comments, the virtual function interface of the relevant class is implemented, which can support the following functions.
* Serial port channel support.
* Setting serial port channel name support.
* Query/set serial port attribute support.
