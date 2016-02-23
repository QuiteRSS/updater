/* =============================================================================
* QuiteRSS is a open-source cross-platform RSS/Atom news feeds reader
* Copyright (C) 2012-2015 QuiteRSS Team <quiterssteam@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* =========================================================================== */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QNetworkProxy>
#include <QNetworkReply>
#include <windows.h>
#include <w32api.h>
#include <tlhelp32.h>

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  explicit MainWindow(QString appDirPath, QWidget *parent = 0);
  ~MainWindow();

  static QString dirPath();

public slots:
  void receiveMessage(const QString&);

private:
  void findFiles(const QDir& dir);
  void finishUpdate(QString str);
  QString copyToQString(WCHAR array[MAX_PATH]);

  static QString appDirPath_;
  QLabel *statusLabel_;
  QProgressBar *progressBar_;
  QTimer *progressTimer_;
  QVBoxLayout *mainLayout_;

  QHBoxLayout *applyButtonLayout_;
  QPushButton *applyButton_;
  QPushButton *cancelButton_;

  QTimer *isProcessRunTimer_;
  QStringList filesList_;
  QStringList md5List_;
  QStringList filesListT_;
  QStringList filesListS_;
  QStringList md5ListS_;
  QStringList filesListR_;
  int cntFiles_;

  QNetworkAccessManager manager_;
  QNetworkReply *reply_;

  QProcess *sevenzaProcess_;

private slots:
  void launchRequest();
  void isProcessRun();
  void finishLoadFilesList();
  void finishLoadFiles();
  void startLoadFile();
  void slotMoveWindows();
  void continueUpgrade();
  void cancelUpgrade();
  void extractFiles();
  void finishExtract(int exitCode, QProcess::ExitStatus exitStatus);
  void errorExtract(QProcess::ProcessError error);

signals:
  void signalMoveWindows();

};

#endif // MAINWINDOW_H
