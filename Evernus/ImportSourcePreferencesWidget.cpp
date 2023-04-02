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
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QSettings>

#include "ImportSettings.h"

#include "ImportSourcePreferencesWidget.h"

namespace Evernus
{
    ImportSourcePreferencesWidget::ImportSourcePreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto sourceGroup = new QGroupBox{tr("Default import sources"), this};
        mainLayout->addWidget(sourceGroup);

        auto sourceGroupLayout = new QFormLayout{sourceGroup};

        QSettings settings;

        mPriceSourceCombo = new QComboBox{this};
        sourceGroupLayout->addRow(tr("Prices:"), mPriceSourceCombo);

        const auto priceSource = static_cast<ImportSettings::PriceImportSource>(
            settings.value(ImportSettings::priceImportSourceKey, static_cast<int>(ImportSettings::priceImportSourceDefault)).toInt());

        addSourceItem(*mPriceSourceCombo, tr("Web"), ImportSettings::PriceImportSource::Web, priceSource);
        addSourceItem(*mPriceSourceCombo, tr("Logs"), ImportSettings::PriceImportSource::Logs, priceSource);

        mMarketOrderSourceCombo = new QComboBox{this};
        sourceGroupLayout->addRow(tr("Market orders:"), mMarketOrderSourceCombo);

        const auto marketOrderSource = static_cast<ImportSettings::MarketOrderImportSource>(
            settings.value(ImportSettings::marketOrderImportSourceKey, static_cast<int>(ImportSettings::marketOrderImportSourceDefault)).toInt());

        addSourceItem(*mMarketOrderSourceCombo, tr("API"), ImportSettings::MarketOrderImportSource::API, marketOrderSource);
        addSourceItem(*mMarketOrderSourceCombo, tr("Logs"), ImportSettings::MarketOrderImportSource::Logs, marketOrderSource);

        mMarketOrderImportTypeCombo = new QComboBox{this};
        sourceGroupLayout->addRow(tr("Market order import type:"), mMarketOrderImportTypeCombo);

        const auto marketOrderImportType = static_cast<ImportSettings::MarketOrderImportType>(
            settings.value(ImportSettings::marketOrderImportTypeKey, static_cast<int>(ImportSettings::marketOrderImportTypeDefault)).toInt());

        addSourceItem(*mMarketOrderImportTypeCombo, tr("Auto"), ImportSettings::MarketOrderImportType::Auto, marketOrderImportType);
        addSourceItem(*mMarketOrderImportTypeCombo, tr("Individual"), ImportSettings::MarketOrderImportType::Individual, marketOrderImportType);
        addSourceItem(*mMarketOrderImportTypeCombo, tr("Whole"), ImportSettings::MarketOrderImportType::Whole, marketOrderImportType);

        mainLayout->addStretch();
    }

    void ImportSourcePreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::priceImportSourceKey, mPriceSourceCombo->currentData());
        settings.setValue(ImportSettings::marketOrderImportSourceKey, mMarketOrderSourceCombo->currentData());
        settings.setValue(ImportSettings::marketOrderImportTypeKey, mMarketOrderImportTypeCombo->currentData());
    }

    template<class T>
    void ImportSourcePreferencesWidget::addSourceItem(QComboBox &combo,const QString &text, T value, T current)
    {
        combo.addItem(text, static_cast<int>(value));
        if (value == current)
            combo.setCurrentIndex(combo.count() - 1);
    }
}
