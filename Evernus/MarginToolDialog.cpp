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
#include <functional>
#include <algorithm>
#include <memory>
#include <thread>
#include <cmath>

#include <QDialogButtonBox>
#include <QStringBuilder>
#include <QDesktopWidget>
#include <QApplication>
#include <QRadioButton>
#include <QTableWidget>
#include <QDirIterator>
#include <QHeaderView>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QClipboard>
#include <QTabWidget>

#include <QDateTime>
#include <QSettings>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QRegExp>
#include <QLabel>
#include <QtDebug>
#include <QFont>
#include <QFile>
#include <QDir>

#include "MarginToolSettings.h"
#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "PriceSettings.h"
#include "PathSettings.h"
#include "StationView.h"
#include "Repository.h"
#include "PriceUtils.h"
#include "UISettings.h"
#include "PathUtils.h"
#include "TextUtils.h"

#include "MarginToolDialog.h"


namespace Evernus
{
    const QString MarginToolDialog::settingsGeometryKey = "marginTool/geometry";
   

    MarginToolDialog::MarginToolDialog(const Repository<Character> &characterRepository,
                                       const ItemCostProvider &itemCostProvider,
                                       EveDataProvider &dataProvider,
                                       QWidget *parent)
        : QDialog(parent, Qt::Dialog | Qt::WindowMinimizeButtonHint)
        , mCharacterRepository(characterRepository)
        , mItemCostProvider(itemCostProvider)
        , mDataProvider(dataProvider)
        
    {
        

        QSettings settings;
        const auto alwaysOnTop
            = settings.value(MarginToolSettings::alwaysOnTopKey, MarginToolSettings::alwaysOnTopDefault).toBool();

        auto mainLayout = new QVBoxLayout{this};
#ifdef Q_OS_OSX
        mainLayout->setContentsMargins(10, 10, 10, 5);
#endif

        auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs);
        tabs->addTab(createMarginDataTab(), tr("Margin data"));
        tabs->addTab(createDataSourceTab(), tr("Data source"));

        auto buttonLayout = new QHBoxLayout{};
        mainLayout->addLayout(buttonLayout);

        auto alwaysOnTopBtn = new QCheckBox{tr("Always on top"), this};
        buttonLayout->addWidget(alwaysOnTopBtn);
        alwaysOnTopBtn->setChecked(alwaysOnTop);
        connect(alwaysOnTopBtn, &QCheckBox::stateChanged, this, &MarginToolDialog::toggleAlwaysOnTop);

        auto buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
        buttonLayout->addWidget(buttons, 1);
        auto quitBtn = buttons->addButton(tr("Quit application"), QDialogButtonBox::DestructiveRole);
        buttons->button(QDialogButtonBox::Close)->setDefault(true);
        connect(buttons, &QDialogButtonBox::rejected, this, &MarginToolDialog::close);
        connect(quitBtn, &QPushButton::clicked, this, [this](){
            savePosition();
            emit quit();
        });

        setUpWatcher();
        connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &MarginToolDialog::refreshData);

        setWindowTitle(tr("Margin tool"));
        setAttribute(Qt::WA_DeleteOnClose);
#ifndef Q_OS_OSX
        setMaximumWidth(510);
