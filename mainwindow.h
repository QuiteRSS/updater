#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QNetworkProxy>
#include <QNetworkReply>

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void receiveMessage(const QString&);

private:
  void findFiles(const QDir& dir);
  void finishUpdate(QString str);

  QLabel *statusLabel_;
  QProgressBar *progressBar_;
  QTimer *progressTimer_;
  QVBoxLayout *mainLayout_;

  QHBoxLayout *applyButtonLayout_;
  QPushButton *applyButton_;
  QPushButton *cancelButton_;

  QTimer *isRuningAppTimer_;
  QStringList filesList_;
  QStringList md5List_;
  QStringList filesListT_;
  QStringList filesListS_;
  QStringList md5ListS_;

  QNetworkAccessManager manager_;
  QNetworkReply *reply_;

private slots:
  void launchRequest();
  void isRuningApp();
  void finishLoadFilesList();
  void finishLoadFiles();
  void startLoadFile();
  void slotMoveWindows();
  void continueUpgrade();
  void cancelUpgrade();
  void extractFiles();

signals:
  void signalMoveWindows();

};

#endif // MAINWINDOW_H
