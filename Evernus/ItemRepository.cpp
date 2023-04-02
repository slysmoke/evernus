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
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <QStringList>
#include <QSqlRecord>
#include <QSqlQuery>

#include "AssetList.h"

#include "ItemRepository.h"

namespace Evernus
{
    ItemRepository::ItemRepository(bool corp, const DatabaseConnectionProvider &connectionProvider)
        : Repository{connectionProvider}
        , mCorp{corp}
    {
    }

    QString ItemRepository::getTableName() const
    {
        return (mCorp) ? (QStringLiteral("corp_items")) : (QStringLiteral("items"));
    }

    QString ItemRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    ItemRepository::EntityPtr ItemRepository::populate(const QSqlRecord &record) const
    {
        const auto locationId = record.value(QStringLiteral("location_id"));
        const auto parentId = record.value(QStringLiteral("parent_id"));
        const auto customValue = record.value(QStringLiteral("custom_value"));
        const auto bpc = record.value(QStringLiteral("bpc"));

        auto item = std::make_shared<Item>(record.value(QStringLiteral("id")).value<Item::IdType>());
        item->setListId(record.value(QStringLiteral("asset_list_id")).value<AssetList::IdType>());
        item->setParentId((parentId.isNull()) ? (Item::ParentIdType{}) : (parentId.value<Item::IdType>()));
        item->setTypeId(record.value(QStringLiteral("type_id")).toUInt());
        item->setLocationId((locationId.isNull()) ? (ItemData::LocationIdType{}) : (locationId.value<ItemData::LocationIdType::value_type>()));
        item->setQuantity(record.value(QStringLiteral("quantity")).toUInt());
        item->setRawQuantity(record.value(QStringLiteral("raw_quantity")).toInt());
        item->setCustomValue((customValue.isNull()) ? (Item::CustomValueType{}) : (customValue.value<Item::CustomValueType::value_type>()));
        item->setBPCFlag((bpc.isNull()) ? (Item::BPCType{}) : (bpc.value<Item::BPCType::value_type>()));
        item->setNew(false);

        return item;
    }

