# asio-chat 💬
Chat application, implemented using asynchronous Boost::Asio library with handful of modern C++ features.
Built using CMake.

## Building the project

1. Create a folder to build the project: `mkdir build`
2. Go to that folder: `cd build`
3. `cmake ..`
4. `make`
5. Start the server using `./server`
6. Start the client using `./client`

## File structure
The project constists of framework, and client/server applications on top of it.
The framework (`framework` folder):
* `Framework.h` -- the main header file of the framework. It includes all the other files from this list.
* `Common.h` -- includes common libraries needed from the framework, such as `mutex`, `chrono`, etc.
* `MessageHandler.h` -- describes the Message structure, templated by the enum of MessageType.
* `Connection.h` -- describes both Server-Client connection and Client-Server connection.
* `ThreadSafeDeque.h` -- Thread-safe Deque, implemented using std::mutex and std::scoped_lock. 
* `Client.h` -- Implementation of Client
* `Server.h` -- Server's implementation

## Usage
TODO
⚠️ The project is too raw to be used. Both client and server are full of bugs.
