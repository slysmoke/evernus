/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <boost/exception/diagnostic_information.hpp>

#include <QCommandLineParser>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QApplication>
#include <QLocalSocket>
#include <QLocalServer>
#include <QMessageBox>
#include <QQmlEngine>
#include <QSettings>
#include <QSysInfo>
#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QUrl>

#ifdef Q_OS_OSX
#   include "QMacPasteboardMimeUnicodeText.h"
#endif

#include "MarketLogExternalOrderImporterThread.h"
#include "IndustryManufacturingSetupController.h"
#include "StandardExceptionQtWrapperException.h"
#include "MarketLogExternalOrderImporter.h"
#include "ProxyWebExternalOrderImporter.h"
#include "MarketOrderFilterProxyModel.h"
#include "ExternalOrderImporterNames.h"
#include "IndustryManufacturingSetup.h"
#include "MarketAnalysisDataFetcher.h"
#include "ContractFilterProxyModel.h"
#include "ChainableFileLogger.h"
#include "ExternalOrderModel.h"
#include "EvernusApplication.h"
#include "CommandLineOptions.h"
#include "EveDatabaseUpdater.h"
#include "UpdaterSettings.h"
#include "ImportSettings.h"
#include "BezierCurve.h"
#include "MainWindow.h"
#include "VolumeType.h"
#include "Version.h"
#include "Defines.h"
#include "Citadel.h"

#if EVERNUS_CREATE_DUMPS
#   include <iostream>

#   include "DumpUploader.h"

#   ifdef Q_OS_LINUX
#       include <client/linux/handler/exception_handler.h>
#   elif defined(Q_OS_OSX)
#       include <client/mac/handler/exception_handler.h>
#   else
#       include <client/windows/handler/exception_handler.h>
#   endif
#endif

#define STR_VALUE(s) #s
#define EVERNUS_TEXT(s) STR_VALUE(s)

#if defined(EVERNUS_CLIENT_ID) && defined(EVERNUS_CLIENT_SECRET)
#   define EVERNUS_CLIENT_ID_TEXT EVERNUS_TEXT(EVERNUS_CLIENT_ID)
#   define EVERNUS_CLIENT_SECRET_TEXT EVERNUS_TEXT(EVERNUS_CLIENT_SECRET)
#else
#   define EVERNUS_CLIENT_ID_TEXT ""
#   define EVERNUS_CLIENT_SECRET_TEXT ""
#endif

#if EVERNUS_CREATE_DUMPS
namespace
{
#   ifdef Q_OS_LINUX
    bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor, void *context, bool succeeded)
    {
        Q_UNUSED(context);

        std::cerr << "Created dump: " << descriptor.path() << std::endl;
        return succeeded;
    }
#   elif defined(Q_OS_OSX)
    bool dumpCallback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
    {
        Q_UNUSED(context);

        std::cerr << "Created dump: " << dump_dir << " " << minidump_id << std::endl;
        return succeeded;
    }
#   else
    bool dumpCallback(const wchar_t *dump_path,
                      const wchar_t *minidump_id,
                      void *context,
                      EXCEPTION_POINTERS *exinfo,
                      MDRawAssertionInfo *assertion,
                      bool succeeded)
   {
        Q_UNUSED(context);
        Q_UNUSED(exinfo);
        Q_UNUSED(assertion);

        std::wcerr << L"Created dump: " << dump_path << L'/' << minidump_id << std::endl;
        return succeeded;
   }
#   endif
}
#endif

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication::setApplicationName(QStringLiteral("Evernus"));
        QCoreApplication::setApplicationVersion(version::fullStr());
        QCoreApplication::setOrganizationDomain(QStringLiteral("evernus.anver.ee"));
        QCoreApplication::setOrganizationName(QStringLiteral("evernus.com"));
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#if EVERNUS_CREATE_DUMPS
        const auto dumpPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/dump");
        QDir{}.mkpath(dumpPath);

#   ifdef Q_OS_LINUX
        google_breakpad::MinidumpDescriptor descriptor{
            dumpPath.toStdString()
        };
        google_breakpad::ExceptionHandler eh{
            descriptor,
            nullptr,
            dumpCallback,
            nullptr,
            true,
            -1
        };
