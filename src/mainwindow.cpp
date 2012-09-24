#include "mainwindow.h"

/*! \brief Обработка сообщений полученных из запущщеной копии программы *******/
void MainWindow::receiveMessage(const QString& message)
{
  qDebug() << QString("Received message: '%1'").arg(message);
  if (!message.isEmpty()){
    QStringList params = message.split('\n');
    foreach (QString param, params) {
      if (param == "--show") {
        showNormal();
        activateWindow();
      }
      if (param == "--exit") {
        hide();
        qApp->quit();
      }
    }
  }
}

MainWindow::MainWindow(QWidget *parent) :
  QWidget(parent)
{
  setWindowTitle("QuiteRSS Updater");
  setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                 Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint |
                 Qt::WindowMinimizeButtonHint);

  statusLabel_ = new QLabel(tr("Checking for updates..."), this);
  statusLabel_->setStyleSheet("background: white;"
                              "padding: 10;");

  progressBar_ = new QProgressBar(this);
  progressBar_->setFixedHeight(16);
  progressBar_->setMaximum(0);
  progressBar_->setTextVisible(false);

  mainLayout_ = new QVBoxLayout();
  mainLayout_->setMargin(0);
  mainLayout_->setSpacing(0);

  mainLayout_->addWidget(statusLabel_);
  mainLayout_->addWidget(progressBar_);

  setLayout(mainLayout_);

  resize(230, 20);

  QSettings *settings = new QSettings(
        QCoreApplication::applicationDirPath() + "/QuiteRSS.ini",
        QSettings::IniFormat);

  QNetworkProxy networkProxy;
  networkProxy.setType(static_cast<QNetworkProxy::ProxyType>(
                         settings->value("networkProxy/type", QNetworkProxy::DefaultProxy).toInt()));
  networkProxy.setHostName(settings->value("networkProxy/hostName", "").toString());
  networkProxy.setPort(    settings->value("networkProxy/port",     "").toUInt());
  networkProxy.setUser(    settings->value("networkProxy/user",     "").toString());
  networkProxy.setPassword(settings->value("networkProxy/password", "").toString());

  if (QNetworkProxy::DefaultProxy == networkProxy.type())
    QNetworkProxyFactory::setUseSystemConfiguration(true);
  else {
    QNetworkProxy::setApplicationProxy(networkProxy);
  }

  isProcessRunTimer_ = new QTimer(this);
  connect(isProcessRunTimer_, SIGNAL(timeout()),
          this, SLOT(isProcessRun()));

  connect(this, SIGNAL(signalMoveWindows()), SLOT(slotMoveWindows()),
          Qt::QueuedConnection);

  QTimer::singleShot(50, this, SLOT(launchRequest()));

  emit signalMoveWindows();
}

MainWindow::~MainWindow()
{
}

void MainWindow::launchRequest()
{
  findFiles(QDir(QCoreApplication::applicationDirPath()));

  reply_ = manager_.get(QNetworkRequest(QUrl("http://file.quite-rss.googlecode.com/hg/file_list.md5")));
  connect(reply_, SIGNAL(finished()), this, SLOT(finishLoadFilesList()));
}

//! Загрузка списка файлов с контрольными суммами с сервера
void MainWindow::finishLoadFilesList()
{
  if (reply_->error() == QNetworkReply::NoError) {
    QStringList listS(QString(QLatin1String(reply_->readAll())).split('\n'));

    foreach (QString str, listS) {
      str = str.simplified();

      QString fileStr = str.right(str.length() - str.indexOf(" *") - 2).replace("\\", "/");
      QString md5Str = str.left(str.indexOf(" *"));

      bool ok = false;
      for (int i = 0; i < filesList_.count(); i++) {
        if ((fileStr == filesList_.at(i)) && (md5Str == md5List_.at(i))) {
          ok = true;
          break;
        }
      }
      if (!ok) {
        filesListS_ << fileStr;
        md5ListS_ << md5Str;
      }
    }
    filesList_ = filesListS_;

    disconnect(reply_, SIGNAL(finished()), this, SLOT(finishLoadFilesList()));

    if (filesList_.count() > 1) {
      startLoadFile();
    } else {
      finishUpdate(tr("No new version available!"));
    }
  } else {
    finishUpdate(tr("Error checking updates!"));
  }
}

