#include "Server.h"
#include <qtconcurrentrun.h>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QUrl>
#include <QtConcurrent>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include "boost/array.hpp"
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

const std::string file_final_sequence = "\r\n\r\n\r\n\r\n\r\n";

Server::Server(GUIUpdater* gui)
    : gui_(gui),
      acceptor_(io_service_),
      pictures_processed_(0),
      speed_summ_(0),
      send_time_summ_(0),
      receive_time_summ_(0),
      server_port_(0),
      running_flag_(1){};

void Server::SetPort(const int port) { server_port_ = port; }

void Server::ProcessPicture(boost::asio::ip::tcp::socket sock) {
  try {
    boost::posix_time::ptime full_beg =
        boost::posix_time::microsec_clock::local_time();

    // Getting ip
    boost::asio::ip::tcp::endpoint remote_ep = sock.remote_endpoint();
    boost::asio::ip::address remote_ad = remote_ep.address();
    PushAddressToProcessing(remote_ad);

    std::ofstream file("buf1", std::ofstream::binary);
    boost::asio::streambuf buffer;
    boost::system::error_code error;

    boost::posix_time::ptime receive_beg =
        boost::posix_time::microsec_clock::local_time();
    boost::asio::read_until(sock, buffer, file_final_sequence);
    boost::posix_time::ptime receive_end =
        boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_receive = receive_end - receive_beg;
    file << &buffer;
    file.close();

    std::fstream file1("buf1", std::ifstream::in);
    std::ofstream file2("picture1", std::ofstream::out);

    std::string my_string;
    std::getline(file1, my_string);
    char ch;
    while (file1 && file1.get(ch)) file2.put(ch);
    file2.close();
    file1.close();

    std::rename("picture1", "picture");
    {
      QImage image("picture");
      if (image.isNull()) {
        std::cerr << "Null image " << std::endl;
        PopAddressToProcessing(remote_ad);
        std::remove("picture");
        std::remove("buf1");
        sock.close();
        return;
      }

      QPainter* painter = new QPainter();
      if (!painter->begin(&image)) return;
      painter->setPen(QPen(Qt::green));
      painter->setFont(QFont("Arial", 30));
      painter->drawText(image.rect(), Qt::AlignLeft | Qt::AlignTop,
                        my_string.c_str());
      painter->end();
      image.save("pc1", "JPEG", 100);
    }

    boost::system::error_code ignored_error;
    const int send_buff_length = 1024;
    char* send_buffer = new char[send_buff_length];
    std::ifstream out_file("pc1", std::ios::in | std::ios::binary);

    boost::posix_time::ptime send_beg =
        boost::posix_time::microsec_clock::local_time();
    while (out_file) {
      out_file.read(send_buffer, send_buff_length);
      boost::asio::write(sock,
                         boost::asio::buffer(send_buffer, out_file.gcount()),
                         boost::asio::transfer_all(), ignored_error);
    }
    boost::asio::write(sock, boost::asio::buffer(file_final_sequence),
                       boost::asio::transfer_all(), ignored_error);
    boost::posix_time::ptime send_end =
        boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_send = send_end - send_beg;

    std::remove("picture");
    std::remove("pc1");
    std::remove("buf1");
    IncrementProcessedPics();
    boost::posix_time::ptime full_end =
        boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration full_time = full_end - full_beg;
    CalcAverageSpeed(full_time.total_milliseconds(),
                     time_receive.total_milliseconds(),
                     time_send.total_milliseconds());
    PopAddressToProcessing(remote_ad);
    sock.close();
  } catch (std::exception& err) {
  }
};

void Server::IncrementProcessedPics() {
  mutex_.lock();
  std::ostringstream str;
  ++pictures_processed_;
  str << pictures_processed_ << " pictures processed ";
  gui_->IncrementProcessedPics(str.str().c_str());
  mutex_.unlock();
};

void Server::PushAddressToProcessing(const boost::asio::ip::address remote_ad) {
  mutex1_.lock();
  std::string s = remote_ad.to_string();
  ip_adresses_.push_back(s);
  gui_->UpdateProcessingPics(TransormFromVecToQStr());
  mutex1_.unlock();
}

void Server::PopAddressToProcessing(const boost::asio::ip::address remote_ad) {
  mutex1_.lock();
  std::string s = remote_ad.to_string();
  auto it = std::find(ip_adresses_.begin(), ip_adresses_.end(), s);
  if (it != ip_adresses_.end()) ip_adresses_.erase(it);
  gui_->UpdateProcessingPics(TransormFromVecToQStr());
  mutex1_.unlock();
}

void Server::CalcAverageSpeed(const int pic_proc_time, const int receive_time,
                              const int send_time) {
  mutex_.lock();
  std::ostringstream str_av_proc, str_av_rec, str_av_send;
  speed_summ_ += pic_proc_time;
  send_time_summ_ += send_time;
  receive_time_summ_ += receive_time;
  str_av_proc << "Average picture processing speed(msec): ";
  str_av_proc << speed_summ_ * 1.0 / pictures_processed_;
  str_av_rec << "Average sending speed(msec): ";
  str_av_rec << receive_time_summ_ * 1.0 / pictures_processed_;
  str_av_send << "Average receiving speed(msec): ";
  str_av_send << send_time_summ_ * 1.0 / pictures_processed_;
  gui_->UpdateAveragePicProcSpeed(str_av_proc.str().c_str());
  gui_->UpdateAverageRecSpeed(str_av_rec.str().c_str());
  gui_->UpdateAverageSendSpeed(str_av_send.str().c_str());
  mutex_.unlock();
}

QString Server::TransormFromVecToQStr() {
  std::ostringstream str;
  const size_t size = ip_adresses_.size();
  for (size_t i = 0; i < size; ++i) {
    str << ip_adresses_[i] << '\n';
  }
  return QString(str.str().c_str());
};

void Server::Run() {
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(),
                                          server_port_);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  while (running_flag_) {
    auto socket =
        std::make_shared<boost::asio::ip::tcp::socket>(std::ref(io_service_));
    acceptor_.accept(*socket);
    clients_.emplace_back(std::thread(
        [socket, this]() { return ProcessPicture(std::move(*socket)); }));
    socket.reset();
  };
};

Server::~Server() {
  running_flag_ = false;
  const size_t threads_size = clients_.size();
  for (size_t i = 0; i < threads_size; ++i) {
    clients_[i].join(); 
  }
}
