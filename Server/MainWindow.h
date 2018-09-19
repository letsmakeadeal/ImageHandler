#pragma once

#include <QDebug>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <QFuture>
#include "Server.h"

class Server;
class GUIUpdater;

namespace Ui {
class MainWindow;
}

/* @brief Class implemented QT GUI */

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();
 public slots:
  void IncrementProcessedPics(const QString &string);
  void UpdateProcIpAdresses(const QString &string);
  void UpdateAveragePicProcSpeed(const QString& string);
  void UpdateAverageRecSpeed(const QString& string);
  void UpdateAverageSendSpeed(const QString& string);
 private:
  void RunServer();

 private:
  GUIUpdater *updater_;
  int number_of_done_pics_;
  QLabel *port_messages_;
  QLabel *nDonePics_;
  QLabel *sourcesLabel_;
  QLabel *avSpeedPicProcess_;
  QLabel *sources_;
  QLabel *netAvReceiveSpeed_;
  QLabel *netAvSendSpeed_;
  QVBoxLayout *layout_;
};

/* @brief Class using for connection beetween GUI and Server*/

class GUIUpdater : public QObject {
  Q_OBJECT
 public:
  GUIUpdater(QObject *parent = 0);
  void IncrementProcessedPics(const QString &str);
  void UpdateProcessingPics(const QString &str);
  void UpdateAveragePicProcSpeed(const QString &str);
  void UpdateAverageRecSpeed(const QString &str);
  void UpdateAverageSendSpeed(const QString &str);
  void RunServer();
  Server * GetServer();
  signals:
  void RequestChangeLabel(const QString &str);
  void RequestChangeProcIp(const QString &str);
  void RequestUpdtAvSpeed(const QString &str);
  void RequestUpdtSendSpeed(const QString &str);
  void RequestUpdtRecSpeed(const QString &str);
public:
  ~GUIUpdater();
 private:
  Server *server_;
};