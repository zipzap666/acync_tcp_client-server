compile client:
    
    g++ client.cpp ../common/message.pb.cc -o client.out -I. -lboost_system -lprotobuf

compile server:

    g++ server.cpp session.cpp ../common/message.pb.cc -o server.out -I. -lboost_system -lprotobuf
