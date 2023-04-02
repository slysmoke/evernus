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

#include <map>
#include <set>

#include <QPropertyAnimation>
#include <QWidget>

#include "TypeSellPriceResolver.h"
#include "PriceType.h"
#include "Character.h"
#include "EveType.h"

class QDate;

namespace QtDataVisualization
{
    class QCategory3DAxis;
    class QValue3DAxis;
    class QBar3DSeries;
    class Q3DBars;
}

namespace Evernus
{
    class MiningLedgerRepository;
    class EveDataProvider;

    class MiningLedgerBarGraph
        : public QWidget
    {
        Q_OBJECT

    public:
        using OrderList = TypeSellPriceResolver::OrderList;

        MiningLedgerBarGraph(const MiningLedgerRepository &ledgerRepo,
                             const EveDataProvider &dataProvider,
                             QWidget *parent = nullptr);
        MiningLedgerBarGraph(const MiningLedgerBarGraph &) = default;
        MiningLedgerBarGraph(MiningLedgerBarGraph &&) = default;
        virtual ~MiningLedgerBarGraph() = default;

        void refresh(Character::IdType charId, const QDate &from, const QDate &to);

        void setOrders(OrderList orders);
        void setSellPriceType(PriceType type);
        void setSellStation(quint64 stationId);

        MiningLedgerBarGraph &operator =(const MiningLedgerBarGraph &) = default;
        MiningLedgerBarGraph &operator =(MiningLedgerBarGraph &&) = default;

    private slots:
        void setFontSize(int size);
        void setGridEnabled(int state);
        void setMeshSmooth(int state);
        void zoomToSelectedBar();

    private:
        struct TypeInfo
        {
            EveType::IdType mTypeId;
            quint64 mQunatity;
        };

        const MiningLedgerRepository &mLedgerRepo;
        const EveDataProvider &mDataProvider;

        QtDataVisualization::Q3DBars *mGraph = nullptr;
        QtDataVisualization::QCategory3DAxis *mTypeAxis = nullptr;
        QtDataVisualization::QCategory3DAxis *mSolarSystemAxis = nullptr;
        QtDataVisualization::QValue3DAxis *mValueAxis = nullptr;
        QtDataVisualization::QBar3DSeries *mQuantitySeries = nullptr;
        QtDataVisualization::QBar3DSeries *mProfitSeries = nullptr;

        QPropertyAnimation mCameraXAnim;
        QPropertyAnimation mCameraYAnim;
        QPropertyAnimation mCameraZoomAnim;
        QPropertyAnimation mCameraTargetAnim;

        TypeSellPriceResolver mPriceResolver;

        std::map<QString, std::map<QString, TypeInfo>, std::greater<QString>> mMappedData;
        std::set<QString> mAllTypes;

        void stopCameraAnimations();
        void refreshProfit();
    };
}
