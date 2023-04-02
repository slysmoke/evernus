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
#include <QSettings>
#include <QLocale>
#include <QColor>

#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "PriceSettings.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"
#include "TextUtils.h"

#include "LMeveTaskModel.h"

namespace Evernus
{
    LMeveTaskModel::LMeveTaskModel(const EveDataProvider &dataProvider,
                                   const ItemCostProvider &costProvider,
                                   const CharacterRepository &characterRepository,
                                   QObject *parent)
        : QAbstractTableModel{parent}
        , mDataProvider{dataProvider}
        , mCostProvider{costProvider}
        , mCharacterRepository{characterRepository}
    {
    }

    int LMeveTaskModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant LMeveTaskModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant{};

        const auto &data = mData[index.row()];
        const auto column = index.column();

        switch (role) {
        case Qt::UserRole:
            switch (column) {
            case typeColumn:
                return mDataProvider.getTypeName(data->getTypeId());
            case activityColumn:
                return data->getActivity();
            case runsColumn:
                if (data->getRuns())
                    return *data->getRuns();
                break;
            case runsDoneColumn:
                if (data->getRunsDone())
                    return *data->getRunsDone();
                break;
            case runsCompletedColumn:
                if (data->getRunsCompleted())
                    return *data->getRunsCompleted();
                break;
            case jobsDoneColumn:
                if (data->getJobsDone())
                    return *data->getJobsDone();
                break;
            case jobsSuccessColumn:
                if (data->getJobsSuccess())
                    return *data->getJobsSuccess();
                break;
            case jobsCompletedColumn:
                if (data->getJobsCompleted())
                    return *data->getJobsCompleted();
                break;
            case costColumn:
                if (mCharacter)
                    return mCostProvider.fetchForCharacterAndType(mCharacter->getId(), data->getTypeId())->getAdjustedCost();
                break;
            case priceColumn:
                return getAdjustedPrice(mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId)->getPrice());
            case profitColumn:
                if (mCharacter)
                {
                    return getAdjustedPrice(mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId)->getPrice()) -
                           mCostProvider.fetchForCharacterAndType(mCharacter->getId(), data->getTypeId())->getAdjustedCost();
                }
                return getAdjustedPrice(mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId)->getPrice());
            case marginColumn:
                return getMargin(*data);
            }
            break;
        case  Qt::DisplayRole:
            {
                QLocale locale;
                switch (column) {
                case typeColumn:
                    return mDataProvider.getTypeName(data->getTypeId());
                case activityColumn:
                    return data->getActivity();
                case runsColumn:
                    if (data->getRuns())
                        return locale.toString(*data->getRuns());
                    break;
                case runsDoneColumn:
                    if (data->getRunsDone())
                        return locale.toString(*data->getRunsDone());
                    break;
                case runsCompletedColumn:
                    if (data->getRunsCompleted())
                        return locale.toString(*data->getRunsCompleted());
                    break;
                case jobsDoneColumn:
                    if (data->getJobsDone())
                        return locale.toString(*data->getJobsDone());
                    break;
                case jobsSuccessColumn:
                    if (data->getJobsSuccess())
                        return locale.toString(*data->getJobsSuccess());
                    break;
                case jobsCompletedColumn:
                    if (data->getJobsCompleted())
                        return locale.toString(*data->getJobsCompleted());
                    break;
                case costColumn:
                    if (mCharacter)
                    {
                        const auto cost = mCostProvider.fetchForCharacterAndType(mCharacter->getId(), data->getTypeId());
                        if (!cost->isNew())
                            return TextUtils::currencyToString(cost->getAdjustedCost(), locale);
                    }
                    break;
                case priceColumn:
                    {
                        const auto price = mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId);
                        if (price->isNew())
                            return tr("unknown");

                        return TextUtils::currencyToString(getAdjustedPrice(price->getPrice()), locale);
                    }
                case profitColumn:
                    {
                        auto cost = 0.;
                        if (mCharacter)
                            cost = mCostProvider.fetchForCharacterAndType(mCharacter->getId(), data->getTypeId())->getAdjustedCost();

                        return TextUtils::currencyToString(getAdjustedPrice(mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId)->getPrice()) - cost, locale);
                    }
                case marginColumn:
                    return QString{"%1%2"}.arg(locale.toString(getMargin(*data), 'f', 2)).arg(locale.percent());
                }
            }
            break;
        case Qt::ToolTipRole:
            if (column == priceColumn)
            {
                const auto price = mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId);
                if (!price->isNew())
                {
                    QSettings settings;
                    const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                    if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                    {
                        return tr("Price data is too old (valid on %1).")
                            .arg(TextUtils::dateTimeToString(price->getUpdateTime().toLocalTime(), QLocale{}));
                    }
                }
            }
            break;
        case Qt::ForegroundRole:
            if (column == marginColumn)
            {
                const auto margin = getMargin(*data);
                return TextUtils::getMarginColor(margin);
            }
            break;
        case Qt::BackgroundRole:
            if (column == priceColumn)
            {
                const auto price = mDataProvider.getTypeStationSellPrice(data->getTypeId(), mStationId);
                if (!price->isNew())
                {
                    QSettings settings;
                    const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                    if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                        return QColor{255, 255, 192};
                }
            }
        }

        return QVariant{};
    }

    QVariant LMeveTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case typeColumn:
                return tr("Item");
            case activityColumn:
                return tr("Activity");
            case runsColumn:
                return tr("Runs");
            case runsDoneColumn:
                return tr("Runs completed and in progress");
            case runsCompletedColumn:
                return tr("Runs completed");
            case jobsDoneColumn:
                return tr("Jobs completed and in progress");
            case jobsSuccessColumn:
                return tr("Successful jobs");
            case jobsCompletedColumn:
                return tr("Jobs completed");
            case costColumn:
                return tr("Custom cost");
            case priceColumn:
                return tr("Sell price");
            case profitColumn:
                return tr("Profit");
            case marginColumn:
                return tr("Margin");
            }
        }

        return QVariant{};
    }

    int LMeveTaskModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    const LMeveTaskModel::TaskList &LMeveTaskModel::getTasks() const noexcept
    {
        return mData;
    }

    void LMeveTaskModel::setTasks(const TaskList &data)
    {
        beginResetModel();
        mData = data;
        endResetModel();
    }

    void LMeveTaskModel::setTasks(TaskList &&data)
    {
        beginResetModel();
        mData = std::move(data);
        endResetModel();
    }

    void LMeveTaskModel::setCharacterId(Character::IdType id)
    {
        try
        {
            mCharacter = mCharacterRepository.find(id);
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            mCharacter.reset();
        }
    }

    quint64 LMeveTaskModel::getStationId() const noexcept
    {
        return mStationId;
    }

    void LMeveTaskModel::setStationId(quint64 id)
    {
        beginResetModel();
        mStationId = id;
        endResetModel();
    }

    double LMeveTaskModel::getMargin(const LMeveTask &task) const
    {
        if (!mCharacter)
            return 0.;

        const auto price = mDataProvider.getTypeStationSellPrice(task.getTypeId(), mStationId);
        if (price->isNew())
            return 100.;

        const auto taxes = PriceUtils::calculateTaxes(*mCharacter);
        const auto cost = mCostProvider.fetchForCharacterAndType(mCharacter->getId(), task.getTypeId());
        return PriceUtils::getMargin(cost->getAdjustedCost(), getAdjustedPrice(price->getPrice()), taxes);
    }

    double LMeveTaskModel::getAdjustedPrice(double price)
    {
        return price - PriceUtils::getPriceDelta();
    }
}
