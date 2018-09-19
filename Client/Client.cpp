#include "Client.h"
#include <QImage>
#include <QString>
#include <fstream>
#include <iostream>
#include "boost/array.hpp"
#include "boost/bind.hpp"

using boost::asio::ip::tcp;
const std::string file_final_sequence = "\r\n\r\n\r\n\r\n\r\n";

Client::Client(const std::string& input_filepath,
               const std::string& server_address, const int server_port,
               const std::string& text, const std::string& output_filepath,
               boost::asio::io_service& io_service)
    : input_filepath_(input_filepath),
      server_address_(server_address),
      server_port_(server_port),
      text_(text),
      output_filepath_(output_filepath),
      io_service_(io_service){};

void Client::Connect() {
  boost::asio::ip::tcp::endpoint ep(
      boost::asio::ip::address::from_string(server_address_), server_port_);
  boost::system::error_code ignored_error;
  tcp::socket sock(io_service_);
  sock.connect(ep);
  int buff_length = 1024;
  char* buffer = new char[buff_length];
  // Checking input file
  std::ifstream file(input_filepath_.c_str());
  file.seekg(0,std::ios::end);
  size_t size_file = file.tellg();
   if (!file.is_open() || size_file == 0) {
    if (size_file == 0){ 
      std::cerr << "File is empty" << std::endl; }
      else {
        std::cerr << "Stream can't open file" << std::endl;
      }
    boost::asio::write(sock, boost::asio::buffer(file_final_sequence),
                       boost::asio::transfer_all(), ignored_error);
    sock.close();
    return;
  }

  std::ofstream f2("file");
  file.seekg(0);
  f2.seekp(0);

  //Adding string to file
  f2 << text_ + '\n';
  
  char ch;
  while (file && file.get(ch)) f2.put(ch);
  f2.close();

  std::ifstream file_last("file", std::ios::binary | std::ios::in);
  while (file_last) {
    file_last.read(buffer, buff_length);
    boost::asio::write(sock, boost::asio::buffer(buffer, file_last.gcount()),
                       boost::asio::transfer_all(), ignored_error);
  }
  boost::asio::write(sock, boost::asio::buffer(file_final_sequence),
                     boost::asio::transfer_all(), ignored_error);

  boost::asio::streambuf response;
  std::ofstream out_file(output_filepath_, std::ofstream::binary);
  boost::asio::read_until(sock, response, file_final_sequence);
  out_file << &response;

  std::remove("file");
  sock.close();
};