#include <QtGui/QApplication>
#include <qtsingleapplication.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QtSingleApplication app(argc, argv);

  QString message = app.arguments().value(1);
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
    if (appDirPath != QString(QDir::tempPath() + "/QuiteRSSUpdater")) {
      QStringList fileDll;
      fileDll << "libgcc_s_dw2-1.dll" << "mingwm10.dll"<< "QtCore4.dll"
              << "QtGui4.dll" << "QtNetwork4.dll" << "Updater.exe";

      QDir(QDir::tempPath()).mkdir("QuiteRSSUpdater");
      foreach (QString file, fileDll) {
        QFile::remove(QDir::tempPath() + "/QuiteRSSUpdater/" + file);
        QFile::copy(appDirPath + "/" + file,
                    QDir::tempPath() + "/QuiteRSSUpdater/" + file);
      }
      QString quiterssFile = QDir::tempPath() + "/QuiteRSSUpdater/" + "/Updater.exe";
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
//  app.setQuitOnLastWindowClosed(false);

  MainWindow window(message);

  app.setActivationWindow(&window, true);
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &window, SLOT(receiveMessage(const QString&)));

  window.show();
  return app.exec();
}