#   elif defined(Q_OS_OSX)
        google_breakpad::ExceptionHandler eh{
            dumpPath.toStdString(),
            nullptr,
            dumpCallback,
            nullptr,
            true,
            nullptr
        };
#   else
        new google_breakpad::ExceptionHandler{
            dumpPath.toStdWString(),
            nullptr,
            dumpCallback,
            nullptr,
            google_breakpad::ExceptionHandler::HANDLER_ALL
        };
#   endif
#endif

        QStringList arguments;
        arguments.reserve(argc);

        for (auto i = 0; i < argc; ++i)
            arguments << QString::fromLocal8Bit(argv[i]);

        QCommandLineParser parser;
        parser.setApplicationDescription(QCoreApplication::translate("main", "Evernus EVE Online trade tool"));
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOptions({
            { Evernus::CommandLineOptions::forceVersionArg, QCoreApplication::translate("main", "Force specific version") },
            { Evernus::CommandLineOptions::noUpdateArg, QCoreApplication::translate("main", "Don't run internal updater") },
            { Evernus::CommandLineOptions::clientIdArg, QCoreApplication::translate("main", "SSO client id"), QStringLiteral("id"), EVERNUS_CLIENT_ID_TEXT },
            { Evernus::CommandLineOptions::clientSecretArg, QCoreApplication::translate("main", "SSO client secret"), QStringLiteral("secret"), EVERNUS_CLIENT_SECRET_TEXT },
            { Evernus::CommandLineOptions::maxLogFileSizeArg, QCoreApplication::translate("main", "Max. log file size"), QStringLiteral("size"), QStringLiteral("%1").arg(10 * 1014 * 1024) },
            { Evernus::CommandLineOptions::maxLogFilesArg, QCoreApplication::translate("main", "Max. log files"), QStringLiteral("n"), QStringLiteral("3") },
            { Evernus::CommandLineOptions::forceSDEUpdateArg, QCoreApplication::translate("main", "Force Eve database update") },
        });

        // NOTE: don't use process here or it will exit on additional args in OSX
        parser.parse(arguments);

        if (parser.isSet(QStringLiteral("version")))
            parser.showVersion();
        if (parser.isSet(QStringLiteral("help")))
            parser.showHelp();

        Evernus::ChainableFileLogger::initialize(parser.value(Evernus::CommandLineOptions::maxLogFileSizeArg).toULongLong(),
                                                 parser.value(Evernus::CommandLineOptions::maxLogFilesArg).toUInt());

        qSetMessagePattern(QStringLiteral("[%{type}] %{time} %{threadid} %{message}"));

#ifdef Q_OS_WIN
        const auto serverName = QCoreApplication::applicationName() + ".socket";
#else
        const auto serverName = QDir::tempPath() + "/" + QCoreApplication::applicationName() + ".socket";
