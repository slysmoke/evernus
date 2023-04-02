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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QSqlQuery>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QtDebug>
#include <QHash>
#include <QFont>

#include "WalletTransactionRepository.h"
#include "EveDataProvider.h"
#include "UISettings.h"
#include "TextUtils.h"

#include "qcustomplot.h"

#include "ItemHistoryWidget.h"

namespace Evernus
{
    ItemHistoryWidget::ItemHistoryWidget(const WalletTransactionRepository &walletRepo,
                                         const WalletTransactionRepository &corpWalletRepo,
                                         const EveDataProvider &dataProvider,
                                         QWidget *parent)
        : QWidget(parent)
        , mWalletRepo(walletRepo)
        , mCorpWalletRepo(corpWalletRepo)
        , mDataProvider(dataProvider)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto itemGroup = new QGroupBox{this};
        mainLayout->addWidget(itemGroup);

        auto itemLayout = new QHBoxLayout{itemGroup};

        itemLayout->addWidget(new QLabel{tr("Item type:"), this});

        mItemTypeCombo = new QComboBox{this};
        itemLayout->addWidget(mItemTypeCombo);
        mItemTypeCombo->setEditable(true);
        mItemTypeCombo->setInsertPolicy(QComboBox::NoInsert);
        mItemTypeCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        connect(mItemTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ItemHistoryWidget::computeValues);

        mAllCharactersBtn = new QCheckBox{tr("Combine for all characters"), this};
        itemLayout->addWidget(mAllCharactersBtn);
        connect(mAllCharactersBtn, &QCheckBox::toggled, this, &ItemHistoryWidget::updateData);

        itemLayout->addStretch();

        QSharedPointer<QCPAxisTickerDateTime> xTicker{new QCPAxisTickerDateTime{}};
        xTicker->setDateTimeFormat(locale().dateFormat(QLocale::NarrowFormat));

        mPlot = new QCustomPlot{this};
        mainLayout->addWidget(mPlot);
        mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        mPlot->setMinimumHeight(300);
        mPlot->xAxis->setTickLabelRotation(60);
        mPlot->xAxis->setTicker(std::move(xTicker));
        mPlot->xAxis->grid()->setVisible(false);
        mPlot->yAxis->setNumberPrecision(2);
        mPlot->yAxis->setLabel("ISK");
        mPlot->yAxis2->setVisible(true);
        mPlot->yAxis2->setLabel(tr("Volume"));
        mPlot->legend->setVisible(true);

        auto locale = mPlot->locale();
        locale.setNumberOptions(0);
        mPlot->setLocale(locale);

        QSettings settings;
        mPlot->yAxis->setNumberFormat(
            settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString());

        mVolumeGraph = new QCPBars{mPlot->xAxis, mPlot->yAxis2};

        mVolumeGraph->setName(tr("Volume"));
        mVolumeGraph->setPen(QPen{Qt::cyan});
        mVolumeGraph->setBrush(Qt::cyan);
        mVolumeGraph->setWidth(3600 * 3);

        mBalanceGraph = mPlot->addGraph();
        mBalanceGraph->setName(tr("Balance"));
        mBalanceGraph->setPen(QPen{Qt::darkGreen});

        auto totalGroup = new QGroupBox{this};
        mainLayout->addWidget(totalGroup);

        auto totalLayout = new QHBoxLayout{totalGroup};

        QFont font;
        font.setBold(true);

        totalLayout->addWidget(new QLabel{tr("Total income:"), this}, 0, Qt::AlignRight);

        mTotalIncomeLabel = new QLabel{"-", this};
        totalLayout->addWidget(mTotalIncomeLabel, 0, Qt::AlignLeft);
        mTotalIncomeLabel->setFont(font);

        totalLayout->addWidget(new QLabel{tr("Total cost:"), this}, 0, Qt::AlignRight);

        mTotalOutcomeLabel = new QLabel{"-", this};
        totalLayout->addWidget(mTotalOutcomeLabel, 0, Qt::AlignLeft);
        mTotalOutcomeLabel->setFont(font);

        totalLayout->addWidget(new QLabel{tr("Balance:"), this}, 0, Qt::AlignRight);

        mTotalBalanceLabel = new QLabel{"-", this};
        totalLayout->addWidget(mTotalBalanceLabel, 0, Qt::AlignLeft);
        mTotalBalanceLabel->setFont(font);

        totalLayout->addWidget(new QLabel{tr("Margin:"), this}, 0, Qt::AlignRight);

        mTotalMarginLabel = new QLabel{"-", this};
        totalLayout->addWidget(mTotalMarginLabel, 0, Qt::AlignLeft);
        mTotalMarginLabel->setFont(font);

        totalLayout->addWidget(new QLabel{tr("Total volume:"), this}, 0, Qt::AlignRight);