//! Запуск загрузки файла с сервера
void MainWindow::startLoadFile()
{
  if (filesList_.count() > 1) {
    QString fileStr(filesList_.takeFirst());
    QString urlStr = "http://file.quite-rss.googlecode.com/hg/windows/" + fileStr + ".7z";

    reply_ = manager_.get(QNetworkRequest(QUrl(urlStr)));
    connect(reply_, SIGNAL(finished()), this, SLOT(finishLoadFiles()));
    statusLabel_->setText(tr("Downloading files (%1)...").
                          arg(filesListS_.count() - filesList_.count()));
  } else {
    statusLabel_->setText(tr("Attention! QuiteRSS will close! \nContinue updating?"));
    progressBar_->hide();

    applyButtonLayout_ = new QHBoxLayout();
    applyButtonLayout_->setMargin(5);
    applyButtonLayout_->setAlignment(Qt::AlignHCenter);
    applyButton_ = new QPushButton(tr("Ok"), this);
    cancelButton_ = new QPushButton(tr("Cancel"), this);
    connect(applyButton_, SIGNAL(clicked()), SLOT(continueUpgrade()));
    connect(cancelButton_, SIGNAL(clicked()), SLOT(cancelUpgrade()));

    applyButtonLayout_->addWidget(applyButton_);
    applyButtonLayout_->addWidget(cancelButton_);
    mainLayout_->addLayout(applyButtonLayout_);

    showNormal();
    activateWindow();

    emit signalMoveWindows();
  }
}

//! Загрузка файлов с сервера
void MainWindow::finishLoadFiles()
{
  if (reply_->error() == QNetworkReply::NoError) {
    disconnect(reply_, SIGNAL(finished()), this, SLOT(finishLoadFiles()));

    QTemporaryFile file;
    file.setAutoRemove(false);
    if (file.open()) {
      file.write(reply_->readAll());
      file.close();
      filesListT_ << file.fileName();
    }

    startLoadFile();
  } else {
    finishUpdate(tr("Error checking updates!"));
  }
}

//! Проверка запущено ли обновляемое приложение
void MainWindow::isProcessRun()
{
  isProcessRunTimer_->stop();

  static bool exitOn = false;
  static int cnt = 0;

  HANDLE hSnap = NULL;
  PROCESSENTRY32 pe32 = {sizeof(pe32)};
  QList <int> ids;

  hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap!=NULL) {
    if (Process32First(hSnap, &pe32)) {
      QString filename = copyToQString(pe32.szExeFile);
      if (filename == "QuiteRSS.exe")
        ids.append(pe32.th32ProcessID);
      while (Process32Next(hSnap, &pe32)) {
        filename = copyToQString(pe32.szExeFile);
        if (filename == "QuiteRSS.exe")
          ids.append(pe32.th32ProcessID);
      }
    }
  }
  CloseHandle(hSnap);

  MODULEENTRY32 mpe32 = {sizeof(mpe32)};
  foreach (int id, ids) {
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, id);
    if (hSnap!=NULL) {
      if (Module32First(hSnap, &mpe32)) {
        QFileInfo file(copyToQString(mpe32.szExePath));
        if (file.path() == QCoreApplication::applicationDirPath()) {
          if (!exitOn) {
            exitOn = true;
            QString quiterssFile = QCoreApplication::applicationDirPath() + "/QuiteRSS.exe";
            (quintptr)ShellExecute(
                  0, 0,
                  (wchar_t *)quiterssFile.utf16(),
                  (wchar_t *)QString("--exit").utf16(),
                  0, SW_SHOWNORMAL);
          }
          CloseHandle(hSnap);
          if (cnt++ < 240)
            isProcessRunTimer_->start(500);
          else
            finishUpdate(tr("Error updating!"));
          return;
        }
      }
    }
    CloseHandle(hSnap);
  }
  extractFiles();
}

QString MainWindow::copyToQString(WCHAR array[MAX_PATH])
{
  QString string;
  int i = 0;

  while (array[i] != 0) {
    string[i] = array[i];
    i++;
  }
  return string;
}

