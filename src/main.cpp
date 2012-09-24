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
  }

  app.setApplicationName("QuiteRss Updater");
  app.setOrganizationName("QuiteRss");
  app.setWindowIcon(QIcon(":/images/images/update.png"));
//  app.setQuitOnLastWindowClosed(false);

  MainWindow window;

  app.setActivationWindow(&window, true);
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &window, SLOT(receiveMessage(const QString&)));


  window.show();

  return app.exec();
}
