#include "MainWindow.h"
#include <stdlib.h>
#include <unistd.h>
#include <QCoreApplication>
#include <QDebug>
#include <QFuture>
#include <QPalette>
#include <QtConcurrent>
#include <sstream>
#include <QInputDialog>
/* Main window implementation */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), number_of_done_pics_(0) {
  bool bOk; 
     int port = QInputDialog::getInt(this, tr("Set server port"),
                                 tr("Port"), 8031, 1024, 49151, 1, &bOk); 
  if (!bOk) { 
    exit(0);
   }

  updater_ = new GUIUpdater(this);
  updater_ -> GetServer() -> SetPort(port);
  updater_ -> RunServer();
  std::ostringstream msg;
  msg << "Server is working on port  ";
  msg << port ;
  port_messages_ = new QLabel(msg.str().c_str(), this);
  nDonePics_ = new QLabel("0 done pictures", this);
  sourcesLabel_ = new QLabel("Processing sources : ", this);
  sources_ = new QLabel("Sources", this);
  avSpeedPicProcess_ = new QLabel("Average picture processing speed(msec): ", this);
  netAvReceiveSpeed_ = new QLabel("Average receiving speed(msec): ", this);
  netAvSendSpeed_ = new QLabel("Average sending speed(msec): ", this);

  QPalette sample_palette;
  sample_palette.setColor(QPalette::WindowText, Qt::blue);
  sourcesLabel_->setPalette(sample_palette);
  nDonePics_->setPalette(sample_palette);
  sample_palette.setColor(QPalette::WindowText, Qt::red);
  port_messages_ -> setPalette(sample_palette);

  QWidget *centralArea = new QWidget(this);
  this->setCentralWidget(centralArea);
  layout_ = new QVBoxLayout(this);
  
  layout_->addWidget(port_messages_);
  layout_->addWidget(nDonePics_);
  layout_->addWidget(sourcesLabel_);
  layout_->addWidget(sources_);
  layout_->addWidget(avSpeedPicProcess_);
  layout_->addWidget(netAvReceiveSpeed_);
  layout_->addWidget(netAvSendSpeed_);
  centralArea->setLayout(layout_);

  // Connecting signals to server processes
  connect(updater_, SIGNAL(RequestChangeLabel(QString)), this,
          SLOT(IncrementProcessedPics(QString)));
  connect(updater_, SIGNAL(RequestChangeProcIp(QString)), this,
          SLOT(UpdateProcIpAdresses(QString)));
  connect(updater_, SIGNAL(RequestUpdtAvSpeed(QString)), this,
          SLOT(UpdateAveragePicProcSpeed(QString)));
  connect(updater_, SIGNAL(RequestUpdtSendSpeed(QString)), this,
          SLOT(UpdateAverageSendSpeed(QString)));
  connect(updater_, SIGNAL(RequestUpdtRecSpeed(QString)), this,
          SLOT(UpdateAverageRecSpeed(QString)));
};

void MainWindow::IncrementProcessedPics(const QString &string) {
  nDonePics_->setText(string);
};

void MainWindow::UpdateProcIpAdresses(const QString &string) {
  sources_->setText(string);
};

void MainWindow::UpdateAveragePicProcSpeed(const QString &string) {
  avSpeedPicProcess_->setText(string);
};

void MainWindow::UpdateAverageRecSpeed(const QString &string) {
  netAvReceiveSpeed_->setText(string);
};
void MainWindow::UpdateAverageSendSpeed(const QString &string) {
  netAvSendSpeed_->setText(string);
};

MainWindow::~MainWindow(){
  delete nDonePics_;
  delete sourcesLabel_;
  delete avSpeedPicProcess_;
  delete sources_;
  delete netAvReceiveSpeed_;
  delete netAvSendSpeed_;
  delete layout_;
  delete updater_;
};

/* GUI updater implementation */

GUIUpdater::GUIUpdater(QObject *parent) : QObject(parent) {
  server_ = new Server(this);
}

void GUIUpdater::RunServer() { QtConcurrent::run(server_, &Server::Run); }

Server * GUIUpdater::GetServer() { return server_;}

void GUIUpdater::IncrementProcessedPics(const QString &num) {
  emit RequestChangeLabel(num);
}

void GUIUpdater::UpdateProcessingPics(const QString &str) {
  emit RequestChangeProcIp(str);
}

void GUIUpdater::UpdateAveragePicProcSpeed(const QString &str) {
  emit RequestUpdtAvSpeed(str);
}

void GUIUpdater::UpdateAverageRecSpeed(const QString &str) {
  emit RequestUpdtRecSpeed(str);
}

void GUIUpdater::UpdateAverageSendSpeed(const QString &str) {
  emit RequestUpdtSendSpeed(str);
}

GUIUpdater::~GUIUpdater() {
  if (server_) delete server_;
}
