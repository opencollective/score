#include <Library/LibrarySettings.hpp>

#include <score/command/Command.hpp>
#include <score/command/Dispatchers/ICommandDispatcher.hpp>
#include <score/command/SettingsCommand.hpp>
#include <score/tools/Bind.hpp>

#include <QApplication>
#include <QDir>
#include <QLineEdit>
#include <QFormLayout>
#include <QStandardPaths>
#include <QStyle>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <zipdownloader.hpp>
#include <wobjectimpl.h>

W_OBJECT_IMPL(Library::Settings::View)
W_OBJECT_IMPL(Library::Settings::Model)

namespace Library::Settings
{

namespace Parameters
{
SETTINGS_PARAMETER_IMPL(Path){QStringLiteral("Library/Path"), [] {
                                auto paths = QStandardPaths::standardLocations(
                                    QStandardPaths::DocumentsLocation);
                                return paths[0] + "/ossia score library";
                              }()};

static auto list()
{
  return std::tie(Path);
}
}

static void initSystemLibrary(QDir& lib_folder)
{
  lib_folder.mkpath(".");
  lib_folder.mkpath("./System");
  lib_folder.mkpath("./Scores");
  lib_folder.mkpath("./Medias");
  lib_folder.mkpath("./Presets");
  lib_folder.mkpath("./Devices");
  lib_folder.mkpath("./Cues");
  lib_folder.mkpath("./Addons");
  lib_folder.mkpath("./Shaders");
}

Model::Model(QSettings& set, const score::ApplicationContext& ctx)
{
  score::setupDefaultSettings(set, Parameters::list(), *this);
  auto lib_folder = getPath();
  if (QDir dir{lib_folder}; !dir.exists())
  {
    initSystemLibrary(dir);

    auto dl = QMessageBox::question(qApp->activeWindow(), tr("Download the user library ?"), tr("The user library has not been found. \n"
                                                                                      "Do you want to download it from the internet ? \n\n"
                                                                                      "Note: you can always download it later from : \n"
                                                                                      "https://github.com/OSSIA/score-user-library"));
    if(dl)
    {
      zdl::download_and_extract(
            QUrl{"https://github.com/OSSIA/score-user-library/archive/master.zip"},
            dir.absolutePath(), [] (const auto&) { }, [] { });
    }
  }
}

SCORE_SETTINGS_PARAMETER_CPP(QString, Model, Path)

Presenter::Presenter(Model& m, View& v, QObject* parent)
    : score::GlobalSettingsPresenter{m, v, parent}
{
  SETTINGS_PRESENTER(Path);
}

QString Presenter::settingsName()
{
  return tr("Library");
}

QIcon Presenter::settingsIcon()
{
  return QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon);
}

View::View() : m_widg{new QWidget}
{
  auto lay = new QFormLayout;
  m_widg->setLayout(lay);

  SETTINGS_UI_PATH_SETUP("Default Path", Path);
}

SETTINGS_UI_PATH_IMPL(Path)
QWidget* View::getWidget()
{
  return m_widg;
}
}
