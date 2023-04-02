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
#pragma once

#include <QWidget>

class QKeySequenceEdit;
class QDoubleSpinBox;
class QCheckBox;
class QSpinBox;

namespace Evernus
{
    class PricePreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit PricePreferencesWidget(QWidget *parent = nullptr);
        virtual ~PricePreferencesWidget() = default;

    public slots:
        void applySettings();

    private:
        QDoubleSpinBox *mMinMarginEdit = nullptr;
        QDoubleSpinBox *mPreferredMarginEdit = nullptr;
        QDoubleSpinBox *mPriceDeltaEdit = nullptr;
        QDoubleSpinBox *mPriceDeltaRandomEdit = nullptr;
        QCheckBox *mIgnoreMinVolumeOrdersBtn = nullptr;
#ifdef Q_OS_WIN
        QCheckBox *mAltImportBtn = nullptr;
#endif
        QSpinBox *mImportLogWaitTimeEdit = nullptr;
        QCheckBox *mAutoAddCustomCostBtn = nullptr;
        QCheckBox *mShareCustomCostsBtn = nullptr;
        QSpinBox *mPriceMaxAgeEdit = nullptr;
        QCheckBox *mRefreshPricesWithOrdersBtn = nullptr;
        QCheckBox *mCopyNonOverbidBtn = nullptr;
        QCheckBox *mLimitSellCopyToCostBtn = nullptr;
        QCheckBox *mLimitSellCopyToTotalCostBtn = nullptr;
        QCheckBox *mFPCBtn = nullptr;
        QKeySequenceEdit *mFPCForwardShortcutEdit = nullptr;
        QKeySequenceEdit *mFPCBackwardShortcutEdit = nullptr;
        QCheckBox *mShowInEveOnFPCBtn = nullptr;
    };
}