#endif

        qInfo() << QSysInfo::prettyProductName();
        qInfo() << QCoreApplication::applicationName() << QCoreApplication::applicationVersion();

        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(500))
        {
            qDebug() << "Connected to" << socket.fullServerName();

            QApplication tempApp{argc, argv};
            QMessageBox::information(nullptr, QCoreApplication::translate("main", "Already running"), QCoreApplication::translate("main",
                "Evernus seems to be already running. If this is not the case, please remove '%1'.").arg(socket.fullServerName()));

            return 0;
        }

        QLocalServer server;
        server.connect(&server, &QLocalServer::newConnection, &QLocalServer::nextPendingConnection);
        if (!server.listen(serverName))
        {
            qDebug() << "Local server listen failed:" << server.errorString();
#ifndef Q_OS_WIN
            if (server.serverError() == QAbstractSocket::AddressInUseError)
            {
                QApplication tempApp{argc, argv};
                const auto ret = QMessageBox::question(nullptr, QCoreApplication::translate("main", "Already running"), QCoreApplication::translate("main",
                    "Evernus probably didn't close cleanly the last time. Do you want to try to perform a cleanup?"));
                if (ret == QMessageBox::Yes)
                {
                    qDebug() << "Cleanup attempt.";
                    if (!QFile::remove(serverName))
                    {
                        qDebug() << "Cleanup failed.";
                        QMessageBox::critical(nullptr,
                                              QCoreApplication::translate("main", "Error"),
                                              QCoreApplication::translate("main", "Couldn't remove '%1'!").arg(serverName));
                        return 1;
                    }
                    else
                    {
                        server.listen(serverName);
                    }
                }
            }
            else
            {
                qDebug() << "Running anyway...";
            }
#endif
        }

        qRegisterMetaType<Evernus::MarketLogExternalOrderImporterThread::ExternalOrderList>("ExternalOrderList");
        qRegisterMetaType<Evernus::EveType::IdType>("EveType::IdType");
        qRegisterMetaType<Evernus::Character::IdType>("Character::IdType");
        qRegisterMetaType<Evernus::MarketOrder::IdType>("MarketOrder::IdType");
        qRegisterMetaType<Evernus::MarketOrderFilterProxyModel::StatusFilters>("MarketOrderFilterProxyModel::StatusFilters");
        qRegisterMetaType<Evernus::MarketOrderFilterProxyModel::StatusFilters>("StatusFilters");
        qRegisterMetaType<Evernus::MarketOrderFilterProxyModel::PriceStatusFilters>("MarketOrderFilterProxyModel::PriceStatusFilters");
        qRegisterMetaType<Evernus::MarketOrderFilterProxyModel::PriceStatusFilters>("PriceStatusFilters");
        qRegisterMetaType<std::vector<Evernus::ExternalOrder>>("std::vector<ExternalOrder>");
        qRegisterMetaType<Evernus::TypeLocationPairs>("TypeLocationPairs");
        qRegisterMetaType<Evernus::ExternalOrderModel::DeviationSourceType>("ExternalOrderModel::DeviationSourceType");
        qRegisterMetaType<Evernus::ExternalOrderModel::DeviationSourceType>("DeviationSourceType");
        qRegisterMetaType<Evernus::ContractFilterProxyModel::StatusFilters>("ContractFilterProxyModel::StatusFilters");
        qRegisterMetaType<Evernus::MarketAnalysisDataFetcher::OrderResultType>("MarketAnalysisDataFetcher::OrderResultType");
        qRegisterMetaType<Evernus::MarketAnalysisDataFetcher::OrderResultType>("OrderResultType");
        qRegisterMetaType<Evernus::MarketAnalysisDataFetcher::HistoryResultType>("MarketAnalysisDataFetcher::HistoryResultType");
        qRegisterMetaType<Evernus::MarketAnalysisDataFetcher::HistoryResultType>("HistoryResultType");
        qRegisterMetaType<Evernus::VolumeType>("VolumeType");
        qRegisterMetaType<Evernus::IndustryManufacturingSetup::InventorySource>("IndustryManufacturingSetup::InventorySource");
        qRegisterMetaType<Evernus::Citadel::IdType>("Citadel::IdType");

        // Eve database must be fetched before the main application starts
        if (Evernus::EveDatabaseUpdater::performUpdate(argc, argv, parser.isSet(Evernus::CommandLineOptions::forceSDEUpdateArg)) == Evernus::EveDatabaseUpdater::Status::Error)
            return 1;

        Evernus::EvernusApplication app{argc,
                                        argv,
                                        parser.value(Evernus::CommandLineOptions::clientIdArg),
                                        parser.value(Evernus::CommandLineOptions::clientSecretArg),
                                        parser.value(Evernus::CommandLineOptions::forceVersionArg),
                                        parser.isSet(Evernus::CommandLineOptions::noUpdateArg)};

#if EVERNUS_CREATE_DUMPS
        // hopefully we'll reach this point
        Evernus::DumpUploader uploader{dumpPath};
        uploader.run();
#endif

#ifdef Q_OS_OSX
        new QMacPasteboardMimeUnicodeText;
