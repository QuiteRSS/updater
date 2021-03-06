/* =============================================================================
* QuiteRSS is a open-source cross-platform RSS/Atom news feeds reader
* Copyright (C) 2012-2020 QuiteRSS Team <quiterssteam@gmail.com>
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
#include <QApplication>
#include <qtsingleapplication.h>

#include "mainwindow.h"
#include "logfile.h"

int main(int argc, char *argv[])
{
  QtSingleApplication app(argc, argv);

#ifdef HAVE_QT5
  qInstallMessageHandler(LogFile::msgHandler);
#else
  qInstallMsgHandler(LogFile::msgHandler);
#endif
  qWarning() << "Start application!";
  qDebug() << "isRunning: " << app.isRunning();

  QString message = app.arguments().value(1);
  qDebug() << "message: " << message;
  if (app.isRunning()) {
    if (1 == argc) {
      app.sendMessage("--show");
    }
    else {
      for (int i = 2; i < argc; ++i)
        message += '\n' + app.arguments().value(i);
      app.sendMessage(message);
    }
    return 0;
  } else {
    QString appDirPath = QCoreApplication::applicationDirPath();
    qDebug() << "Application dir path: " << appDirPath;
    if (!appDirPath.contains("QuiteRSSUpdater")) {
      qDebug() << "Temp dir path: " << QDir::tempPath();

      QStringList fileDll;
#ifdef HAVE_QT5
      fileDll
  #if QT_VERSION < 0x050d00
              << "icudt53.dll" << "icuin53.dll" << "icuuc53.dll"
  #endif
  #if defined(__MINGW32__)
              << "libgcc_s_dw2-1.dll" << "libstdc++-6.dll" << "libwinpthread-1.dll"
  #endif
              << "Qt5Core.dll" << "Qt5Gui.dll" << "Qt5Widgets.dll"
              << "Qt5Network.dll" << "Updater.exe" << "7za.exe"
              << "platforms/qwindows.dll" << "styles/qwindowsvistastyle.dll"
              << "msvcp140.dll" << "vcruntime140.dll";
#else
      fileDll << "libgcc_s_dw2-1.dll" << "mingwm10.dll"<< "QtCore4.dll"
              << "QtGui4.dll" << "QtNetwork4.dll" << "Updater.exe"
              << "7za.exe";
#endif

      QDir(QDir::tempPath()).mkdir("QuiteRSSUpdater");
#ifdef HAVE_QT5
      QDir(QDir::tempPath()).mkdir("QuiteRSSUpdater/platforms");
      QDir(QDir::tempPath()).mkdir("QuiteRSSUpdater/styles");
#endif
      foreach (QString file, fileDll) {
        QFile::remove(QDir::tempPath() + "/QuiteRSSUpdater/" + file);
        bool okCopy = QFile::copy(appDirPath + "/" + file,
                                  QDir::tempPath() + "/QuiteRSSUpdater/" + file);
        if (!okCopy)
          qCritical() << "Error copying file: " << file;
      }
      QString quiterssFile = QDir::tempPath() + "/QuiteRSSUpdater/Updater.exe";
      appDirPath = "\"" + appDirPath + "\"";
      (quintptr)ShellExecute(
            0, 0,
            (wchar_t *)quiterssFile.utf16(),
            (wchar_t *)appDirPath.utf16(),
            0, SW_SHOWNORMAL);
      return 0;
    }
  }

  app.setApplicationName("QuiteRss Updater");
  app.setOrganizationName("QuiteRss");
  app.setWindowIcon(QIcon(":/images/images/update.png"));

  MainWindow window(message);

  app.setActivationWindow(&window, true);
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &window, SLOT(receiveMessage(const QString&)));

  window.show();
  return app.exec();
}