    void ItemRepository::create(const Repository<AssetList> &assetRepo) const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "asset_list_id INTEGER NOT NULL REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE,"
            "parent_id BIGINT NULL REFERENCES %1(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "type_id INTEGER NOT NULL,"
            "location_id BIGINT NULL,"
            "quantity INTEGER NOT NULL,"
            "raw_quantity INTEGER NOT NULL,"
            "custom_value NUMERIC NULL,"
            "bpc TINYINT NULL"
        ")").arg(getTableName()).arg(assetRepo.getTableName()).arg(assetRepo.getIdColumn()));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(asset_list_id)").arg(getTableName()).arg(assetRepo.getTableName()));
    }

    void ItemRepository::batchStore(const PropertyMap &map) const
    {
        if (map.empty())
            return;

        const auto maxRowsPerInsert = 100;
        const auto totalRows = std::begin(map)->size();
        const auto batches = totalRows / maxRowsPerInsert;

        const auto columns = getColumns();
        const auto baseQueryStr = QStringLiteral("REPLACE INTO %1 (%2) VALUES %3").arg(getTableName()).arg(columns.join(QStringLiteral(", ")));

        QStringList columnBindings;
        for (auto i = 0; i < columns.count(); ++i)
            columnBindings << QStringLiteral("?");

        const auto bindings = QStringLiteral("(%1)").arg(columnBindings.join(QStringLiteral(", ")));

        QStringList batchBindings;
        for (auto i = 0; i < maxRowsPerInsert; ++i)
            batchBindings << bindings;

        const auto batchQueryStr = baseQueryStr.arg(batchBindings.join(QStringLiteral(", ")));

        for (auto batch = 0; batch < batches; ++batch)
        {
            auto query = prepare(batchQueryStr);

            for (auto row = batch * maxRowsPerInsert; row < (batch + 1) * maxRowsPerInsert; ++row)
            {
                for (const auto &column : columns)
                    query.addBindValue(map[column][row]);
            }

            DatabaseUtils::execQuery(query);
        }

        if ((totalRows % maxRowsPerInsert) != 0)
        {
            QStringList restBindings;
            for (auto i = 0; i < totalRows % maxRowsPerInsert; ++i)
                restBindings << bindings;

            const auto restQueryStr = baseQueryStr.arg(restBindings.join(QStringLiteral(", ")));
            auto query = prepare(restQueryStr);

            for (auto row = batches * maxRowsPerInsert; row < totalRows; ++row)
            {
                for (const auto &column : columns)
                    query.addBindValue(map[column][row]);
            }

            DatabaseUtils::execQuery(query);
        }
    }

    void ItemRepository::fillCustomValues(AssetList &assets) const
    {
        QStringList ids;
        std::unordered_map<Item::IdType, std::reference_wrapper<Item>> items;

        const std::function<void (Item &)> fillItems = [&](Item &item) {
            ids << QString::number(item.getId());
            items.emplace(item.getId(), std::ref(item));

            for (const auto &child : item)
                fillItems(*child);
        };

        for (const auto &item : assets)
            fillItems(*item);

        const auto sql = QStringLiteral("SELECT id, custom_value FROM %1 WHERE %2 IN (%3)")
            .arg(getTableName())
            .arg(getIdColumn())
            .arg(ids.join(QStringLiteral(", ")));

        auto query = exec(sql);
        while (query.next())
        {
            const auto item = items.find(query.value(0).value<Item::IdType>());
            Q_ASSERT(item != std::end(items));

            const auto value = query.value(1);
            if (!value.isNull())
                item->second.get().setCustomValue(value.toDouble());
        }
    }

    void ItemRepository::setCustomValue(Item::IdType id, double value) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET custom_value = ? WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn())
        );
        query.addBindValue(id);
        query.addBindValue(value);

        DatabaseUtils::execQuery(query);
    }

    void ItemRepository::setBPC(const std::vector<Item::IdType> &ids, bool value) const
    {
        if (ids.empty())
            return;

        QStringList placeholders;
        std::fill_n(std::back_inserter(placeholders), ids.size(), QStringLiteral("?"));

        auto query = prepare(QStringLiteral("UPDATE %1 SET bpc = ? WHERE %2 IN (%3)")
             .arg(getTableName())
             .arg(getIdColumn())
             .arg(placeholders.join(QStringLiteral(", ")))
         );
        query.addBindValue(value);

        for (const auto id : ids)
            query.addBindValue(id);

        DatabaseUtils::execQuery(query);
    }

    void ItemRepository::clearCustomValue(Item::IdType id) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET custom_value = NULL WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn())
        );
        query.addBindValue(id);

        DatabaseUtils::execQuery(query);
    }

    void ItemRepository::fillProperties(const Item &entity, PropertyMap &map)
    {
        const auto locationId = entity.getLocationId();
        const auto parentId = entity.getParentId();
        const auto customValue = entity.getCustomValue();
        const auto bpc = entity.getBPCFlag();

        map[QStringLiteral("id")] << entity.getId();
        map[QStringLiteral("asset_list_id")] << entity.getListId();
        map[QStringLiteral("parent_id")] << ((parentId) ? (*parentId) : (QVariant{QVariant::ULongLong}));
        map[QStringLiteral("type_id")] << entity.getTypeId();
        map[QStringLiteral("location_id")] << ((locationId) ? (*locationId) : (QVariant{QVariant::ULongLong}));
        map[QStringLiteral("quantity")] << entity.getQuantity();
        map[QStringLiteral("raw_quantity")] << entity.getRawQuantity();
        map[QStringLiteral("custom_value")] << ((customValue) ? (*customValue) : (QVariant{QVariant::Double}));
        map[QStringLiteral("bpc")] << ((bpc) ? (*bpc) : (QVariant{QVariant::Bool}));

        for (const auto &item : entity)
            fillProperties(*item, map);
    }

    QStringList ItemRepository::getColumns() const
    {
        return {
            QStringLiteral("id"),
            QStringLiteral("asset_list_id"),
            QStringLiteral("parent_id"),
            QStringLiteral("type_id"),
            QStringLiteral("location_id"),
            QStringLiteral("quantity"),
            QStringLiteral("raw_quantity"),
            QStringLiteral("custom_value"),
            QStringLiteral("bpc"),
        };
    }

    void ItemRepository::bindValues(const Item &entity, QSqlQuery &query) const
    {
        const auto locationId = entity.getLocationId();
        const auto parentId = entity.getParentId();
        const auto customValue = entity.getCustomValue();
        const auto bpc = entity.getBPCFlag();

        if (entity.getId() != Item::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":asset_list_id"), entity.getListId());
        query.bindValue(QStringLiteral(":parent_id"), (parentId) ? (*parentId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
        query.bindValue(QStringLiteral(":location_id"), (locationId) ? (*locationId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":quantity"), entity.getQuantity());
        query.bindValue(QStringLiteral(":raw_quantity"), entity.getRawQuantity());
        query.bindValue(QStringLiteral(":custom_value"), (customValue) ? (*customValue) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":bpc"), (bpc) ? (*bpc) : (QVariant{QVariant::Bool}));
    }

    void ItemRepository::bindPositionalValues(const Item &entity, QSqlQuery &query) const
    {
        const auto locationId = entity.getLocationId();
        const auto parentId = entity.getParentId();
        const auto customValue = entity.getCustomValue();
        const auto bpc = entity.getBPCFlag();

        if (entity.getId() != Item::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getListId());
        query.addBindValue((parentId) ? (*parentId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue(entity.getTypeId());
        query.addBindValue((locationId) ? (*locationId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue(entity.getQuantity());
        query.addBindValue(entity.getRawQuantity());
        query.addBindValue((customValue) ? (*customValue) : (QVariant{QVariant::Double}));
        query.addBindValue((bpc) ? (*bpc) : (QVariant{QVariant::Bool}));
    }
}
