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

#include <unordered_set>
#include <vector>

#include <QFileSystemWatcher>
#include <QDateTime>
#include <QDialog>
#include <QSet>

#include "ExternalOrder.h"
#include "Character.h"

class QTableWidget;
class QRadioButton;
class QLineEdit;
class QSpinBox;
class QLabel;

namespace Evernus
{
    template<class T>
    class Repository;
    class ItemCostProvider;
    class EveDataProvider;
    class ExternalOrder;
    class StationView;

    class MarginToolDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        MarginToolDialog(const Repository<Character> &characterRepository,
                         const ItemCostProvider &itemCostProvider,
                         EveDataProvider &dataProvider,
                         QWidget *parent = nullptr);
        virtual ~MarginToolDialog() = default;

    signals:
        void hidden();

        void quit();

    public slots:
        void setCharacter(Character::IdType id);

        void copyBuyPrice();
        void copySellPrice();

        void handleNewPreferences();

    private slots:
        void toggleAlwaysOnTop(int state);

        void refreshData(const QString &path);
        void refreshDataByEdits();

        void saveCopyMode();
        void saveSelectedStation(quint64 id);

    protected:
        virtual void closeEvent(QCloseEvent *event) override;

    private:
        using FileList = QSet<QString>;

        static const auto samples = 100000000;

        static const QString settingsGeometryKey;

        const Repository<Character> &mCharacterRepository;
        const ItemCostProvider &mItemCostProvider;
        EveDataProvider &mDataProvider;

        QFileSystemWatcher mWatcher;

        QLabel *mNameLabel = nullptr;
        QLineEdit *mBestBuyEdit = nullptr;
        QLineEdit *mBestSellEdit = nullptr;
        QLabel *mProfitLabel = nullptr;
        QLabel *mRevenueLabel = nullptr;
        QLabel *mCostOfSalesLabel = nullptr;
        QLabel *mBuyoutLabel = nullptr;
        QLabel *mMarginLabel = nullptr;
        QLabel *mMarkupLabel = nullptr;
        QLabel *mBrokerFeeLabel = nullptr;
        QLabel *mSalesTaxLabel = nullptr;
        QLabel *mBuyVolLabel = nullptr;
        QLabel *mSellVolLabel = nullptr;
        QLabel *mBuyOrdersLabel = nullptr;
        QLabel *mSellOrdersLabel = nullptr;

        QRadioButton *mDontCopyBtn = nullptr;
        QRadioButton *mCopySellBtn = nullptr;
        QRadioButton *mCopyBuyBtn = nullptr;

        QTableWidget *m1SampleDataTable = nullptr;
        QTableWidget *m5SampleDataTable = nullptr;

        QSpinBox *mRangeThresholdEdit = nullptr;

        QRadioButton *mOrderSourceBtn = nullptr;
        QRadioButton *mItemCostSourceBtn = nullptr;
        QRadioButton *mStationSourceBtn = nullptr;

        StationView *mStationView = nullptr;

        FileList mKnownFiles;

        Character::IdType mCharacterId = Character::invalidId;

        double mBuyPrice = 0., mSellPrice = 0.;

        void setNewWindowFlags(bool alwaysOnTop);
        QTableWidget *createSampleTable();
        void savePosition() const;

        QWidget *createMarginDataTab();
        QWidget *createDataSourceTab();

        void updateInfo(double buy, double sell, bool updatePriceEdits);

        void setUpWatcher();

        QString getNewFile(const QString &path) const;

        static void fillSampleData(QTableWidget &table, double revenue, double cos, int multiplier);

        static FileList getKnownFiles(const QString &path);
    };
}
