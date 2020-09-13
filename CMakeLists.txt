cmake_minimum_required(VERSION 3.10)

project(Evernus)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "evernus")
set(MAJOR_VERSION "3")
set(MINOR_VERSION "9.4")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeHelpers")


option(EVERNUS_CREATE_DUMPS "create crash dumps using Google Breakpad" ON)

find_package(Boost REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Core 5.12.7 REQUIRED)
find_package(Qt5DataVisualization REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5NetworkAuth REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Threads REQUIRED)

if(EVERNUS_CREATE_DUMPS)
    find_package(Breakpad REQUIRED)
    add_definitions(-DEVERNUS_CREATE_DUMPS=1)
else()
    # because Defines.h will define to 1 if undefined
    add_definitions(-DEVERNUS_CREATE_DUMPS=0)
endif()


if(DEFINED EVERNUS_CLIENT_ID AND DEFINED EVERNUS_CLIENT_SECRET)
    add_definitions(-DEVERNUS_CLIENT_ID=${EVERNUS_CLIENT_ID} -DEVERNUS_CLIENT_SECRET=${EVERNUS_CLIENT_SECRET})
endif()

configure_file(Version.h.in Version.h)

if(APPLE)
    find_package(Qt5MacExtras REQUIRED)
    find_library(CarbonLibrary Carbon)
    mark_as_advanced(CarbonLibrary)

    configure_file("${CMAKE_SOURCE_DIR}/Info.plist.in" Info.plist)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -stdlib=libc++ -pthread")

    # From Qt 5.6 onwards SSL parts use native MacOS backend
    # https://forum.qt.io/topic/55853/openssl-and-mac-os-x/6
    # but LibQxt checks for QT_NO_OPENSSL and disables ssl in this case.
    # To workaround that we define QT_OPENSSL - see qconfig.h
    add_definitions(-DQT_OPENSSL)

    # Somehow qmake does not have this defined when creating mockups
    # in Qt 5.8
    add_definitions(-DQ_OS_DARWIN)

    set(ADDITIONAL_SRC qxtglobalshortcut_mac.cpp QMacPasteboardMimeUnicodeText.cpp QMacPasteboardMimeUnicodeText.h)
    set(ADDITIONAL_LIBS Qt5::MacExtras ${CarbonLibrary})
elseif(UNIX)
    find_package(X11 REQUIRED)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(ADDITIONAL_SRC qxtglobalshortcut_x11.cpp)
    set(ADDITIONAL_LIBS ${X11_LIBRARIES})
elseif(WIN32)
    find_package(Qt5WinExtras REQUIRED)

    include(generate_product_version)
    generate_product_version(
       VersionFilesOutputVariable
       NAME "Evernus"
       ICON "images/main-icon.ico"
       VERSION_MAJOR ${MAJOR_VERSION}
       VERSION_MINOR ${MINOR_VERSION}
    )

    set(ADDITIONAL_SRC qxtglobalshortcut_win.cpp ${VersionFilesOutputVariable})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /MP /wd4503")
    set(ADDITIONAL_LIBS Qt5::WinExtras)

    if(MSVC_VERSION LESS 1910)
        message(FATAL_ERROR "Compiler too old, VS 2017 or newer required.")
    endif()
endif()

set(TRANSLATIONS lang/lang_pl_PL.ts lang/lang_de_DE.ts)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

add_definitions(-DQXT_STATIC -DQTDROPBOX_DEBUG)

if(DEFINED EVERNUS_DROPBOX_APP_KEY AND DEFINED EVERNUS_DROPBOX_APP_SECRET)
    add_definitions(-DEVERNUS_DROPBOX_APP_KEY=${EVERNUS_DROPBOX_APP_KEY} -DEVERNUS_DROPBOX_APP_SECRET=${EVERNUS_DROPBOX_APP_SECRET})

    if(DEFINED QTDROPBOX_DEBUG)
        add_definitions(-DQTDROPBOX_DEBUG)
    endif()
endif()

if(DEFINED EVERNUS_ESI_SISI)
    add_definitions(-DEVERNUS_ESI_SISI)
endif()

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})