        mTotalVolumeLabel = new QLabel{"-", this};
        totalLayout->addWidget(mTotalVolumeLabel, 0, Qt::AlignLeft);
        mTotalVolumeLabel->setFont(font);

        mainLayout->addStretch();
    }

    void ItemHistoryWidget::setCharacter(Character::IdType id)
    {
        qDebug() << "Switching to character" << id;

        mCharacterId = id;
        if (!mAllCharactersBtn->isChecked())
            updateData();
    }

    void ItemHistoryWidget::updateData()
    {
        QSqlQuery query{mWalletRepo.getDatabase()};

        if (mAllCharactersBtn->isChecked())
        {
            query.prepare(QString{"SELECT DISTINCT ids.type_id FROM ("
                "SELECT type_id FROM %1 "
                "UNION "
                "SELECT type_id FROM %2"
            ") ids"}.arg(mWalletRepo.getTableName()).arg(mCorpWalletRepo.getTableName()));
        }
        else
        {
            query.prepare(QString{"SELECT DISTINCT ids.type_id FROM ("
                "SELECT type_id, character_id FROM %1 "
                "UNION "
                "SELECT type_id, character_id FROM %2"
            ") ids WHERE ids.character_id = ?"}.arg(mWalletRepo.getTableName()).arg(mCorpWalletRepo.getTableName()));
            query.bindValue(0, mCharacterId);
        }

        DatabaseUtils::execQuery(query);

        mItemTypeCombo->clear();
        while (query.next())
        {
            const auto id = query.value(0).value<EveType::IdType>();
            mItemTypeCombo->addItem(mDataProvider.getTypeName(id), id);
        }

        mItemTypeCombo->model()->sort(0);
    }

    void ItemHistoryWidget::handleNewPreferences()
    {
        QSettings settings;
        mPlot->yAxis->setNumberFormat(
            settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString());

        mPlot->replot();

        setMarginColor();
    }

    void ItemHistoryWidget::computeValues()
    {
        const auto id = mItemTypeCombo->currentData().value<EveType::IdType>();

        QHash<QDate, std::pair<double, uint>> values;
        uint totalVolume = 0;

        mTotalIncome = mTotalOutcome = 0.;

        const auto inserter = [&values, &totalVolume, this](const auto &entries) {
            for (const auto &entry : entries)
            {
                if (entry->isIgnored())
                    continue;

                auto &value = values[entry->getTimestamp().toLocalTime().date()];

                const auto volume = entry->getQuantity();
                const auto amount = entry->getPrice() * volume;

                if (entry->getType() == Evernus::WalletTransaction::Type::Buy)
                {
                    value.first -= amount;
                    mTotalOutcome += amount;
                }
                else
                {
                    value.first += amount;
                    mTotalIncome += amount;
                }

                value.second += volume;
                totalVolume += volume;
            }
        };

        if (mAllCharactersBtn->isChecked())
        {
            inserter(mWalletRepo.fetchForTypeId(id));
            inserter(mCorpWalletRepo.fetchForTypeId(id));
        }
        else
        {
            inserter(mWalletRepo.fetchForTypeIdAndCharacter(id, mCharacterId));
            inserter(mCorpWalletRepo.fetchForTypeIdAndCharacter(id, mCharacterId));
        }

        QVector<double> ticks, balance, volume;

        QHashIterator<QDate, std::pair<double, uint>> it{values};
        while (it.hasNext())
        {
            it.next();

            ticks << QDateTime{it.key()}.toMSecsSinceEpoch() / 1000.;
            balance << it.value().first;
            volume << it.value().second;
        }

        mBalanceGraph->setData(ticks, balance);
        mVolumeGraph->setData(ticks, volume);

        mPlot->rescaleAxes();
        mPlot->replot();

        const auto curLocale = locale();

        mTotalIncomeLabel->setText(TextUtils::currencyToString(mTotalIncome, curLocale));
        mTotalOutcomeLabel->setText(TextUtils::currencyToString(mTotalOutcome, curLocale));
        mTotalBalanceLabel->setText(TextUtils::currencyToString(mTotalIncome - mTotalOutcome, curLocale));
        mTotalMarginLabel->setText(QString{"%1%2"}.arg(curLocale.toString(getMargin(), 'f', 2)).arg(curLocale.percent()));
        mTotalVolumeLabel->setText(curLocale.toString(totalVolume));

        setMarginColor();
    }

    void ItemHistoryWidget::setMarginColor()
    {
        mTotalMarginLabel->setStyleSheet(TextUtils::getMarginStyleSheet(getMargin()));
    }

    double ItemHistoryWidget::getMargin() const noexcept
    {
        return (qFuzzyIsNull(mTotalIncome)) ? (0.) : (100. * (mTotalIncome - mTotalOutcome) / mTotalIncome);
    }
}