//! Поиск всех файлов в папке с программой для заполнения списка
void MainWindow::findFiles(const QDir& dir)
{
  QApplication::processEvents();

  QStringList listFiles =
      dir.entryList(QDir::Files);

  foreach (QString file, listFiles) {
    QString str;
    QFile file_(dir.absoluteFilePath(file));
    if (file_.open(QIODevice::ReadOnly)) {
      str = QCryptographicHash::hash(file_.readAll(), QCryptographicHash::Md5).toHex();
      file_.close();
    }
    filesList_ << dir.absoluteFilePath(file).remove(QCoreApplication::applicationDirPath()+"/");
    md5List_ << str;
  }

  QStringList listDir = dir.entryList(QDir::Dirs);
  foreach (QString subdir, listDir) {
    if (subdir == "." || subdir == "..") {
      continue;
    }
    findFiles(QDir(dir.absoluteFilePath(subdir)));
  }
}

void MainWindow::finishUpdate(QString str)
{
  statusLabel_->setText(str);
  progressBar_->hide();

  QHBoxLayout *buttonLayout_ = new QHBoxLayout();
  buttonLayout_->setMargin(5);
  buttonLayout_->setAlignment(Qt::AlignRight);
  QPushButton *closeButton_ = new QPushButton(tr("&Close"), this);
  closeButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  closeButton_->setDefault(true);
  closeButton_->setFocus(Qt::OtherFocusReason);
  connect(closeButton_, SIGNAL(clicked()), SLOT(cancelUpgrade()));

  buttonLayout_->addStretch(1);
  buttonLayout_->addWidget(closeButton_);
  mainLayout_->addLayout(buttonLayout_);

  emit signalMoveWindows();
}

void MainWindow::slotMoveWindows()
{
  move(QApplication::desktop()->availableGeometry(0).width()-frameSize().width()-5,
       QApplication::desktop()->availableGeometry(0).height()-frameSize().height()-5);
}

void MainWindow::continueUpgrade()
{
  statusLabel_->setText(tr("Closing QuiteRSS..."));
  applyButtonLayout_->setMargin(0);
  applyButton_->hide();
  cancelButton_->hide();
  progressBar_->show();
  resize(230, 20);
  emit signalMoveWindows();

  isProcessRun();
}

void MainWindow::cancelUpgrade()
{
  foreach (QString file, filesListT_) {
    QFile::remove(file);
  }
  close();
}

//! Переименовать и распаковать файлы в папку с программой
void MainWindow::extractFiles()
{
  sevenzaProcess_ = new QProcess(this);
  connect(sevenzaProcess_, SIGNAL(finished(int,QProcess::ExitStatus)),
          this, SLOT(finishExtract(int,QProcess::ExitStatus)));
  connect(sevenzaProcess_, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(errorExtract(QProcess::ProcessError)));
  finishExtract(0, QProcess::NormalExit);
}

void MainWindow::finishExtract(int t, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::CrashExit) {
    finishUpdate(tr("Error extracting files!"));
    return;
  }
  if (filesListS_.count() > 1) {
    QString file = filesListS_.takeFirst();
    statusLabel_->setText(tr("Extract files (%1)...").
                          arg(filesListT_.count() - filesListS_.count() + 1));

    QString program = "7za.exe";
    QStringList arguments;
    QString path = QCoreApplication::applicationDirPath();

    if (file.lastIndexOf("/") > 0)
      path.append(QString("/%1").arg(file.left(file.lastIndexOf("/"))));
    arguments << "x" << "-r" << "-aoa"
              << filesListT_.at(filesListT_.count() - filesListS_.count())
              << QString("-o%1").arg(path);
    sevenzaProcess_->start(program, arguments);
  } else {
    statusLabel_->setText(tr("Update completed!"));
    progressBar_->hide();

    QString quiterssFile = QCoreApplication::applicationDirPath() + "/QuiteRSS.exe";
    (quintptr)ShellExecute(
          0, 0,
          (wchar_t *)quiterssFile.utf16(),
          (wchar_t *)QString("--show").utf16(),
          0, SW_SHOWNORMAL);
    cancelUpgrade();
  }
}

void MainWindow::errorExtract(QProcess::ProcessError)
{
  finishUpdate(tr("Error extracting files!"));
}
