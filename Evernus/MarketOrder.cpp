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
#include "MarketOrder.h"

namespace Evernus
{
    const short MarketOrder::rangeStation;
    const short MarketOrder::rangeSystem;
    const short MarketOrder::rangeRegion;

    Character::IdType MarketOrder::getCharacterId() const
    {
        return mCharacterId;
    }

    void MarketOrder::setCharacterId(Character::IdType id)
    {
        mCharacterId = id;
    }

    quint64 MarketOrder::getStationId() const noexcept
    {
        return mStationId;
    }

    void MarketOrder::setStationId(quint64 id) noexcept
    {
        mStationId = id;
    }

    MarketOrder::CutomLocationType MarketOrder::getCustomStationId() const &
    {
        return mCustomStationId;
    }

    MarketOrder::CutomLocationType MarketOrder::getCustomStationId() && noexcept
    {
        return std::move(mCustomStationId);
    }

    void MarketOrder::setCustomStationId(const CutomLocationType &id)
    {
        mCustomStationId = id;
    }

    void MarketOrder::setCustomStationId(CutomLocationType &&id) noexcept
    {
        mCustomStationId = std::move(id);
    }

    quint64 MarketOrder::getEffectiveStationId() const noexcept
    {
        return (mCustomStationId) ? (*mCustomStationId) : (mStationId);
    }

    uint MarketOrder::getVolumeEntered() const noexcept
    {
        return mVolumeEntered;
    }

    void MarketOrder::setVolumeEntered(uint value) noexcept
    {
        mVolumeEntered = value;
    }

    uint MarketOrder::getVolumeRemaining() const noexcept
    {
        return mVolumeRemaining;
    }

    void MarketOrder::setVolumeRemaining(uint value) noexcept
    {
        mVolumeRemaining = value;
    }

    uint MarketOrder::getMinVolume() const noexcept
    {
        return mMinVolume;
    }

    void MarketOrder::setMinVolume(uint value) noexcept
    {
        mMinVolume = value;
    }

    int MarketOrder::getDelta() const noexcept
    {
        return mDelta;
    }

    void MarketOrder::setDelta(int value) noexcept
    {
        mDelta = value;
    }

    MarketOrder::State MarketOrder::getState() const noexcept
    {
        return mState;
    }

    void MarketOrder::setState(State state) noexcept
    {
        mState = state;
    }

    EveType::IdType MarketOrder::getTypeId() const
    {
        return mTypeId;
    }

    void MarketOrder::setTypeId(EveType::IdType id)
    {
        mTypeId = id;
    }

    short MarketOrder::getRange() const noexcept
    {
        return mRange;
    }

    void MarketOrder::setRange(short value) noexcept
    {
        mRange = value;
    }

    short MarketOrder::getAccountKey() const noexcept
    {
        return mAccountKey;
    }

    void MarketOrder::setAccountKey(short key) noexcept
    {
        mAccountKey = key;
    }

    short MarketOrder::getDuration() const noexcept
    {
        return mDuration;
    }

    void MarketOrder::setDuration(short value) noexcept
    {
        mDuration = value;
    }

    double MarketOrder::getEscrow() const noexcept
    {
        return mEscrow;
    }

    void MarketOrder::setEscrow(double value) noexcept
    {
        mEscrow = value;
    }

    double MarketOrder::getPrice() const noexcept
    {
        return mPrice;
    }

    void MarketOrder::setPrice(double value) noexcept
    {
        mPrice = value;
    }

    MarketOrder::Type MarketOrder::getType() const noexcept
    {
        return mType;
    }

    void MarketOrder::setType(Type type) noexcept
    {
        mType = type;
    }

    QDateTime MarketOrder::getIssued() const
    {
        return mIssued;
    }

    void MarketOrder::setIssued(const QDateTime &dt)
    {
        mIssued = dt;
    }

    QDateTime MarketOrder::getFirstSeen() const
    {
        return mFirstSeen;
    }

    void MarketOrder::setFirstSeen(const QDateTime &dt)
    {
        mFirstSeen = dt;
    }

    QDateTime MarketOrder::getLastSeen() const
    {
        return mLastSeen;
    }

    void MarketOrder::setLastSeen(const QDateTime &dt)
    {
        mLastSeen = dt;
    }

    quint64 MarketOrder::getCorporationId() const noexcept
    {
        return mCorporationId;
    }

    void MarketOrder::setCorporationId(quint64 id) noexcept
    {
        mCorporationId = id;
    }

    QString MarketOrder::getNotes() const &
    {
        return mNotes;
    }

    QString &&MarketOrder::getNotes() && noexcept
    {
        return std::move(mNotes);
    }

    void MarketOrder::setNotes(const QString &notes)
    {
        mNotes = notes;
    }

    void MarketOrder::setNotes(QString &&notes)
    {
        mNotes = std::move(notes);
    }

    QColor MarketOrder::getColorTag() const &
    {
        return mColorTag;
    }

    QColor &&MarketOrder::getColorTag() && noexcept
    {
        return std::move(mColorTag);
    }

    void MarketOrder::setColorTag(const QColor &color)
    {
        mColorTag = color;
    }

    void MarketOrder::setColorTag(QColor &&color) noexcept
    {
        mColorTag = std::move(color);
    }

    bool MarketOrder::isArchived() const
    {
        return !mLastSeen.isNull();
    }

    QString MarketOrder::stateToString(State state)
    {
        switch (state) {
        case State::Active:
            return QT_TRANSLATE_NOOP("MarketOrder", "Active");
        case State::Closed:
            return QT_TRANSLATE_NOOP("MarketOrder", "Closed");
        case State::Fulfilled:
            return QT_TRANSLATE_NOOP("MarketOrder", "Fulfilled");
        case State::Cancelled:
            return QT_TRANSLATE_NOOP("MarketOrder", "Cancelled");
        case State::Pending:
            return QT_TRANSLATE_NOOP("MarketOrder", "Pending");
        case State::CharacterDeleted:
            return QT_TRANSLATE_NOOP("MarketOrder", "Deleted");
        default:
            return QT_TRANSLATE_NOOP("MarketOrder", "Unknown");
        }
    }
}