#endif

        auto webImporter = std::make_unique<Evernus::ProxyWebExternalOrderImporter>(app.getDataProvider(),
                                                                                    app.getESIInterfaceManager());
        auto webImporterPtr = webImporter.get();

        app.registerImporter(Evernus::ExternalOrderImporterNames::webImporter, std::move(webImporter));
        app.registerImporter(Evernus::ExternalOrderImporterNames::logImporter,
                             std::make_unique<Evernus::MarketLogExternalOrderImporter>());

        const auto evernusQmlUri = "com.evernus.qmlcomponents";

        qmlRegisterType<Evernus::BezierCurve>(evernusQmlUri, 2, 6, "BezierCurve");
        qmlRegisterUncreatableType<Evernus::IndustryManufacturingSetupController>(
            evernusQmlUri,
            2,
            6,
            "IndustryManufacturingSetupController",
            QStringLiteral("Type reserved.")
        );

        try
        {
            Evernus::MainWindow mainWnd{app,
                                        app.getMarketOrderProvider(),
                                        app.getCorpMarketOrderProvider(),
                                        app.getAssetProvider(),
                                        app.getCorpAssetProvider(),
                                        app.getContractProvider(),
                                        app.getCorpContractProvider(),
                                        app.getDataProvider(),
                                        app,
                                        app,
                                        app,
                                        app.getESIInterfaceManager(),
                                        app};

            QObject::connect(&mainWnd, &Evernus::MainWindow::refreshCharacters,
                             &app, &Evernus::EvernusApplication::refreshCharacters);
            QObject::connect(&mainWnd, &Evernus::MainWindow::refreshCitadels,
                             &app, &Evernus::EvernusApplication::refreshCitadels);
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacter,
                             &app, [&app](auto id) { app.refreshCharacter(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterAssets,
                             &app, [&app](auto id) { app.refreshCharacterAssets(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterContracts,
                             &app, [&app](auto id) { app.refreshCharacterContracts(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterWalletJournal,
                             &app, [&app](auto id) { app.refreshCharacterWalletJournal(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterWalletTransactions,
                             &app, [&app](auto id) { app.refreshCharacterWalletTransactions(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterMarketOrdersFromAPI,
                             &app, [&app](auto id) { app.refreshCharacterMarketOrdersFromAPI(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterMarketOrdersFromLogs,
                             &app, [&app](auto id) { app.refreshCharacterMarketOrdersFromLogs(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCharacterMiningLedger,
                             &app, [&app](auto id) { app.refreshCharacterMiningLedger(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpAssets,
                             &app, [&app](auto id) { app.refreshCorpAssets(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpContracts,
                             &app, [&app](auto id) { app.refreshCorpContracts(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpWalletJournal,
                             &app, [&app](auto id) { app.refreshCorpWalletJournal(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpWalletTransactions,
                             &app, [&app](auto id) { app.refreshCorpWalletTransactions(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpMarketOrdersFromAPI,
                             &app, [&app](auto id) { app.refreshCorpMarketOrdersFromAPI(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importCorpMarketOrdersFromLogs,
                             &app, [&app](auto id) { app.refreshCorpMarketOrdersFromLogs(id); });
            QObject::connect(&mainWnd, &Evernus::MainWindow::importExternalOrdersFromWeb,
                             &app, &Evernus::EvernusApplication::refreshExternalOrdersFromWeb);
            QObject::connect(&mainWnd, &Evernus::MainWindow::importExternalOrdersFromFile,
                             &app, &Evernus::EvernusApplication::refreshExternalOrdersFromFile);
            QObject::connect(&mainWnd, &Evernus::MainWindow::preferencesChanged,
                             &app, &Evernus::EvernusApplication::handleNewPreferences);
            QObject::connect(&mainWnd, &Evernus::MainWindow::importFromMentat,
                             &app, &Evernus::EvernusApplication::importFromMentat);
            QObject::connect(&mainWnd, &Evernus::MainWindow::syncLMeve,
                             &app, &Evernus::EvernusApplication::syncLMeve);
            QObject::connect(&mainWnd, &Evernus::MainWindow::openMarketInEve,
                             &app, &Evernus::EvernusApplication::showInEve);
            QObject::connect(&mainWnd, &Evernus::MainWindow::setDestinationInEve,
                             &app, &Evernus::EvernusApplication::setDestinationInEve);
            QObject::connect(&mainWnd, &Evernus::MainWindow::updateExternalOrders,
                             &app, &Evernus::EvernusApplication::updateExternalOrdersAndAssetValue);
            QObject::connect(&mainWnd, &Evernus::MainWindow::clearCorpWalletData,
                             &app, &Evernus::EvernusApplication::clearCorpWalletData);
            QObject::connect(&mainWnd, &Evernus::MainWindow::clearRefreshTokens,
                             &app, &Evernus::EvernusApplication::clearRefreshTokens);
            QObject::connect(&mainWnd, &Evernus::MainWindow::makeValueSnapshots,
                             &app, &Evernus::EvernusApplication::makeValueSnapshots);
            QObject::connect(&mainWnd, &Evernus::MainWindow::citadelsEdited,
                             &app, &Evernus::EvernusApplication::clearCitadelCache);
            QObject::connect(&mainWnd, &Evernus::MainWindow::gotSSOAuthorizationCode,
                             &app, &Evernus::EvernusApplication::processSSOAuthorizationCode);
            QObject::connect(&mainWnd, &Evernus::MainWindow::ssoAuthCanceled,
                             &app, &Evernus::EvernusApplication::cancelSsoAuth);
            QObject::connect(&mainWnd, &Evernus::MainWindow::authorizedCharacter,
                             &app, &Evernus::EvernusApplication::processNewCharacter);
            QObject::connect(&app, QOverload<uint, const QString &>::of(&Evernus::EvernusApplication::taskStarted),
                             &mainWnd, &Evernus::MainWindow::addNewTaskInfo);
            QObject::connect(&app, QOverload<uint, uint, const QString &>::of(&Evernus::EvernusApplication::taskStarted),
                             &mainWnd, &Evernus::MainWindow::newSubTaskInfoAdded);
            QObject::connect(&app, &Evernus::EvernusApplication::taskInfoChanged,
                             &mainWnd, &Evernus::MainWindow::taskInfoChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::taskEnded,
                             &mainWnd, &Evernus::MainWindow::taskEnded);
            QObject::connect(&app, &Evernus::EvernusApplication::citadelsChanged,
                             &mainWnd, &Evernus::MainWindow::citadelsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::charactersChanged,
                             &mainWnd, &Evernus::MainWindow::updateCharacters);
            QObject::connect(&app, &Evernus::EvernusApplication::characterAssetsChanged,
                             &mainWnd, &Evernus::MainWindow::characterAssetsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::externalOrdersChanged,
                             &mainWnd, &Evernus::MainWindow::externalOrdersChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::externalOrdersChangedWithMarketOrders,
                             &mainWnd, &Evernus::MainWindow::externalOrdersChangedWithMarketOrders);
            QObject::connect(&app, &Evernus::EvernusApplication::characterWalletJournalChanged,
                             &mainWnd, &Evernus::MainWindow::characterWalletJournalChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::characterWalletTransactionsChanged,
                             &mainWnd, &Evernus::MainWindow::characterWalletTransactionsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::characterMarketOrdersChanged,
                             &mainWnd, &Evernus::MainWindow::characterMarketOrdersChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::characterContractsChanged,
                             &mainWnd, &Evernus::MainWindow::characterContractsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::characterMiningLedgerChanged,
                             &mainWnd, &Evernus::MainWindow::characterMiningLedgerChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::corpAssetsChanged,
                             &mainWnd, &Evernus::MainWindow::corpAssetsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::corpWalletJournalChanged,
                             &mainWnd, &Evernus::MainWindow::corpWalletJournalChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::corpWalletTransactionsChanged,
                             &mainWnd, &Evernus::MainWindow::corpWalletTransactionsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::corpMarketOrdersChanged,
                             &mainWnd, &Evernus::MainWindow::corpMarketOrdersChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::corpContractsChanged,
                             &mainWnd, &Evernus::MainWindow::corpContractsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::itemCostsChanged,
                             &mainWnd, &Evernus::MainWindow::itemCostsChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::itemVolumeChanged,
                             &mainWnd, &Evernus::MainWindow::itemVolumeChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::lMeveTasksChanged,
                             &mainWnd, &Evernus::MainWindow::lMeveTasksChanged);
            QObject::connect(&app, &Evernus::EvernusApplication::charactersChanged,
                             &mainWnd, &Evernus::MainWindow::updateIskData);
            QObject::connect(&app, &Evernus::EvernusApplication::openMarginTool,
                             &mainWnd, &Evernus::MainWindow::showMarginTool);
            QObject::connect(&app, &Evernus::EvernusApplication::snapshotsTaken,
                             &mainWnd, &Evernus::MainWindow::snapshotsTaken);
            QObject::connect(&app, &Evernus::EvernusApplication::ssoError,
                             &mainWnd, &Evernus::MainWindow::showSSOError);
            QObject::connect(&app, &Evernus::EvernusApplication::ssoAuthRequested,
                             &mainWnd, &Evernus::MainWindow::requestSSOAuth);
            QObject::connect(&mainWnd, &Evernus::MainWindow::preferencesChanged,
                             webImporterPtr, &Evernus::ProxyWebExternalOrderImporter::handleNewPreferences);
            mainWnd.showAsSaved();

            QSettings settings;
            if (!settings.value(Evernus::UpdaterSettings::askedToShowReleaseNotesKey, false).toBool())
            {
                const auto ret = QMessageBox::question(
                    &mainWnd,
                    QCoreApplication::translate("main", "New version"),
                    QCoreApplication::translate("main", "Would you like to see what's new in this version?")
                );

                if (ret == QMessageBox::Yes)
                    QDesktopServices::openUrl(QUrl{QStringLiteral("http://evernus.anver.ee/")});

                settings.setValue(Evernus::UpdaterSettings::askedToShowReleaseNotesKey, true);
            }

            if (settings.value(Evernus::ImportSettings::citadelAccessCacheWarningKey, Evernus::ImportSettings::citadelAccessCacheWarningDefault).toBool())
            {
                QMessageBox::information(
                    &mainWnd,
                    QCoreApplication::translate("main", "Citadel data import"),
                    QCoreApplication::translate(
                        "main",
                        "EVE doesn't provide any information about your access rights to citadels. This means, in order to fetch data from citadels, Evernus needs to ask "
                        "EVE for data for all possible citadels in given regions. This usually takes a lot of time. After doing the first citadel import, the next imports "
                        "will be much, much faster. From time to time, Evernus needs to refresh its citadel data and ask for all of it again - you can change the refresh period "
                        "in the preferences."
                    )
                );

                settings.setValue(Evernus::ImportSettings::citadelAccessCacheWarningKey, false);
            }

            if (!app.getCharacterRepository().hasCharacters())
                QMetaObject::invokeMethod(&mainWnd, "showCharacterManagement", Qt::QueuedConnection);

            return app.exec();
        }
        catch (const Evernus::StandardExceptionQtWrapperException &e)
        {
            try
            {
                e.rethrow();
            }
            catch (...)
            {
                const auto info = boost::current_exception_diagnostic_information();

                qCritical() << info.c_str();
                QMessageBox::critical(nullptr, QCoreApplication::translate("main", "Error"), QString::fromStdString(info));
                return 1;
            }
        }
        catch (...)
        {
            const auto info = boost::current_exception_diagnostic_information();

            qCritical() << info.c_str();
            QMessageBox::critical(nullptr, QCoreApplication::translate("main", "Error"), QString::fromStdString(info));
            return 1;
        }
    }
    catch (...)
    {
        const auto info = boost::current_exception_diagnostic_information();

        qCritical() << info.c_str();

        QApplication tempApp{argc, argv};
        QMessageBox::critical(nullptr, QCoreApplication::translate("main", "Initialization error"), QString::fromStdString(info));
        return 1;
    }

    return 0;
}