add_executable(
    ${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE
    AboutDialog.cpp
    AboutDialog.h
    ActiveTasksDialog.cpp
    ActiveTasksDialog.h
    AdjustableTableView.cpp
    AdjustableTableView.h
    AdvancedStatisticsWidget.cpp
    AdvancedStatisticsWidget.h
    AggregatedAssetModel.cpp
    AggregatedAssetModel.h
    AggregatedEventProcessor.cpp
    AggregatedEventProcessor.h
    AggregatedStatisticsModel.cpp
    AggregatedStatisticsModel.h
    ArbitrageUtils.cpp
    ArbitrageUtils.h
    AssetList.cpp
    AssetList.h
    AssetListRepository.cpp
    AssetListRepository.h
    AssetModel.cpp
    AssetModel.h
    AssetProvider.h
    AssetsImportPreferencesWidget.cpp
    AssetsImportPreferencesWidget.h
    AssetsWidget.cpp
    AssetsWidget.h
    AssetValueSnapshot.cpp
    AssetValueSnapshot.h
    AssetValueSnapshotRepository.cpp
    AssetValueSnapshotRepository.h
    AssignedContractModel.cpp
    AssignedContractModel.h
    BasicStatisticsWidget.cpp
    BasicStatisticsWidget.h
    ButtonWithTimer.cpp
    ButtonWithTimer.h
    BezierCurve.cpp
    BezierCurve.h
    Blueprint.cpp
    Blueprint.h
    CacheTimer.cpp
    CacheTimer.h
    CacheTimerProvider.h
    CacheTimerRepository.cpp
    CacheTimerRepository.h
    CachingAssetProvider.cpp
    CachingAssetProvider.h
    CachingContractProvider.cpp
    CachingContractProvider.h
    CachingEveDataProvider.cpp
    CachingEveDataProvider.h
    CachingMarketOrderProvider.cpp
    CachingMarketOrderProvider.h
    CachingNetworkAccessManagerFactory.cpp
    CachingNetworkAccessManagerFactory.h
    CalculatingDataWidget.cpp
    CalculatingDataWidget.h
    CallbackEvent.cpp
    CallbackEvent.h
    CallbackExternalOrderImporter.cpp
    CallbackExternalOrderImporter.h
    ChainableFileLogger.cpp
    ChainableFileLogger.h
    Character.cpp
    Character.h
    CharacterBoundWidget.cpp
    CharacterBoundWidget.h
    CharacterCorporationCombinedMarketOrderProvider.cpp
    CharacterCorporationCombinedMarketOrderProvider.h
    CharacterData.h
    CharacterImportPreferencesWidget.cpp
    CharacterImportPreferencesWidget.h
    CharacterManagerDialog.cpp
    CharacterManagerDialog.h
    CharacterModel.cpp
    CharacterModel.h
    CharacterRepository.cpp
    CharacterRepository.h
    CharacterWidget.cpp
    CharacterWidget.h
    CheckableTreeView.cpp
    CheckableTreeView.h
    Citadel.cpp
    Citadel.h
    CitadelAccessCache.cpp
    CitadelAccessCache.h
    CitadelEditDialog.cpp
    CitadelEditDialog.h
    CitadelManagementModel.cpp
    CitadelManagementModel.h
    CitadelManagementWidget.cpp
    CitadelManagementWidget.h
    CitadelManager.cpp
    CitadelManager.h
    CitadelManagerDialog.cpp
    CitadelManagerDialog.h
    CitadelRepository.cpp
    CitadelRepository.h
    ClickableLabel.cpp
    ClickableLabel.h
    ColorButton.cpp
    ColorButton.h
    CommandLineOptions.h
    CommonScriptAPI.cpp
    CommonScriptAPI.h
    Contract.cpp
    Contract.h
    ContractFilterProxyModel.cpp
    ContractFilterProxyModel.h
    ContractImportPreferencesWidget.cpp
    ContractImportPreferencesWidget.h
    ContractItem.cpp
    ContractItem.h
    ContractItemRepository.cpp
    ContractItemRepository.h
    ContractModel.cpp
    ContractModel.h
    ContractProvider.h
    ContractRepository.cpp
    ContractRepository.h
    Contracts.h
    ContractStatusesWidget.cpp
    ContractStatusesWidget.h
    ContractView.cpp
    ContractView.h
    ContractWidget.cpp
    ContractWidget.h
    CorpAssetValueSnapshot.cpp
    CorpAssetValueSnapshot.h
    CorpAssetValueSnapshotRepository.cpp
    CorpAssetValueSnapshotRepository.h
    CorpImportPreferencesWidget.cpp
    CorpImportPreferencesWidget.h
    CorpMarketOrderValueSnapshot.cpp
    CorpMarketOrderValueSnapshot.h
    CorpMarketOrderValueSnapshotRepository.cpp
    CorpMarketOrderValueSnapshotRepository.h
    CorpWalletSnapshot.cpp
    CorpWalletSnapshot.h
    CorpWalletSnapshotRepository.cpp
    CorpWalletSnapshotRepository.h
    CustomFPCDialog.cpp
    CustomFPCDialog.h
    DatabaseConnectionProvider.h
    DatabaseUtils.cpp
    DatabaseUtils.h
    DateFilteredPlotWidget.cpp
    DateFilteredPlotWidget.h
    DateRangeWidget.cpp
    DateRangeWidget.h
    DbSettings.h
    Defines.h
    DeviationSourceWidget.cpp
    DeviationSourceWidget.h
    DontSaveImportedOrdersCheckBox.cpp
    DontSaveImportedOrdersCheckBox.h
    DoubleTypeAggregatedDetailsWidget.cpp
    DoubleTypeAggregatedDetailsWidget.h
    DoubleTypeCompareWidget.cpp
    DoubleTypeCompareWidget.h
    DumpUploader.cpp
    DumpUploader.h
    Entity.h
    ESIExternalOrderImporter.cpp
    ESIExternalOrderImporter.h
    ESIIndividualExternalOrderImporter.cpp
    ESIIndividualExternalOrderImporter.h
    ESIInterface.cpp
    ESIInterface.h
    ESIInterfaceErrorLimiter.cpp
    ESIInterfaceErrorLimiter.h
    ESIInterfaceManager.cpp
    ESIInterfaceManager.h
    ESIManager.cpp
    ESIManager.h
    ESINetworkAccessManager.cpp
    ESINetworkAccessManager.h
    ESIOAuth.cpp
    ESIOAuth.h
    ESIOAuth2AuthorizationCodeFlow.cpp
    ESIOAuth2AuthorizationCodeFlow.h
    ESIOAuth2CharacterAuthorizationCodeFlow.cpp
    ESIOAuth2CharacterAuthorizationCodeFlow.h
    ESIOAuth2UnknownCharacterAuthorizationCodeFlow.cpp
    ESIOAuth2UnknownCharacterAuthorizationCodeFlow.h
    ESIOAuthReplyHandler.cpp
    ESIOAuthReplyHandler.h
    ESIUrls.h
    ESIWholeExternalOrderImporter.cpp
    ESIWholeExternalOrderImporter.h
    EveDatabaseConnectionProvider.cpp
    EveDatabaseConnectionProvider.h
    EveDatabaseUpdater.cpp
    EveDatabaseUpdater.h
    EveDataManagerProvider.h
    EveDataProvider.cpp
    EveDataProvider.h
    evernus.qrc
    EvernusApplication.cpp
    EvernusApplication.h
    EveType.cpp
    EveType.h
    EveTypeProvider.h
    EveTypeRepository.cpp
    EveTypeRepository.h
    ExcelDoubleSpinBox.cpp
    ExcelDoubleSpinBox.h
    ExternalOrder.cpp
    ExternalOrder.h
    ExternalOrderBuyModel.cpp
    ExternalOrderBuyModel.h
    ExternalOrderFilterProxyModel.cpp
    ExternalOrderFilterProxyModel.h
    ExternalOrderImporter.h
    ExternalOrderImporterNames.h
    ExternalOrderImporterRegistry.h
    ExternalOrderModel.cpp
    ExternalOrderModel.h
    ExternalOrderRepository.cpp
    ExternalOrderRepository.h
    ExternalOrderSellModel.cpp
    ExternalOrderSellModel.h
    ExternalOrderView.cpp
    ExternalOrderView.h
    FavoriteItem.h
    FavoriteItemRepository.cpp
    FavoriteItemRepository.h
    FavoriteLocationsButton.cpp
    FavoriteLocationsButton.h
    FileDownload.cpp
    FileDownload.h
    FilterText.cpp
    FilterText.h
    FilterTextRepository.cpp
    FilterTextRepository.h
    FlowLayout.cpp
    FlowLayout.h
    FPCController.cpp
    FPCController.h
    GeneralPreferencesWidget.cpp
    GeneralPreferencesWidget.h
    GenericMarketOrdersInfoWidget.cpp
    GenericMarketOrdersInfoWidget.h
    HttpPreferencesWidget.cpp
    HttpPreferencesWidget.h
    HttpService.cpp
    HttpService.h
    HttpSettings.h
    IconUtils.cpp
    IconUtils.h
    ImportingAnalysisWidget.cpp
    ImportingAnalysisWidget.h
    ImportingDataModel.cpp
    ImportingDataModel.h
    ImportingDataModelProxyModel.cpp
    ImportingDataModelProxyModel.h
    ImportPreferencesWidget.cpp
    ImportPreferencesWidget.h
    ImportSettings.h
    ImportSourcePreferencesWidget.cpp
    ImportSourcePreferencesWidget.h
    IndustryCostIndices.h
    IndustryImportPreferencesWidget.cpp
    IndustryImportPreferencesWidget.h
    IndustryManufacturingSetup.cpp
    IndustryManufacturingSetup.h
    IndustryManufacturingSetupController.cpp
    IndustryManufacturingSetupController.h
    IndustryManufacturingSetupEntity.cpp
    IndustryManufacturingSetupEntity.h
    IndustryManufacturingSetupModel.cpp
    IndustryManufacturingSetupModel.h
    IndustryManufacturingSetupRepository.cpp
    IndustryManufacturingSetupRepository.h
    IndustryManufacturingWidget.cpp
    IndustryManufacturingWidget.h
    IndustryMiningLedgerWidget.cpp
    IndustryMiningLedgerWidget.h
    IndustrySettings.h
    IndustryUtils.cpp
    IndustryUtils.h
    IndustryWidget.cpp
    IndustryWidget.h
    InterRegionAnalysisWidget.cpp
    InterRegionAnalysisWidget.h
    InterRegionMarketDataFilterProxyModel.cpp
    InterRegionMarketDataFilterProxyModel.h
    InterRegionMarketDataModel.cpp
    InterRegionMarketDataModel.h
    InterRegionTypeDetailsWidget.cpp
    InterRegionTypeDetailsWidget.h
    IssuedContractModel.cpp
    IssuedContractModel.h
    Item.cpp
    Item.h
    ItemCost.cpp
    ItemCost.h
    ItemCostEditDialog.cpp
    ItemCostEditDialog.h
    ItemCostModel.cpp
    ItemCostModel.h
    ItemCostProvider.h
    ItemCostRepository.cpp
    ItemCostRepository.h
    ItemCostWidget.cpp
    ItemCostWidget.h
    ItemData.h
    ItemHistoriesWidget.cpp
    ItemHistoriesWidget.h
    ItemHistoryWidget.cpp
    ItemHistoryWidget.h
    ItemNameModel.cpp
    ItemNameModel.h
    ItemRepository.cpp
    ItemRepository.h
    ItemTypeSelectDialog.cpp
    ItemTypeSelectDialog.h
    JSEveDataProvider.cpp
    JSEveDataProvider.h
    LanguageComboBox.cpp
    LanguageComboBox.h
    LanguageSelectDialog.cpp
    LanguageSelectDialog.h
    LeafFilterProxyModel.cpp
    LeafFilterProxyModel.h
    LMeveAPIManager.cpp
    LMeveAPIManager.h
    LMeveAPIInterface.cpp
    LMeveAPIInterface.h
    LMeveDataProvider.h
    LMevePreferencesWidget.cpp
    LMevePreferencesWidget.h
    LMeveTaskModel.cpp
    LMeveTaskModel.h
    LMeveTaskRepository.cpp
    LMeveTaskRepository.h
    LMeveWidget.cpp
    LMeveWidget.h
    LMeveSettings.h
    LMeveTask.cpp
    LMeveTask.h
    LocationBookmark.cpp
    LocationBookmark.h
    LocationBookmarkRepository.cpp
    LocationBookmarkRepository.h
    LocationBookmarkSelectDialog.cpp
    LocationBookmarkSelectDialog.h
    LookupActionGroup.cpp
    LookupActionGroup.h
    LookupActionGroupModelConnector.cpp
    LookupActionGroupModelConnector.h
    main.cpp
    MainDatabaseConnectionProvider.cpp
    MainDatabaseConnectionProvider.h
    MainWindow.cpp
    MainWindow.h
    MarginToolDialog.cpp
    MarginToolDialog.h
    MarginToolSettings.h
    MarketAnalysisDataFetcher.cpp
    MarketAnalysisDataFetcher.h
    MarketAnalysisSettings.h
    MarketAnalysisUtils.cpp
    MarketAnalysisUtils.h
    MarketAnalysisWidget.h
    MarketAnalysisWidget.cpp
    MarketBrowserWidget.cpp
    MarketBrowserWidget.h
    MarketDataProvider.h
    MarketGroup.cpp
    MarketGroup.h
    MarketGroupRepository.cpp
    MarketGroupRepository.h
    MarketHistory.h
    MarketHistoryEntry.h
    MarketLogExternalOrderImporter.cpp
    MarketLogExternalOrderImporter.h
    MarketLogExternalOrderImporterThread.cpp
    MarketLogExternalOrderImporterThread.h
    MarketOrder.cpp
    MarketOrder.h
    MarketOrderArchiveModel.cpp
    MarketOrderArchiveModel.h
    MarketOrderBuyModel.cpp
    MarketOrderBuyModel.h
    MarketOrderDataFetcher.cpp
    MarketOrderDataFetcher.h
    MarketOrderFilterProxyModel.cpp
    MarketOrderFilterProxyModel.h
    MarketOrderFilterWidget.cpp
    MarketOrderFilterWidget.h
    MarketOrderInfoWidget.cpp
    MarketOrderInfoWidget.h
    MarketOrderModel.h
    MarketOrderPerformanceModel.cpp
    MarketOrderPerformanceModel.h
    MarketOrderPriceStatusesWidget.cpp
    MarketOrderPriceStatusesWidget.h
    MarketOrderProvider.h
    MarketOrderRepository.cpp
    MarketOrderRepository.h
    MarketOrders.h
    MarketOrderSellModel.cpp
    MarketOrderSellModel.h
    MarketOrdersInfoWidget.h
    MarketOrderStatesWidget.cpp
    MarketOrderStatesWidget.h
    MarketOrderTreeModel.cpp
    MarketOrderTreeModel.h
    MarketOrderValueSnapshot.cpp
    MarketOrderValueSnapshot.h
    MarketOrderValueSnapshotRepository.cpp
    MarketOrderValueSnapshotRepository.h
    MarketOrderView.cpp
    MarketOrderView.h
    MarketOrderViewWithTransactions.cpp
    MarketOrderViewWithTransactions.h
    MarketOrderVolumeItemDelegate.cpp
    MarketOrderVolumeItemDelegate.h
    MarketOrderWidget.cpp
    MarketOrderWidget.h
    MarketPrices.h
    MathUtils.h
    MenuBarWidget.cpp
    MenuBarWidget.h
    MetaGroup.cpp
    MetaGroup.h
    MetaGroupRepository.cpp
    MetaGroupRepository.h
    MiningLedger.cpp
    MiningLedger.h
    MiningLedgerBarGraph.cpp
    MiningLedgerBarGraph.h
    MiningLedgerModel.cpp
    MiningLedgerModel.h
    MiningLedgerRepository.cpp
    MiningLedgerRepository.h
    MiningLedgerSolarSystemsModel.cpp
    MiningLedgerSolarSystemsModel.h
    MiningLedgerTypesModel.cpp
    MiningLedgerTypesModel.h
    ModelUtils.cpp
    ModelUtils.h
    ModelWithTypes.h
    NetworkPreferencesWidget.cpp
    NetworkPreferencesWidget.h
    NetworkSettings.h
    NewCharacterController.cpp
    NewCharacterController.h
    NumberFormatDelegate.cpp
    NumberFormatDelegate.h
    OrderScript.cpp
    OrderScript.h
    OrderScriptRepository.cpp
    OrderScriptRepository.h
    OrderSettings.h
    OreReprocessingArbitrageModel.cpp
    OreReprocessingArbitrageModel.h
    OreReprocessingArbitrageWidget.cpp
    OreReprocessingArbitrageWidget.h
    OrderPreferencesWidget.cpp
    OrderPreferencesWidget.h
    PathPreferencesWidget.cpp
    PathPreferencesWidget.h
    PathSettings.h
    PathUtils.cpp
    PathUtils.h
    PreferencesDialog.cpp
    PreferencesDialog.h
    PricePreferencesWidget.cpp
    PricePreferencesWidget.h
    PriceSettings.h
    PriceType.h
    PriceTypeComboBox.cpp
    PriceTypeComboBox.h
    PriceUtils.cpp
    PriceUtils.h
    ProgressiveCounter.cpp
    ProgressiveCounter.h
    ProxyWebExternalOrderImporter.cpp
    ProxyWebExternalOrderImporter.h
    qcustomplot.cpp
    qcustomplot.h
    QDataStreamUtils.h
    qdropbox2.cpp
    qdropbox2.h
    qdropbox2account.cpp
    qdropbox2account.h
    qdropbox2common.h
    qdropbox2entity.h
    qdropbox2entityinfo.cpp
    qdropbox2entityinfo.h
    qdropbox2file.cpp
    qdropbox2file.h
    qdropbox2folder.cpp
    qdropbox2folder.h
    qdropbox2global.h
    qhttpheader.cpp
    qhttpheader.h
    QObjectDeleteLaterDeleter.h
    QtScriptSyntaxHighlighter.cpp
    QtScriptSyntaxHighlighter.h
    qxtabstracthttpconnector.cpp
    qxtabstracthttpconnector.h
    qxtabstractwebservice.cpp
    qxtabstractwebservice.h
    qxtabstractwebsessionmanager_p.h
    qxtabstractwebsessionmanager.cpp
    qxtabstractwebsessionmanager.h
    qxtboundcfunction.h
    qxtboundfunction.h
    qxtboundfunctionbase.h
    qxtcsvmodel.cpp
    qxtcsvmodel.h
    qxtfifo.cpp
    qxtfifo.h
    qxtglobal.cpp
    qxtglobal.h
    qxtglobalshortcut.cpp
    qxtglobalshortcut.h
    qxtglobalshortcut_p.h
    qxthmac.cpp
    qxthmac.h
    qxthtmltemplate.cpp
    qxthtmltemplate.h
    qxthttpserverconnector.cpp
    qxthttpsessionmanager.cpp
    qxthttpsessionmanager.h
    qxtmail_p.h
    qxtmailattachment.cpp
    qxtmailattachment.h
    qxtmailmessage.cpp
    qxtmailmessage.h
    qxtmetaobject.cpp
    qxtmetaobject.h
    qxtmetatype.h
    qxtnull.cpp
    qxtnull.h
    qxtnullable.h
    qxtscgiserverconnector.cpp
    qxtsmtp_p.h
    qxtsmtp.cpp
    qxtsmtp.h
    qxtsslserver.cpp
    qxtsslserver.h
    qxtwebcontent.cpp
    qxtwebcontent.h
    qxtwebevent.cpp
    qxtwebevent.h
    qxtwebslotservice.cpp
    qxtwebslotservice.h
    RegionAnalysisWidget.cpp
    RegionAnalysisWidget.h
    RegionComboBox.cpp
    RegionComboBox.h
    RegionStationPreset.cpp
    RegionStationPreset.h
    RegionStationPresetDialog.cpp
    RegionStationPresetDialog.h
    RegionStationPresetRepository.cpp
    RegionStationPresetRepository.h
    RegionTypePreset.cpp
    RegionTypePreset.h
    RegionTypePresetRepository.cpp
    RegionTypePresetRepository.h
    RegionTypeSelectDialog.cpp
    RegionTypeSelectDialog.h
    ReplyTimeout.cpp
    ReplyTimeout.h
    ReportsWidget.cpp
    ReportsWidget.h
    Repository.h
    RepositoryProvider.h
    ReprocessingArbitrageModel.cpp
    ReprocessingArbitrageModel.h
    ReprocessingArbitrageWidget.cpp
    ReprocessingArbitrageWidget.h
    ScrapmetalReprocessingArbitrageModel.cpp
    ScrapmetalReprocessingArbitrageModel.h
    ScrapmetalReprocessingArbitrageWidget.cpp
    ScrapmetalReprocessingArbitrageWidget.h
    ScriptOrderProcessingModel.cpp
    ScriptOrderProcessingModel.h
    ScriptUtils.cpp
    ScriptUtils.h
    SecurityHelper.cpp
    SecurityHelper.h
    SellMarketOrdersInfoWidget.cpp
    SellMarketOrdersInfoWidget.h
    SimpleCrypt.cpp
    SimpleCrypt.h
    SingleRegionComboBox.cpp
    SingleRegionComboBox.h
    SizeRememberingWidget.cpp
    SizeRememberingWidget.h
    SolarSystemComboBox.cpp
    SolarSystemComboBox.h
    SoundPreferencesWidget.cpp
    SoundPreferencesWidget.h
    SoundSettings.h
    SourceDestinationSelectWidget.cpp
    SourceDestinationSelectWidget.h
    SSOAuthDialog.cpp
    SSOAuthDialog.h
    SSOMessageBox.cpp
    SSOMessageBox.h
    SSOSettings.h
    SSOUtils.cpp
    SSOUtils.h
    StandardExceptionQtWrapperException.h
    StandardModelProxyWidget.cpp
    StandardModelProxyWidget.h
    StationModel.cpp
    StationModel.h
    StationSelectButton.cpp
    StationSelectButton.h
    StationSelectDialog.cpp
    StationSelectDialog.h
    StationView.cpp
    StationView.h
    StatisticsPreferencesWidget.cpp
    StatisticsPreferencesWidget.h
    StatisticsSettings.h
    StatisticsWidget.cpp
    StatisticsWidget.h
    StructureHuntInterface.cpp
    StructureHuntInterface.h
    StyledTreeView.cpp
    StyledTreeView.h
    StyledTreeViewItemDelegate.cpp
    StyledTreeViewItemDelegate.h
    SyncDialog.cpp
    SyncDialog.h
    SyncPreferencesWidget.cpp
    SyncPreferencesWidget.h
    SyncSettings.h
    TaskConstants.h
    TaskManager.h
    TextFilterWidget.cpp
    TextFilterWidget.h
    TextUtils.cpp
    TextUtils.h
    TimerTypes.h
    TradeableTypesTreeModel.cpp
    TradeableTypesTreeModel.h
    TradeableTypesTreeView.cpp
    TradeableTypesTreeView.h
    TypeAggregatedDetailsFilterWidget.cpp
    TypeAggregatedDetailsFilterWidget.h
    TypeAggregatedDetailsWidget.cpp
    TypeAggregatedDetailsWidget.h
    TypeAggregatedGraphWidget.cpp
    TypeAggregatedGraphWidget.h
    TypeAggregatedMarketDataFilterProxyModel.cpp
    TypeAggregatedMarketDataFilterProxyModel.h
    TypeAggregatedMarketDataModel.cpp
    TypeAggregatedMarketDataModel.h
    TypeLocationPairs.h
    TypeLookupUtils.cpp
    TypeLookupUtils.h
    TypePerformanceModel.cpp
    TypePerformanceModel.h
    TypeSellPriceResolver.cpp
    TypeSellPriceResolver.h
    UISettings.h
    Updater.cpp
    Updater.h
    UpdaterSettings.h
    UpdateTimer.cpp
    UpdateTimer.h
    UpdateTimerRepository.cpp
    UpdateTimerRepository.h
    Version.h
    VolumeType.h
    WalletEntryFilterWidget.cpp
    WalletEntryFilterWidget.h
    WalletJournal.h
    WalletJournalEntry.cpp
    WalletJournalEntry.h
    WalletJournalEntryRepository.cpp
    WalletJournalEntryRepository.h
    WalletJournalModel.cpp
    WalletJournalModel.h
    WalletJournalWidget.cpp
    WalletJournalWidget.h
    WalletPreferencesWidget.cpp
    WalletPreferencesWidget.h
    WalletSettings.h
    WalletSnapshot.cpp
    WalletSnapshot.h
    WalletSnapshotRepository.cpp
    WalletSnapshotRepository.h
    WalletTransaction.cpp
    WalletTransaction.h
    WalletTransactionRepository.cpp
    WalletTransactionRepository.h
    WalletTransactions.h
    WalletTransactionsModel.cpp
    WalletTransactionsModel.h
    WalletTransactionsWidget.cpp
    WalletTransactionsWidget.h
    WalletTransactionView.cpp
    WalletTransactionView.h
    WarningBarWidget.cpp
    WarningBarWidget.h
    ${TRANSLATIONS}
    ${QM_FILES}
    ${ADDITIONAL_SRC}
)

if(WIN32)
    if(${CMAKE_CL_64})
        # cmake bug
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LINK_FLAGS} /machine:x64")
    endif()
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist")

    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CONFIGURATION_TYPES "Release;MinSizeRel;RelWithDebInfo;Debug")

    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Evernus")

    # Make sure bundle will be always created at the same path
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
    set(BUNDLE_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app")
endif()

target_link_libraries(
    ${PROJECT_NAME}
    Boost::boost
    Qt5::Concurrent
    Qt5::Core
    Qt5::DataVisualization
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::Multimedia
    Qt5::Network
    Qt5::NetworkAuth
    Qt5::PrintSupport
    Qt5::Qml
    Qt5::QuickWidgets
    Qt5::Sql
    Qt5::Widgets
    Threads::Threads
    ${ADDITIONAL_LIBS}
)

if(EVERNUS_CREATE_DUMPS)
    target_link_libraries(
        ${PROJECT_NAME}
        Breakpad::Client
    )
endif()

set(RESOURCES
    "resources"
    "${CMAKE_CURRENT_BINARY_DIR}/translations"
)

if(APPLE)
    foreach(RES ${RESOURCES})
        install(DIRECTORY ${RES} DESTINATION "${BUNDLE_PATH}/Contents/MacOS/")
    endforeach()

    install(FILES iconfile.icns DESTINATION "${BUNDLE_PATH}/Contents/Resources/")

    find_path(MACDEPLOYQT_PATH macdeployqt PATH_SUFFIXES bin)
    if(NOT MACDEPLOYQT_PATH)
        message(FATAL_ERROR "Could not find macdeployqt for OSX bundling. You can point MACDEPLOYQT_PATH to it's path.")
    endif()

    # If macdeployqt is found just build and install.
    # With xcode it is:
    # $ xcodebuild -project Evernus.xcodeproj
    # $ xcodebuild -project Evernus.xcodeproj -target install
    # OSX budle is ready.
    install(CODE "
            execute_process(COMMAND ${MACDEPLOYQT_PATH}/macdeployqt \"${BUNDLE_PATH}\" \"-qmldir=${CMAKE_SOURCE_DIR}/qml\" -verbose=2)
            message(STATUS \"Creating dmg of the app\")
            execute_process(COMMAND hdiutil create -fs HFS+ -volname ${PROJECT_NAME} -srcfolder \"${BUNDLE_PATH}\" -ov -format UDZO \"${PROJECT_NAME}.dmg\")
            " COMPONENT Runtime
    )
elseif(UNIX)
    foreach(RES ${RESOURCES})
        install(DIRECTORY ${RES} DESTINATION "share/Evernus")
    endforeach()

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
endif()
