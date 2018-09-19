#pragma once
#include <QMutex>
#include <QString>
#include <boost/thread.hpp>
#include "MainWindow.h"
#include "boost/asio.hpp"

class GUIUpdater;

class Server {
 public:
  Server(GUIUpdater* gui);
  void SetPort(const int port);
  void Run();
  ~Server();

 private:
  void ProcessPicture(boost::asio::ip::tcp::socket sock);
  void IncrementProcessedPics();
  void PushAddressToProcessing(const boost::asio::ip::address);
  void PopAddressToProcessing(const boost::asio::ip::address);
  void CalcAverageSpeed(const int pic_proc_time, const int receive_time,
                        const int send_time);
  QString TransormFromVecToQStr();

 public:
  GUIUpdater* gui_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  QMutex mutex_;
  QMutex mutex1_;
  int server_port_;
  std::vector<std::string> ip_adresses_;
  int pictures_processed_;
  float speed_summ_;
  float send_time_summ_;
  float receive_time_summ_;
  std::vector<std::thread> clients_;
  bool running_flag_;
};