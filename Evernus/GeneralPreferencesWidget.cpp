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
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <QSqlQuery>
#include <QLabel>

#include "LanguageComboBox.h"
#include "UpdaterSettings.h"
#include "UISettings.h"
#include "DbSettings.h"

#include "GeneralPreferencesWidget.h"

namespace Evernus
{
    GeneralPreferencesWidget::GeneralPreferencesWidget(const QSqlDatabase &db, QWidget *parent)
        : QWidget{parent}
        , mDb{db}
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto languageGroup = new QGroupBox{this};
        mainLayout->addWidget(languageGroup);

        auto languageGroupLayout = new QVBoxLayout{languageGroup};

        auto languageEditLayout = new QHBoxLayout{};
        languageGroupLayout->addLayout(languageEditLayout);

        languageEditLayout->addWidget(new QLabel{tr("Language:"), this});

        mLanguageEdit = new LanguageComboBox{this};
        languageEditLayout->addWidget(mLanguageEdit);

        languageEditLayout->addStretch();

        auto languageInfoLabel = new QLabel{tr("Language changes require application restart."), this};
        languageGroupLayout->addWidget(languageInfoLabel);
        languageInfoLabel->setWordWrap(true);

        auto generalGroup = new QGroupBox{this};
        mainLayout->addWidget(generalGroup);

        auto generalGroupLayout = new QVBoxLayout{generalGroup};

        mMinimizeToTrayBtn = new QCheckBox{tr("Minimize to tray"), this};
        generalGroupLayout->addWidget(mMinimizeToTrayBtn);
        mMinimizeToTrayBtn->setChecked(settings.value(UISettings::minimizeToTrayKey, UISettings::minimizeToTrayDefault).toBool());

        mMinimizeByMarginToolBtn = new QCheckBox{tr("Minimize when opening the Margin Tool"), this};
        generalGroupLayout->addWidget(mMinimizeByMarginToolBtn);
        mMinimizeByMarginToolBtn->setChecked(settings.value(UISettings::minimizeByMarginToolKey, UISettings::minimizeByMarginToolDefault).toBool());

        mAutoUpdateBtn = new QCheckBox{tr("Check for updates on startup"), this};
        generalGroupLayout->addWidget(mAutoUpdateBtn);
        mAutoUpdateBtn->setChecked(settings.value(UpdaterSettings::autoUpdateKey, UpdaterSettings::autoUpdateDefault).toBool());

        mUsePackagedVolumeBtn = new QCheckBox{tr("Use packaged size for ships and modules"), this};
        generalGroupLayout->addWidget(mUsePackagedVolumeBtn);
        mUsePackagedVolumeBtn->setChecked(settings.value(UISettings::usePackagedVolumeKey, UISettings::usePackagedVolumeDefault).toBool());

        mOmitCurrencySymbolBtn = new QCheckBox{tr("Omit currency symbol (requires restart)"), this};
        generalGroupLayout->addWidget(mOmitCurrencySymbolBtn);
        mOmitCurrencySymbolBtn->setChecked(settings.value(UISettings::omitCurrencySymbolKey, UISettings::omitCurrencySymbolDefault).toBool());

        mUseUTCDatesBtn = new QCheckBox{tr("Force UTC date/time (requires restart)"), this};
        generalGroupLayout->addWidget(mUseUTCDatesBtn);
        mUseUTCDatesBtn->setChecked(settings.value(UISettings::useUTCDatesKey, UISettings::useUTCDatesDefault).toBool());

        auto generalFormLayout = new QFormLayout{};
        generalGroupLayout->addLayout(generalFormLayout);

        mDateFormEdit
            = new QLineEdit{settings.value(UISettings::dateTimeFormatKey, locale().dateTimeFormat(QLocale::ShortFormat)).toString(), this};
        generalFormLayout->addRow(tr("Date/time format (requires restart):"), mDateFormEdit);

        mApplyDateFormatToGraphsBtn = new QCheckBox{tr("Apply date format to graphs (requires restart)"), this};
        generalFormLayout->addRow(mApplyDateFormatToGraphsBtn);
        mApplyDateFormatToGraphsBtn->setChecked(
            settings.value(UISettings::applyDateFormatToGraphsKey, UISettings::applyDateFormatToGraphsDefault).toBool());

        mColumnDelimiterEdit = new QComboBox{this};
        generalFormLayout->addRow(tr("Column data delimiter:"), mColumnDelimiterEdit);
        mColumnDelimiterEdit->addItem(tr("Tab"), '\t');
        mColumnDelimiterEdit->addItem(tr("Space"), ' ');
        mColumnDelimiterEdit->addItem(tr(";"), ';');
        mColumnDelimiterEdit->addItem(tr(","), ',');
        mColumnDelimiterEdit->setCurrentIndex(mColumnDelimiterEdit->findData(
            settings.value(UISettings::columnDelimiterKey, UISettings::columnDelimiterDefault).value<char>()));

        mDbSynchronousEdit = new QComboBox{this};
        generalFormLayout->addRow(tr("Database synchronous flag:"), mDbSynchronousEdit);
        mDbSynchronousEdit->addItem(QStringLiteral("EXTRA"), 3);
        mDbSynchronousEdit->addItem(QStringLiteral("FULL"), 2);
        mDbSynchronousEdit->addItem(QStringLiteral("NORMAL"), 1);
        mDbSynchronousEdit->addItem(QStringLiteral("OFF"), 0);
        mDbSynchronousEdit->setToolTip(tr("Value of the \"synchronous\" flag for SQLite. Change it only when you know what it means."));
        mDbSynchronousEdit->setCurrentIndex(mDbSynchronousEdit->findData(
            settings.value(DbSettings::synchronousKey, DbSettings::synchronousDefault).toInt()));

        mainLayout->addStretch();
    }

    void GeneralPreferencesWidget::applySettings()
    {
        const auto synchronousFlag = mDbSynchronousEdit->currentData().toInt();

        mDb.exec(QStringLiteral("PRAGMA synchronous = %1").arg(synchronousFlag));

        QSettings settings;
        settings.setValue(UISettings::languageKey, mLanguageEdit->currentData(Qt::UserRole));
        settings.setValue(UISettings::minimizeToTrayKey, mMinimizeToTrayBtn->isChecked());
        settings.setValue(UISettings::minimizeByMarginToolKey, mMinimizeByMarginToolBtn->isChecked());
        settings.setValue(UpdaterSettings::autoUpdateKey, mAutoUpdateBtn->isChecked());
        settings.setValue(UISettings::usePackagedVolumeKey, mUsePackagedVolumeBtn->isChecked());
        settings.setValue(UISettings::omitCurrencySymbolKey, mOmitCurrencySymbolBtn->isChecked());
        settings.setValue(UISettings::useUTCDatesKey, mUseUTCDatesBtn->isChecked());
        settings.setValue(UISettings::dateTimeFormatKey, mDateFormEdit->text());
        settings.setValue(UISettings::applyDateFormatToGraphsKey, mApplyDateFormatToGraphsBtn->isChecked());
        settings.setValue(UISettings::columnDelimiterKey, mColumnDelimiterEdit->currentData().value<char>());
        settings.setValue(DbSettings::synchronousKey, synchronousFlag);
    }
}
