#include <exception>
#include <iostream>
#include <string>
#include "Client.h"
#include <sstream>

void ReadFromStream(std::string* input_filepath, std::string* server_address,
                    int* server_port, std::string* text,
                    std::string* output_filepath) {
  std::string str;
  std::cout << "Input path to initial jpeg file : " << std::endl;
  std::getline(std::cin, *input_filepath);
  std::cout << "Input text : " << std::endl;
  std::getline(std::cin, *text);
  std::cout << "Input server address : " << std::endl;
  std::getline(std::cin, *server_address);
  std::cout << "Input server port : " << std::endl;
  std::getline(std::cin, str);
  std::istringstream istr(str);
  istr >> *server_port;
  std::cout << "Output filepath : " << std::endl;
  std::getline(std::cin, *output_filepath);
  return;
};

int main() {
  std::string input_filepath, server_address, text, output_filepath;
  int  server_port = 0 ;
  boost::asio::io_service io_service;
  ReadFromStream(&input_filepath, &server_address, &server_port, &text,
                 &output_filepath);
  Client client(input_filepath, server_address, server_port, text, output_filepath,
                io_service);
  try {
    client.Connect();
  } catch (std::exception& err) {
    std::cerr << err.what() << std::endl;
  }
  return 0;
}