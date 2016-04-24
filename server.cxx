#include "server_http.hpp"
#include "hw_interface.hxx"

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//Added for the default_resource example
#include <fstream>
#include <boost/filesystem.hpp>
#include <vector>
#include <algorithm>

using namespace std;
//Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;

int main() {
    //HTTP-server at port 8080 using 4 threads
    HwInterface hwInterface;

    HttpServer server(8080, 4);

    server.resource["^/query$"]["GET"]=[&hwInterface](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        stringstream content_stream;
        content_stream << "<h1>Request from " << request->remote_endpoint_address << " (" << request->remote_endpoint_port << ")</h1>";
        content_stream << request->method << " " << request->path << " HTTP/" << request->http_version << "<br>";
        auto q = request->header.find("Query");
        if(q != request->header.end()){
            hwInterface.parseQuery(q->second);
            content_stream << (request->header.find("Query"))->second;
        }
        for(auto& header: request->header) {
            content_stream << header.first << ": " << header.second << "<br>";
        }
        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);
        
        response <<  "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n" << content_stream.rdbuf();
    };

    //Default get request
    server.default_resource["GET"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << 0 << "\r\n\r\n";
    };
    
    thread server_thread([&server](){
        //Start server
        server.start();
    });
    
    //Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));
        
    server_thread.join();

    return 0;
}