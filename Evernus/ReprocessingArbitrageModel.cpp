/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for m details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QLocale>
#include <QColor>

#include "EveDataProvider.h"
#include "TextUtils.h"

#include "ReprocessingArbitrageModel.h"

namespace Evernus
{
    ReprocessingArbitrageModel::ReprocessingArbitrageModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractTableModel{parent}
        , ModelWithTypes{}
        , mDataProvider{dataProvider}
    {
    }

    int ReprocessingArbitrageModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant ReprocessingArbitrageModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto column = index.column();
        const auto &data = mData[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case nameColumn:
                    return mDataProvider.getTypeName(data.mId);
                case volumeColumn:
                    return locale.toString(data.mVolume);
                case totalProfitColumn:
                    return TextUtils::currencyToString(data.mTotalProfit, locale);
                case totalCostColumn:
                    return TextUtils::currencyToString(data.mTotalCost, locale);
                case differenceColumn:
                    return TextUtils::currencyToString(data.mTotalProfit - data.mTotalCost, locale);
                case marginColumn:
                    return QStringLiteral("%1%2").arg(locale.toString(data.mMargin, 'f', 2)).arg(locale.percent());
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mId);
            case volumeColumn:
                return data.mVolume;
            case totalProfitColumn:
                return data.mTotalProfit;
            case totalCostColumn:
                return data.mTotalCost;
            case differenceColumn:
                return data.mTotalProfit - data.mTotalCost;
            case marginColumn:
                return data.mMargin;
            }
            break;
        case Qt::ForegroundRole:
            if (column == marginColumn)
                return TextUtils::getMarginColor(data.mMargin);
        }

        return {};
    }

    QVariant ReprocessingArbitrageModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case volumeColumn:
                return tr("Volume");
            case totalProfitColumn:
                return tr("Total profit");
            case totalCostColumn:
                return tr("Total cost");
            case differenceColumn:
                return tr("Difference");
            case marginColumn:
                return tr("Margin");
            }
        }

        return {};
    }

    int ReprocessingArbitrageModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    EveType::IdType ReprocessingArbitrageModel::getTypeId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return EveType::invalidId;

        return mData[index.row()].mId;
    }

    void ReprocessingArbitrageModel::setCharacter(std::shared_ptr<Character> character)
    {
        beginResetModel();
        mCharacter = std::move(character);
        mData.clear();
        endResetModel();
    }

    void ReprocessingArbitrageModel::reset()
    {
        beginResetModel();
        mData.clear();
        endResetModel();
    }
}
