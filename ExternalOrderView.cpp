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
#include <algorithm>

#include <QItemSelectionModel>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QClipboard>
#include <QLocale>
#include <QAction>
#include <QLabel>
#include <QFont>

#include <boost/range/adaptor/transformed.hpp>

#include "MarketOrderVolumeItemDelegate.h"
#include "ExternalOrderModel.h"
#include "ItemCostProvider.h"
#include "StyledTreeView.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"
#include "TextUtils.h"

#include "ExternalOrderView.h"

namespace Evernus
{
	ExternalOrderView::ExternalOrderView(const ItemCostProvider &costProvider,
		const EveDataProvider &dataProvider,
		const QString &objectName,
		QWidget *parent)
		: QWidget{ parent }
		, mCostProvider{ costProvider }
		, mDataProvider{ dataProvider }
		, mProxy{ mDataProvider }
	{
		auto mainLayout = new QVBoxLayout{ this };

		mProxy.setSortRole(Qt::UserRole);
		connect(&mProxy, &QSortFilterProxyModel::modelReset, this, &ExternalOrderView::handleModelReset, Qt::QueuedConnection);

		mView = new StyledTreeView{ objectName, this };
		mainLayout->addWidget(mView, 1);
		mView->setModel(&mProxy);
		mView->setRootIsDecorated(false);
		connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &ExternalOrderView::selectOrder);

		mCopySuggestedPriceAct = new QAction{ getDefaultCopySuggestedPriceText(), this };
		mCopySuggestedPriceAct->setEnabled(false);
		connect(mCopySuggestedPriceAct, &QAction::triggered, this, &ExternalOrderView::copySuggestedPrice);

		mView->addAction(mCopySuggestedPriceAct);

		auto infoLayout = new QHBoxLayout{};
		mainLayout->addLayout(infoLayout);

		QFont font;
		font.setBold(true);

		infoLayout->addWidget(new QLabel{ tr("Total cost:"), this });

		mTotalPriceLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mTotalPriceLabel);
		mTotalPriceLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Total volume:"), this });

		mTotalVolumeLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mTotalVolumeLabel);
		mTotalVolumeLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Total size:"), this });

		mTotalSizeLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mTotalSizeLabel);
		mTotalSizeLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Min. price:"), this });

		mMinPriceLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mMinPriceLabel);
		mMinPriceLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Avg. price:"), this });

		mMedianPriceLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mMedianPriceLabel);
		mMedianPriceLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Max. price:"), this });

		mMaxPriceLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mMaxPriceLabel);
		mMaxPriceLabel->setFont(font);

		infoLayout->addWidget(new QLabel{ tr("Custom cost:"), this });

		mItemCostLabel = new QLabel{ "-", this };
		infoLayout->addWidget(mItemCostLabel);
		mItemCostLabel->setFont(font);

		infoLayout->addStretch();
	}

	void ExternalOrderView::setModel(ExternalOrderModel *model)
	{
		if (mSource == model)
			return;

		mSource = model;
		mProxy.setSourceModel(mSource);
		mView->restoreHeaderState();

		if (mSource != nullptr)
		{
			sortByPrice();
			mView->setItemDelegateForColumn(mSource->getVolumeColumn(), new MarketOrderVolumeItemDelegate{ this });
		}
	}

	void ExternalOrderView::setCharacterId(Character::IdType id)
	{
		mCharacterId = id;
		setCustomCost();
	}

	void ExternalOrderView::setTypeId(EveType::IdType id)
	{
		mTypeId = id;
		setCustomCost();
	}

	void ExternalOrderView
		::setFilter(double minPrice, double maxPrice, uint minVolume, uint maxVolume, ExternalOrderFilterProxyModel::SecurityStatuses security)
	{
		mProxy.setFilter(minPrice, maxPrice, minVolume, maxVolume, security);
	}

	void ExternalOrderView::sortByPrice()
	{
		mView->sortByColumn(mSource->getPriceColumn(), mSource->getPriceSortOrder());
	}

	void ExternalOrderView::addTreeViewAction(QAction *action)
	{
		mView->addAction(action);
	}

	QModelIndex ExternalOrderView::currentIndex() const
	{
		return mProxy.mapToSource(mView->currentIndex());
	}

	void ExternalOrderView::handleModelReset()
	{
		setDefaultTotals();
		mView->header()->resizeSections(QHeaderView::ResizeToContents);
	}

	void ExternalOrderView::selectOrder()
	{
		const auto selected = mView->selectionModel()->selection();
		if (selected.isEmpty() || mSource == nullptr || mSource->getGrouping() != ExternalOrderModel::Grouping::None)
		{
			mCurrentOrder = QModelIndex{};

			mCopySuggestedPriceAct->setText(getDefaultCopySuggestedPriceText());
			mCopySuggestedPriceAct->setEnabled(false);

			setDefaultTotals();
		}
		else
		{
			auto indexes = selected.indexes();

			mCurrentOrder = mProxy.mapToSource(indexes.first());

			const auto price = getSuggestedPrice();
			mCopySuggestedPriceAct->setText(tr("Copy suggested price: %1").arg(TextUtils::currencyToString(price, locale())));

			mCopySuggestedPriceAct->setEnabled(true);

			const auto indexEqual = [](const auto &a, const auto &b) {
				return a.row() == b.row();
			};

			indexes.erase(std::unique(std::begin(indexes), std::end(indexes), indexEqual), std::end(indexes));

			const auto getOrder = [=](const auto &index) {
				return &mSource->getOrder(mProxy.mapToSource(index).row());
			};

			setAggregatedTotals(MathUtils::calcAggregates(indexes | boost::adaptors::transformed(getOrder), mDataProvider));
		}
	}

	void ExternalOrderView::copySuggestedPrice() const
	{
		const auto price = getSuggestedPrice();
		QApplication::clipboard()->setText(QString::number(price, 'f', 2));
	}

	void ExternalOrderView::setCustomCost()
	{
		if (mCharacterId == Character::invalidId || mTypeId == EveType::invalidId)
		{
			mItemCostLabel->setText(tr("N/A"));
		}
		else
		{
			const auto cost = mCostProvider.fetchForCharacterAndType(mCharacterId, mTypeId);
			if (cost->isNew())
				mItemCostLabel->setText(tr("N/A"));
			else
				mItemCostLabel->setText(TextUtils::currencyToString(cost->getAdjustedCost(), locale()));
		}
	}

	void ExternalOrderView::setDefaultTotals()
	{
		if (mSource != nullptr)
		{
			setAggregatedTotals({
				mSource->getTotalPrice(),
				mSource->getMinPrice(),
				mSource->getMaxPrice(),
				mSource->getMedianPrice(),
				mSource->getTotalVolume(),
				mSource->getTotalSize()
				});
		}
	}

	double ExternalOrderView::getSuggestedPrice() const
	{
		const auto priceDelta = PriceUtils::getPriceDelta();
		const auto &order = mSource->getOrder(mCurrentOrder.row());
		auto price = 0.;

		if (order.getType() == ExternalOrder::Type::Buy)
			price = order.getPrice() + priceDelta;
		else
			price = order.getPrice() - priceDelta;

		return price;
	}

	void ExternalOrderView::setAggregatedTotals(const MathUtils::AggregateData &data)
	{
		const auto curLocale = locale();
		mTotalPriceLabel->setText(TextUtils::currencyToString(data.mTotalPrice, curLocale));
		mTotalVolumeLabel->setText(curLocale.toString(data.mTotalVolume));
		mTotalSizeLabel->setText(QString{ "%1m³" }.arg(curLocale.toString(data.mTotalSize, 'f', 2)));
		mMinPriceLabel->setText(TextUtils::currencyToString(data.mMinPrice, curLocale));
		//mMedianPriceLabel->setText(TextUtils::currencyToString(data.mMedianPrice, curLocale));
		mMedianPriceLabel->setText(TextUtils::currencyToString(data.mTotalPrice / data.mTotalVolume, curLocale));
		mMaxPriceLabel->setText(TextUtils::currencyToString(data.mMaxPrice, curLocale));
	}

	QString ExternalOrderView::getDefaultCopySuggestedPriceText()
	{
		return tr("Copy suggested price");
	}
}