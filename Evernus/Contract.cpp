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
#include "Contract.h"

namespace Evernus
{
    Character::IdType Contract::getIssuerId() const noexcept
    {
        return mIssuerId;
    }

    void Contract::setIssuerId(Character::IdType id) noexcept
    {
        mIssuerId = id;
    }

    quint64 Contract::getIssuerCorpId() const noexcept
    {
        return mIssuerCorpId;
    }

    void Contract::setIssuerCorpId(quint64 id) noexcept
    {
        mIssuerCorpId = id;
    }

    quint64 Contract::getAssigneeId() const noexcept
    {
        return mAssigneeId;
    }

    void Contract::setAssigneeId(quint64 id) noexcept
    {
        mAssigneeId = id;
    }

    quint64 Contract::getAcceptorId() const noexcept
    {
        return mAcceptorId;
    }

    void Contract::setAcceptorId(quint64 id) noexcept
    {
        mAcceptorId = id;
    }

    uint Contract::getStartStationId() const noexcept
    {
        return mStartStationId;
    }

    void Contract::setStartStationId(uint id) noexcept
    {
        mStartStationId = id;
    }

    uint Contract::getEndStationId() const noexcept
    {
        return mEndStationId;
    }

    void Contract::setEndStationId(uint id) noexcept
    {
        mEndStationId = id;
    }

    Contract::Type Contract::getType() const noexcept
    {
        return mType;
    }

    void Contract::setType(Type type) noexcept
    {
        mType = type;
    }

    Contract::Status Contract::getStatus() const noexcept
    {
        return mStatus;
    }

    void Contract::setStatus(Status status) noexcept
    {
        mStatus = status;
    }

    QString Contract::getTitle() const &
    {
        return mTitle;
    }

    QString &&Contract::getTitle() && noexcept
    {
        return std::move(mTitle);
    }

    void Contract::setTitle(const QString &title)
    {
        mTitle = title;
    }

    void Contract::setTitle(QString &&title)
    {
        mTitle = std::move(title);
    }

    bool Contract::isForCorp() const noexcept
    {
        return mForCorp;
    }

    void Contract::setForCorp(bool flag) noexcept
    {
        mForCorp = flag;
    }

    Contract::Availability Contract::getAvailability() const noexcept
    {
        return mAvailability;
    }

    void Contract::setAvailability(Availability value) noexcept
    {
        mAvailability = value;
    }

    QDateTime Contract::getIssued() const
    {
        return mIssued;
    }

    void Contract::setIssued(const QDateTime &dt)
    {
        mIssued = dt;
    }

    QDateTime Contract::getExpired() const
    {
        return mExpired;
    }

    void Contract::setExpired(const QDateTime &dt)
    {
        mExpired = dt;
    }

    QDateTime Contract::getAccepted() const
    {
        return mAccepted;
    }

    void Contract::setAccepted(const QDateTime &dt)
    {
        mAccepted = dt;
    }

    QDateTime Contract::getCompleted() const
    {
        return mCompleted;
    }

    void Contract::setCompleted(const QDateTime &dt)
    {
        mCompleted = dt;
    }

    int Contract::getNumDays() const noexcept
    {
        return mNumDays;
    }

    void Contract::setNumDays(int value) noexcept
    {
        mNumDays = value;
    }

    double Contract::getPrice() const noexcept
    {
        return mPrice;
    }

    void Contract::setPrice(double value) noexcept
    {
        mPrice = value;
    }

    double Contract::getReward() const noexcept
    {
        return mReward;
    }

    void Contract::setReward(double value) noexcept
    {
        mReward = value;
    }

    double Contract::getCollateral() const noexcept
    {
        return mCollateral;
    }

    void Contract::setCollateral(double value) noexcept
    {
        mCollateral = value;
    }

    double Contract::getBuyout() const noexcept
    {
        return mBuyout;
    }

    void Contract::setBuyout(double value) noexcept
    {
        mBuyout = value;
    }

    double Contract::getVolume() const noexcept
    {
        return mVolume;
    }

    void Contract::setVolume(double value) noexcept
    {
        mVolume = value;
    }

    void Contract::addItem(const std::shared_ptr<ContractItem> &item)
    {
        mItems.emplace_back(item);
    }

    void Contract::addItem(std::shared_ptr<ContractItem> &&item)
    {
        mItems.emplace_back(std::move(item));
    }

    std::shared_ptr<ContractItem> Contract::getItem(size_t index) const
    {
        return mItems[index];
    }

    size_t Contract::getItemCount() const noexcept
    {
        return mItems.size();
    }
}
