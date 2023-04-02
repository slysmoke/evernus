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

#include <vector>
#include <memory>

#include <QDateTime>

#include "ContractItem.h"
#include "Character.h"

namespace Evernus
{
    class Contract
        : public Entity<quint64>
    {
    public:
        typedef std::vector<std::shared_ptr<ContractItem>> ItemList;

        enum class Type
        {
            Unknown,
            ItemExchange,
            Courier,
            Auction,
            Loan
        };

        enum class Status
        {
            Outstanding,
            Deleted,
            Completed,
            Failed,
            CompletedByIssuer,
            CompletedByContractor,
            Cancelled,
            Rejected,
            Reversed,
            InProgress
        };

        enum class Availability
        {
            Public,
            Private,
            Corporation,
            Alliance,
        };

        using Entity::Entity;

        Contract() = default;
        Contract(const Contract &) = default;
        Contract(Contract &&) = default;
        virtual ~Contract() = default;

        Character::IdType getIssuerId() const noexcept;
        void setIssuerId(Character::IdType id) noexcept;

        quint64 getIssuerCorpId() const noexcept;
        void setIssuerCorpId(quint64 id) noexcept;

        quint64 getAssigneeId() const noexcept;
        void setAssigneeId(quint64 id) noexcept;

        quint64 getAcceptorId() const noexcept;
        void setAcceptorId(quint64 id) noexcept;

        uint getStartStationId() const noexcept;
        void setStartStationId(uint id) noexcept;

        uint getEndStationId() const noexcept;
        void setEndStationId(uint id) noexcept;

        Type getType() const noexcept;
        void setType(Type type) noexcept;

        Status getStatus() const noexcept;
        void setStatus(Status status) noexcept;

        QString getTitle() const &;
        QString &&getTitle() && noexcept;
        void setTitle(const QString &title);
        void setTitle(QString &&title);

        bool isForCorp() const noexcept;
        void setForCorp(bool flag) noexcept;

        Availability getAvailability() const noexcept;
        void setAvailability(Availability value) noexcept;

        QDateTime getIssued() const;
        void setIssued(const QDateTime &dt);

        QDateTime getExpired() const;
        void setExpired(const QDateTime &dt);

        QDateTime getAccepted() const;
        void setAccepted(const QDateTime &dt);

        QDateTime getCompleted() const;
        void setCompleted(const QDateTime &dt);

        int getNumDays() const noexcept;
        void setNumDays(int value) noexcept;

        double getPrice() const noexcept;
        void setPrice(double value) noexcept;

        double getReward() const noexcept;
        void setReward(double value) noexcept;

        double getCollateral() const noexcept;
        void setCollateral(double value) noexcept;

        double getBuyout() const noexcept;
        void setBuyout(double value) noexcept;

        double getVolume() const noexcept;
        void setVolume(double value) noexcept;

        void addItem(const std::shared_ptr<ContractItem> &item);
        void addItem(std::shared_ptr<ContractItem> &&item);
        std::shared_ptr<ContractItem> getItem(size_t index) const;

        size_t getItemCount() const noexcept;

        Contract &operator =(const Contract &) = default;
        Contract &operator =(Contract &&) = default;

    private:
        Character::IdType mIssuerId = Character::invalidId;
        quint64 mIssuerCorpId = 0;
        quint64 mAssigneeId = 0;
        quint64 mAcceptorId = 0;
        uint mStartStationId = 0;
        uint mEndStationId = 0;
        Type mType = Type::ItemExchange;
        Status mStatus = Status::Outstanding;
        QString mTitle;
        bool mForCorp = false;
        Availability mAvailability = Availability::Private;
        QDateTime mIssued;
        QDateTime mExpired;
        QDateTime mAccepted;
        QDateTime mCompleted;
        int mNumDays = 0;
        double mPrice = 0.;
        double mReward = 0.;
        double mCollateral = 0.;
        double mBuyout = 0.;
        double mVolume = 0.;
        ItemList mItems;
    };
}