#endif
        setNewWindowFlags(alwaysOnTop);

        restoreGeometry(settings.value(settingsGeometryKey).toByteArray());


    }

    void MarginToolDialog::setCharacter(Character::IdType id)
    {
        mCharacterId = id;

        try
        {
            const auto taxes = PriceUtils::calculateTaxes(*mCharacterRepository.find(mCharacterId));

            mBrokerFeeLabel->setText(QStringLiteral("%1%/%2%").arg(taxes.mBuyBrokerFee * 100., 0, 'f', 2).arg(taxes.mSellBrokerFee * 100., 0, 'f', 2));
            mSalesTaxLabel->setText(QStringLiteral("%1%").arg(taxes.mSalesTax * 100., 0, 'f', 2));
        }
        catch (const Repository<Character>::NotFoundException &)
        {
            mBrokerFeeLabel->setText("-");
            mSalesTaxLabel->setText("-");
            mRevenueLabel->setText("-");
            mCostOfSalesLabel->setText("-");
            mMarginLabel->setText("-");
            mMarkupLabel->setText("-");
        }
    }

    void MarginToolDialog::copyBuyPrice()
    {
        auto clipboard = QApplication::clipboard();
        clipboard->setText(QString::number(mBuyPrice, 'f', 2));
    }

    void MarginToolDialog::copySellPrice()
    {
        auto clipboard = QApplication::clipboard();
        clipboard->setText(QString::number(mSellPrice, 'f', 2));
    }

    void MarginToolDialog::handleNewPreferences()
    {
        setUpWatcher();
    }

    void MarginToolDialog::toggleAlwaysOnTop(int state)
    {
        const auto alwaysOnTop = state == Qt::Checked;

        QSettings settings;
        settings.setValue(MarginToolSettings::alwaysOnTopKey, alwaysOnTop);

        setNewWindowFlags(alwaysOnTop);

        const auto geom = geometry();
        show();
        setGeometry(geom);
    }

    void MarginToolDialog::refreshData(const QString &path)
    {
        const auto targetFile = getNewFile(path);
        if (targetFile.isEmpty())
            return;

        mKnownFiles << targetFile;

        const QString logFile = path % "/" % targetFile;

        qDebug() << "Calculating margin from file: " << logFile;

        QSettings settings;
        QFile file{logFile};
        QFileInfo info{file};

#ifdef Q_OS_WIN
        if (settings.value(PriceSettings::priceAltImportKey, PriceSettings::priceAltImportDefault).toBool())
        {
            while (!file.open(QIODevice::ReadWrite))
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
        else
        {
#endif
            const auto modTimeDelay = settings.value(PriceSettings::importLogWaitTimeKey, PriceSettings::importLogWaitTimeDefault).toUInt();

            // wait for Eve to finish dumping data
            forever
            {
                const auto modTime = info.lastModified();
                if (modTime.msecsTo(QDateTime::currentDateTime()) >= modTimeDelay)
                    break;
            }

            if (!file.open(QIODevice::ReadOnly))
                return;
#ifdef Q_OS_WIN
        }
#endif

        file.readLine();

        auto buy = -1.;
        auto sell = -1.;

        uint buyVol = 0, buyInit = 0;
        uint sellVol = 0, sellInit = 0;

        auto buyCount = 0u;
        auto sellCount = 0u;
        auto buyout = 0.;

        QString name;
        const auto priceTime = info.created();

        auto typeId = EveType::invalidId;

        const auto logColumns = 14;

        const auto volRemainingColumn = 1;
        const auto volEnteredColumn = 5;
        const auto jumpsColumn = 13;

        const auto ignoreMinVolume
            = settings.value(PriceSettings::ignoreOrdersWithMinVolumeKey, PriceSettings::ignoreOrdersWithMinVolumeDefault).toBool();

        const auto rangeThreshold = mRangeThresholdEdit->value();

        std::vector<ExternalOrder> parsedOrders;

        while (!file.atEnd())
        {
            const QString line = file.readLine();
            const auto values = line.split(',');

            if (values.count() >= logColumns)
            {
                ExternalOrder order = ExternalOrder::parseLogLine(values);

                if (ignoreMinVolume && order.getMinVolume() > 1)
                    continue;

                order.setUpdateTime(priceTime);

                if (typeId == EveType::invalidId)
                {
                    typeId = order.getTypeId();
                    name = mDataProvider.getTypeName(typeId);
                }

                const auto jumps = values[jumpsColumn].toInt();

                if (order.getType() == ExternalOrder::Type::Buy)
                {
                    // warning: this does not take into account orders in the same system, but different station -
                    //          there's no way to check if the station matches
                    if (jumps != 0)
                    {
                        const int range = order.getRange();
                        if (jumps - std::max(range, 0) > rangeThreshold)
                        {
                            parsedOrders.emplace_back(std::move(order));
                            continue;
                        }
                    }

                    if (order.getPrice() > buy)
                        buy = order.getPrice();

                    buyVol += static_cast<uint>(values[volRemainingColumn].toDouble());
                    buyInit += static_cast<uint>(values[volEnteredColumn].toDouble());

                    ++buyCount;
                }
                else if (jumps <= rangeThreshold)
                {
                    const auto price = order.getPrice();
                    if (price < sell || sell < 0.)
                        sell = price;

                    const auto remaining = static_cast<uint>(values[volRemainingColumn].toDouble());

                    buyout += remaining * price;
                    sellVol += remaining;
                    sellInit += static_cast<uint>(values[volEnteredColumn].toDouble());

                    ++sellCount;
                }

                parsedOrders.emplace_back(std::move(order));
            }
        }

        mDataProvider.updateExternalOrders(parsedOrders);

        if (settings.value(PathSettings::deleteLogsKey, PathSettings::deleteLogsDefault).toBool())
        {
            mWatcher.blockSignals(true);
            file.remove();
            mWatcher.blockSignals(false);
        }

        if (mItemCostSourceBtn->isChecked())
        {
            const auto cost = mItemCostProvider.fetchForCharacterAndType(mCharacterId, typeId);
            if (!cost->isNew())
                buy = cost->getAdjustedCost() - PriceUtils::getPriceDelta();
        }
        else if (mStationSourceBtn->isChecked())
        {
            const auto station = mStationView->getStationId();
            if (station == 0)
                buy = 0.;
            else
                buy = mDataProvider.getTypeBuyPrice(typeId, station)->getPrice();
        }

        const auto curLocale = locale();

        mNameLabel->setText(name);
        mBuyOrdersLabel->setText(curLocale.toString(buyCount));
        mSellOrdersLabel->setText(curLocale.toString(sellCount));
        mBuyVolLabel->setText(QString{"%1/%2"}.arg(curLocale.toString(buyVol)).arg(curLocale.toString(buyInit - buyVol)));
        mSellVolLabel->setText(QString{"%1/%2"}.arg(curLocale.toString(sellVol)).arg(curLocale.toString(sellInit - sellVol)));
        mBuyoutLabel->setText(TextUtils::currencyToString(buyout, curLocale));

        updateInfo(buy, sell, true);
    }

    void MarginToolDialog::refreshDataByEdits()
    {
        const auto curLocale = locale();
        updateInfo(curLocale.toDouble(mBestBuyEdit->text()), curLocale.toDouble(mBestSellEdit->text()), false);
    }

    void MarginToolDialog::saveCopyMode()
    {
        QSettings settings;

        if (mDontCopyBtn->isChecked())
            settings.setValue(PriceSettings::copyModeKey, static_cast<int>(PriceSettings::CopyMode::DontCopy));
        else if (mCopySellBtn->isChecked())
            settings.setValue(PriceSettings::copyModeKey, static_cast<int>(PriceSettings::CopyMode::CopySell));
        else if (mCopyBuyBtn->isChecked())
            settings.setValue(PriceSettings::copyModeKey, static_cast<int>(PriceSettings::CopyMode::CopyBuy));
    }

    void MarginToolDialog::saveSelectedStation(quint64 id)
    {
        if (id != 0)
        {
            QSettings settings;
            settings.setValue(PriceSettings::costSourceStationKey, mStationView->getSelectedPath());
        }
    }

    void MarginToolDialog::closeEvent(QCloseEvent *event)
    {
        emit hidden();

        savePosition();
        QDialog::closeEvent(event);
    }

    void MarginToolDialog::setNewWindowFlags(bool alwaysOnTop)
    {
        auto flags = windowFlags();
        if (alwaysOnTop)
            flags |= Qt::WindowStaysOnTopHint;
        else
            flags &= ~Qt::WindowStaysOnTopHint;

        setWindowFlags(flags);
    }

    QTableWidget *MarginToolDialog::createSampleTable()
    {
        auto table = new QTableWidget{this};
        table->setColumnCount(3);
        table->setAlternatingRowColors(true);
        table->setRowCount(static_cast<int>(std::log10(samples)));
        table->setHorizontalHeaderLabels(QStringList{} << "Volume" << "Cost" << "Profit");
        table->verticalHeader()->hide();

        return table;
    }

    void MarginToolDialog::savePosition() const
    {
        QSettings settings;
        settings.setValue(settingsGeometryKey, saveGeometry());
    }

    QWidget *MarginToolDialog::createMarginDataTab()
    {
        QFont bigFont;
        bigFont.setBold(true);
        bigFont.setPointSize(bigFont.pointSize() + 3);

        QFont boldFont;
        boldFont.setBold(true);

        auto tabWidget = new QWidget{this};

        auto mainLayout = new QVBoxLayout{tabWidget};
#ifdef Q_OS_OSX
        mainLayout->setContentsMargins(5, 0, 5, 5);
        mainLayout->setSpacing(7);
#endif

        mNameLabel = new QLabel{tr("export market logs in game"), this};
        mainLayout->addWidget(mNameLabel);
        mNameLabel->setFont(bigFont);

        auto priceGroup = new QGroupBox{this};
        mainLayout->addWidget(priceGroup);

        auto priceLayout = new QGridLayout{priceGroup};
#ifdef Q_OS_OSX
        priceLayout->setVerticalSpacing(0);
#endif

        priceLayout->addWidget(new QLabel{tr("Sell:")}, 0, 0);
        priceLayout->addWidget(new QLabel{tr("Buy:")}, 1, 0);
        priceLayout->addWidget(new QLabel{tr("Profit:")}, 2, 0);
        priceLayout->addWidget(new QLabel{tr("Revenue:")}, 0, 2);
        priceLayout->addWidget(new QLabel{tr("Cost of sales:")}, 1, 2);
        priceLayout->addWidget(new QLabel{tr("Buyout:")}, 2, 2);

        auto sellLayout = new QHBoxLayout{};
        priceLayout->addLayout(sellLayout, 0, 1);

        mBestSellEdit = new QLineEdit{this};
        sellLayout->addWidget(mBestSellEdit);
        mBestSellEdit->setPlaceholderText("-");
        mBestSellEdit->setSizePolicy(QSizePolicy{QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::LineEdit});
        connect(mBestSellEdit, &QLineEdit::textEdited, this, &MarginToolDialog::refreshDataByEdits);

        auto copySellBtn = new QPushButton{QIcon{":/images/paste_plain.png"}, QString{}, this};
        sellLayout->addWidget(copySellBtn);
        copySellBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        copySellBtn->setToolTip(tr("Copy to clipboard"));
        connect(copySellBtn, &QPushButton::clicked, this, &MarginToolDialog::copySellPrice);

        auto buyLayout = new QHBoxLayout{};
        priceLayout->addLayout(buyLayout, 1, 1);

        mBestBuyEdit = new QLineEdit{this};
        buyLayout->addWidget(mBestBuyEdit);
        mBestBuyEdit->setPlaceholderText("-");
        mBestBuyEdit->setSizePolicy(QSizePolicy{QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::LineEdit});
        connect(mBestBuyEdit, &QLineEdit::textEdited, this, &MarginToolDialog::refreshDataByEdits);

        auto copyBuyBtn = new QPushButton{QIcon{":/images/paste_plain.png"}, QString{}, this};
        buyLayout->addWidget(copyBuyBtn);
        copyBuyBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        copyBuyBtn->setToolTip(tr("Copy to clipboard"));
        connect(copyBuyBtn, &QPushButton::clicked, this, &MarginToolDialog::copyBuyPrice);

        mProfitLabel = new QLabel{"-", this};
        priceLayout->addWidget(mProfitLabel, 2, 1);
        mProfitLabel->setFont(boldFont);

        mRevenueLabel = new QLabel{"-", this};
        priceLayout->addWidget(mRevenueLabel, 0, 3);

        mCostOfSalesLabel = new QLabel{"-", this};
        priceLayout->addWidget(mCostOfSalesLabel, 1, 3);

        mBuyoutLabel = new QLabel{"-", this};
        priceLayout->addWidget(mBuyoutLabel, 2, 3);

        auto orderGroup = new QGroupBox{this};
        mainLayout->addWidget(orderGroup);

        auto orderLayout = new QGridLayout{orderGroup};

        orderLayout->addWidget(new QLabel{tr("Buy orders:")}, 0, 0);

        mBuyOrdersLabel = new QLabel{"-", this};
        orderLayout->addWidget(mBuyOrdersLabel, 0, 1);

        orderLayout->addWidget(new QLabel{tr("Sell orders:")}, 1, 0);

        mSellOrdersLabel = new QLabel{"-", this};
        orderLayout->addWidget(mSellOrdersLabel, 1, 1);

        orderLayout->addWidget(new QLabel{tr("Buy volume/movement:")}, 0, 2);

        mBuyVolLabel = new QLabel{"-", this};
        orderLayout->addWidget(mBuyVolLabel, 0, 3);

        orderLayout->addWidget(new QLabel{tr("Sell volume/movement:")}, 1, 2);

        mSellVolLabel = new QLabel{"-", this};
        orderLayout->addWidget(mSellVolLabel, 1, 3);

        auto infoLayout = new QHBoxLayout{};
        mainLayout->addLayout(infoLayout);

        auto marginGroup = new QGroupBox{this};
        infoLayout->addWidget(marginGroup);

        auto marginLayout = new QGridLayout{marginGroup};

        marginLayout->addWidget(new QLabel{tr("Margin:")}, 0, 0);

        mMarginLabel = new QLabel{"-", this};
        marginLayout->addWidget(mMarginLabel, 0, 1);
        mMarginLabel->setFont(bigFont);

        marginLayout->addWidget(new QLabel{tr("Markup:")}, 1, 0);

        mMarkupLabel = new QLabel{"-", this};
        marginLayout->addWidget(mMarkupLabel, 1, 1);

        auto copyGroup = new QGroupBox{tr("Autocopy"), this};
        infoLayout->addWidget(copyGroup);

        auto copyLayout = new QVBoxLayout{copyGroup};

        QSettings settings;

        const auto copyMode = static_cast<const PriceSettings::CopyMode>(
            settings.value(PriceSettings::copyModeKey, static_cast<int>(PriceSettings::copyModeDefault)).toInt());

        mDontCopyBtn = new QRadioButton{tr("Nothing"), this};
        copyLayout->addWidget(mDontCopyBtn);
        if (copyMode == PriceSettings::CopyMode::DontCopy)
            mDontCopyBtn->setChecked(true);

        mCopySellBtn = new QRadioButton{tr("Sell price"), this};
        copyLayout->addWidget(mCopySellBtn);
        if (copyMode == PriceSettings::CopyMode::CopySell)
            mCopySellBtn->setChecked(true);

        mCopyBuyBtn = new QRadioButton{tr("Buy price"), this};
        copyLayout->addWidget(mCopyBuyBtn);
        if (copyMode == PriceSettings::CopyMode::CopyBuy)
            mCopyBuyBtn->setChecked(true);

        connect(mDontCopyBtn, &QRadioButton::toggled, this, &MarginToolDialog::saveCopyMode);
        connect(mCopySellBtn, &QRadioButton::toggled, this, &MarginToolDialog::saveCopyMode);
        connect(mCopySellBtn, &QRadioButton::toggled, this, [=](bool checked) {
            if (checked)
                copySellPrice();
        });
        connect(mCopyBuyBtn, &QRadioButton::toggled, this, &MarginToolDialog::saveCopyMode);
        connect(mCopyBuyBtn, &QRadioButton::toggled, this, [=](bool checked) {
            if (checked)
                copyBuyPrice();
        });

        auto taxesGroup = new QGroupBox{this};
        mainLayout->addWidget(taxesGroup);

        auto taxesLayout = new QHBoxLayout{taxesGroup};

        taxesLayout->addWidget(new QLabel{tr("Broker fee:"), this});

        mBrokerFeeLabel = new QLabel{"-", this};
        taxesLayout->addWidget(mBrokerFeeLabel);

        taxesLayout->addWidget(new QLabel{tr("Sales tax:"), this});

        mSalesTaxLabel = new QLabel{"-", this};
        taxesLayout->addWidget(mSalesTaxLabel);

        auto sampleGroup = new QGroupBox{tr("Sample data"), this};
        mainLayout->addWidget(sampleGroup);

        auto sampleLayout = new QHBoxLayout{sampleGroup};

        m1SampleDataTable = createSampleTable();
        sampleLayout->addWidget(m1SampleDataTable);

        m5SampleDataTable = createSampleTable();
        sampleLayout->addWidget(m5SampleDataTable);

        auto helpLayout = new QHBoxLayout{};
        mainLayout->addLayout(helpLayout);

        auto hideSamplesBtn = new QCheckBox{tr("Show sample data"), this};
        helpLayout->addWidget(hideSamplesBtn);
        hideSamplesBtn->setChecked(
            settings.value(UISettings::showMarginToolSampleDataKey, UISettings::showMarginToolSampleDataDefault).toBool());
        sampleGroup->setVisible(hideSamplesBtn->isChecked());
        connect(hideSamplesBtn, &QCheckBox::stateChanged, this, [=](auto state) {
            const auto show = state == Qt::Checked;

            sampleGroup->setVisible(show);
            adjustSize();

            QSettings settings;
            settings.setValue(UISettings::showMarginToolSampleDataKey, show);
        });

        auto helpLabel = new QLabel{tr("If you experience problems with importing orders, adjust margin tool settings in the Preferences."), this};
        helpLayout->addWidget(helpLabel, 1);
        helpLabel->setWordWrap(true);

        return tabWidget;
    }

    QWidget *MarginToolDialog::createDataSourceTab()
    {
        auto tabWidget = new QWidget{this};
        auto mainLayout = new QVBoxLayout{tabWidget};
#ifdef Q_OS_OSX
        mainLayout->setContentsMargins(5, 5, 5, 5);
#endif

        auto preferencesGroup = new QGroupBox{this};
        mainLayout->addWidget(preferencesGroup);

        auto preferencesLayout = new QHBoxLayout{preferencesGroup};
        preferencesLayout->addWidget(new QLabel{tr("Range threshold:"), this});

        QSettings settings;

        mRangeThresholdEdit = new QSpinBox{this};
        preferencesLayout->addWidget(mRangeThresholdEdit);
        mRangeThresholdEdit->setSuffix(tr(" jumps"));
        mRangeThresholdEdit->setValue(settings.value(PriceSettings::rangeThresholdKey, PriceSettings::rangeThresholdDefault).toInt());
        connect(mRangeThresholdEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](const auto value) {
            QSettings settings;
            settings.setValue(PriceSettings::rangeThresholdKey, value);
        });

        preferencesLayout->addStretch();

        auto sourceGroup = new QGroupBox{tr("Preferred source"), this};
        mainLayout->addWidget(sourceGroup);

        auto sourceLayout = new QHBoxLayout{sourceGroup};

        const auto dataSource = static_cast<PriceSettings::DataSource>(
            settings.value(PriceSettings::costDataSourceKey, static_cast<int>(PriceSettings::costDataSourceDefault)).toInt());

        mOrderSourceBtn = new QRadioButton{tr("Orders only"), this};
        sourceLayout->addWidget(mOrderSourceBtn);
        mOrderSourceBtn->setChecked(dataSource == PriceSettings::DataSource::Orders);

        mItemCostSourceBtn = new QRadioButton{tr("Prefer custom item costs (if available)"), this};
        sourceLayout->addWidget(mItemCostSourceBtn);
        mItemCostSourceBtn->setChecked(dataSource == PriceSettings::DataSource::ItemCost);

        mStationSourceBtn = new QRadioButton{tr("Custom station"), this};
        sourceLayout->addWidget(mStationSourceBtn);
        mStationSourceBtn->setChecked(dataSource == PriceSettings::DataSource::Station);

        auto stationGroup = new QGroupBox{tr("Source station"), this};
        mainLayout->addWidget(stationGroup, 1);
        stationGroup->setEnabled(dataSource == PriceSettings::DataSource::Station);

        auto stationLayout = new QVBoxLayout{stationGroup};

        mStationView = new StationView{mDataProvider, this};
        stationLayout->addWidget(mStationView);

        const auto path = settings.value(PriceSettings::costSourceStationKey).toList();
        mStationView->selectPath(path);

        connect(mStationView, &StationView::stationChanged,
                this, &MarginToolDialog::saveSelectedStation);

        connect(mOrderSourceBtn, &QRadioButton::toggled, [](bool checked) {
            if (checked)
            {
                QSettings settings;
                settings.setValue(PriceSettings::costDataSourceKey, static_cast<int>(PriceSettings::DataSource::Orders));
            }
        });
        connect(mItemCostSourceBtn, &QRadioButton::toggled, [](bool checked) {
            if (checked)
            {
                QSettings settings;
                settings.setValue(PriceSettings::costDataSourceKey, static_cast<int>(PriceSettings::DataSource::ItemCost));
            }
        });
        connect(mStationSourceBtn, &QRadioButton::toggled, [stationGroup](bool checked) {
            stationGroup->setEnabled(checked);

            if (checked)
            {
                QSettings settings;
                settings.setValue(PriceSettings::costDataSourceKey, static_cast<int>(PriceSettings::DataSource::Station));
            }
        });

        return tabWidget;
    }

    void MarginToolDialog::updateInfo(double buy, double sell, bool updatePriceEdits)
    {
        const auto priceDelta = PriceUtils::getPriceDelta();
        auto curLocale = locale();

        try
        {
            const auto character = mCharacterRepository.find(mCharacterId);
            const auto taxes = PriceUtils::calculateTaxes(*character);

            if (buy < 0. || sell < 0.)
            {
                if (buy < 0.)
                {
                    if (updatePriceEdits)
                        mBestBuyEdit->clear();

                    mCostOfSalesLabel->setText("-");

                    mBuyPrice = 0.;
                }
                else
                {
                    mBuyPrice = PriceUtils::round_to_digits(buy,4) + PriceUtils::getPriceStep(buy);

                    mBestBuyEdit->setText(curLocale.toString(mBuyPrice, 'f', 2));
                    mCostOfSalesLabel->setText(TextUtils::currencyToString(PriceUtils::getBuyPrice(mBuyPrice, taxes), curLocale));

                    if (mCopyBuyBtn->isChecked())
                        QApplication::clipboard()->setText(QString::number(mBuyPrice, 'f', 2));
                }

                if (sell < 0.)
                {
                    if (updatePriceEdits)
                        mBestSellEdit->clear();

                    mRevenueLabel->setText("-");

                    mSellPrice = 0.;
                }
                else
                {
                    mSellPrice = PriceUtils::round_to_digits(sell,4) - PriceUtils::getPriceStep(sell);

                    mBestSellEdit->setText(curLocale.toString(mSellPrice, 'f', 2));
                    mRevenueLabel->setText(TextUtils::currencyToString(PriceUtils::getSellPrice(mSellPrice, taxes), curLocale));

                    if (mCopySellBtn->isChecked())
                        QApplication::clipboard()->setText(QString::number(mSellPrice, 'f', 2));
                }

                mProfitLabel->setText("-");
                mMarginLabel->setText("-");
                mMarkupLabel->setText("-");
                mMarginLabel->setStyleSheet("color: palette(text);");
                m1SampleDataTable->clearContents();
                m5SampleDataTable->clearContents();
            }
            else
            {
                    const auto bestBuy = buy;
                    const auto bestSell = sell;

                    mSellPrice = PriceUtils::round_to_digits(bestSell,4) - PriceUtils::getPriceStep(bestSell);
                    mBuyPrice = PriceUtils::round_to_digits(bestBuy,4) + PriceUtils::getPriceStep(bestBuy);

                    const auto revenue = PriceUtils::getSellPrice(mSellPrice, taxes);
                    const auto cos = PriceUtils::getBuyPrice(mBuyPrice, taxes);
                    const auto margin = 100. * (revenue - cos) / revenue;
                    const auto markup = 100. * (revenue - cos) / cos;

                    mMarginLabel->setText(QString{"%1%2"}.arg(curLocale.toString(margin, 'f', 2)).arg(curLocale.percent()));
                    mMarkupLabel->setText(QString{"%1%2"}.arg(curLocale.toString(markup, 'f', 2)).arg(curLocale.percent()));

                    if (updatePriceEdits)
                    {
                        mBestBuyEdit->setText(curLocale.toString(mBuyPrice, 'f', 2));
                        mBestSellEdit->setText(curLocale.toString(mSellPrice, 'f', 2));
                    }

                    mProfitLabel->setText(TextUtils::currencyToString(revenue - cos, curLocale));
                    mRevenueLabel->setText(TextUtils::currencyToString(revenue, curLocale));
                    mCostOfSalesLabel->setText(TextUtils::currencyToString(cos, curLocale));

                    mMarginLabel->setStyleSheet(TextUtils::getMarginStyleSheet(margin));

                    auto clipboard = QApplication::clipboard();

                    if (mCopySellBtn->isChecked())
                        clipboard->setText(QString::number(mSellPrice, 'f', 2));
                    else if (mCopyBuyBtn->isChecked())
                        clipboard->setText(QString::number(mBuyPrice, 'f', 2));

                    fillSampleData(*m1SampleDataTable, revenue, cos, 1);
                    fillSampleData(*m5SampleDataTable, revenue, cos, 5);
            }
        }
        catch (const Repository<Character>::NotFoundException &)
        {
        }
    }

    void MarginToolDialog::setUpWatcher()
    {
        mWatcher.removePaths(mWatcher.directories());

        const auto logPath = PathUtils::getMarketLogsPath();

        qDebug() << "Using market log path:" << logPath;

        if (logPath.isEmpty())
        {
            QMessageBox::warning(this,
                                 tr("Margin tool error"),
                                 tr("Could not determine market log path. Please enter log path in settings."));
        }
        else if (!mWatcher.addPath(logPath))
        {
            QMessageBox::warning(this,
                                 tr("Margin tool error"),
                                 tr("Could not start watching market log path. Make sure the path exists (eg. export some logs) and try again."));
        }
        else
        {
            mKnownFiles = getKnownFiles(logPath);
        }
    }

    QString MarginToolDialog::getNewFile(const QString &path) const
    {
        QDirIterator files{path, { QStringLiteral("*.txt") }, QDir::Files | QDir::Readable};
        if (!files.hasNext())
            return {};

        QSettings settings;

        const QRegExp charLogWildcard{
            settings.value(PathSettings::characterLogWildcardKey, PathSettings::characterLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};
        const QRegExp corpLogWildcard{
            settings.value(PathSettings::corporationLogWildcardKey, PathSettings::corporationLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};

        QString file;
        QDateTime bestModified;

        while (files.hasNext())
        {
            files.next();

            const auto fileName = files.fileName();

            if (mKnownFiles.contains(fileName) || charLogWildcard.exactMatch(fileName) || corpLogWildcard.exactMatch(fileName))
                continue;

            const auto lastModified = files.fileInfo().lastModified();
            if (lastModified > bestModified)
            {
                file = fileName;
                bestModified = lastModified;
            }
        }

        return file;
    }

    void MarginToolDialog::fillSampleData(QTableWidget &table, double revenue, double cos, int multiplier)
    {
        const auto inserter = [&table](auto &&text, auto row, auto column) {
            std::unique_ptr<QTableWidgetItem> item{new QTableWidgetItem{std::forward<decltype(text)>(text)}};
            table.setItem(row, column, item.get());
            item.release();
        };

        const auto realMultiplier = multiplier / 1000000.;
        const auto profit = revenue - cos;

        QLocale locale;

        for (auto i = 1, row = 0; i < samples; i *= 10, ++row)
        {
            inserter(locale.toString(i * multiplier), row, 0);
            inserter(locale.toString(cos * i * realMultiplier, 'f', 2) + "M", row, 1);
            inserter(locale.toString(profit * i * realMultiplier, 'f', 2) + "M", row, 2);
        }

        table.resizeColumnsToContents();
    }

    MarginToolDialog::FileList MarginToolDialog::getKnownFiles(const QString &path)
    {
        const QDir basePath{path};
        const auto files = basePath.entryList(QStringList{"*.txt"}, QDir::Files | QDir::Readable);

        QSettings settings;

        const QRegExp charLogWildcard{
            settings.value(PathSettings::characterLogWildcardKey, PathSettings::characterLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};
        const QRegExp corpLogWildcard{
            settings.value(PathSettings::corporationLogWildcardKey, PathSettings::corporationLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};

        FileList out;
        for (const auto &file : files)
        {
            if (charLogWildcard.exactMatch(file) || corpLogWildcard.exactMatch(file))
                continue;

            out << file;
        }

        return out;
    }
}
