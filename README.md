compile client:
    <g++ client.cpp ./proto/message.pb.cc -o client.out -I. -lboost_system -lprotobuf>

compile server:
    <g++ server.cpp ./proto/message.pb.cc -o server.out -I. -lboost_system -lprotobuf>
