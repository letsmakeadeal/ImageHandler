#include <string>
#include "boost/asio.hpp"

class Client {
 public:
  Client(const std::string& input_filepath, const std::string& server_address,
         const int server_port, const std::string& text,
         const std::string& output_filepath,
         boost::asio::io_service& io_service);
  void Connect();
  std::string input_filepath_;
  std::string server_address_;
  std::string text_;
  std::string output_filepath_;
  boost::asio::io_service& io_service_;
  const int server_port_;
};