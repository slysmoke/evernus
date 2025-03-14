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
#include <QTableWidgetItem>
#include <QApplication>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QClipboard>
#include <QSettings>
#include <QLabel>
#include <QDebug>

#include <QMessageBox>


#include "PriceSettings.h"

#include "CustomFPCDialog.h"

namespace Evernus
{
    CustomFPCDialog::CustomFPCDialog(QWidget* parent)
        : QDialog(parent)
    {

        CustomFPCDialog::setMinimumWidth(335);
        CustomFPCDialog::resize(400, 650);

        // CustomFPCDialog::setWindowFlags(Qt::WindowStaysOnTopHint);

        auto mainLayout = new QVBoxLayout{ this };

        auto helpLabel = new QLabel{ tr(
                                        "You can copy raw data into the clipboard for use as a custom Fast Price Copy source. "
                                        "The first column should contain item type id, the second its price to copy (optional) and third its Quantity (optional) ."),
                                    this };
        mainLayout->addWidget(helpLabel);
        helpLabel->setWordWrap(true);

        auto pasteBtn = new QPushButton{ tr("Paste data"), this };
        mainLayout->addWidget(pasteBtn);
        connect(pasteBtn, &QPushButton::clicked, this, &CustomFPCDialog::pasteData);

        mDataView = new QTableWidget{ this };
        mainLayout->addWidget(mDataView);
        mDataView->setColumnCount(3);
        mDataView->setHorizontalHeaderLabels({ tr("Type"), tr("Price"), tr("Quantity") });
        mDataView->setSelectionMode(QAbstractItemView::SingleSelection);
        mDataView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mDataView->setEditTriggers(QAbstractItemView::NoEditTriggers);


    }



    void CustomFPCDialog::executeFPC()
    {
        static int mCurrentColumn = 1;

        auto row = mDataView->currentRow();
        if (row < 0 || row >= mDataView->rowCount())
            return;

        auto item = mDataView->item(row, mCurrentColumn);
        if (item == nullptr)
            return;

        copyData(row, mCurrentColumn);

        try
        {
            QIcon icon(":/images/accept.png");
            item->setIcon(icon);
        }
        catch (const std::exception& e)
        {
            // Handle exception or log error
            qDebug() << "Error setting icon:" << e.what();
            return;
        }

        if (mCurrentColumn == 1 && mDataView->item(row, 2) != nullptr)
        {
            mCurrentColumn = 2;
        }
        else
        {
            mCurrentColumn = 1;
            ++row;
            if (row >= mDataView->rowCount())
                row = 0;
        }

        mDataView->setCurrentCell(row, 0);
    }

    // void CustomFPCDialog::executeFPC()
    // {
    //     auto row = mDataView->currentRow();
    //     if (row >= 0)
    //     {
    //         copyData(row);

    //         ++row;
    //         if (row >= mDataView->rowCount())
    //             row = 0;

    //         mDataView->setCurrentCell(row, 0);
    //     }
    // }

    void CustomFPCDialog::executeBackwardFPC()
    {
        static int mCurrentColumn = 1;

        auto row = mDataView->currentRow();
        if (row < 0)
            return;

        auto item = mDataView->item(row, mCurrentColumn);
        if (item == nullptr)
            return;

        copyData(row, mCurrentColumn);

        if (item != nullptr)
            item->setIcon(QIcon());

        if (mCurrentColumn == 1 && mDataView->item(row, 2) != nullptr)
        {
            mCurrentColumn = 2;
        }
        else
        {
            mCurrentColumn = 1;

            --row;
            if (row < 0)
                row = mDataView->rowCount() - 1;
        }

        if (row >= 0 && row < mDataView->rowCount())
            mDataView->setCurrentCell(row, 0);
        else
            mDataView->clearSelection();
    }

    void CustomFPCDialog::pasteData()
    {
        const auto data = QApplication::clipboard()->text();
        if (data.isEmpty())
            return;

        mDataView->clearContents();

        auto row = 0;
        const auto lines = data.split('\n', QString::SkipEmptyParts);
        mDataView->setRowCount(lines.size());

        for (const auto& line : lines)
        {
            const auto values = line.split('\t', QString::SkipEmptyParts);
            if (values.size() > 0)
            {
                bool isInt;
                values[0].toInt(&isInt);
                if (!isInt)
                {

                    QMessageBox::critical(this, tr("Invalid Input"), tr("The first column must contain integers only."));
                    mDataView->setRowCount(0);
                    return;
                }

                mDataView->setItem(row, 0, new QTableWidgetItem{ values[0] });
                if (values.size() > 1)
                    mDataView->setItem(row, 1, new QTableWidgetItem{ values[1] });
                if (values.size() > 2)
                    mDataView->setItem(row, 2, new QTableWidgetItem{ values[2] });

                ++row;
            }
        }

        mDataView->setRowCount(row);
        mDataView->setCurrentCell(0, 0);
    }

    void CustomFPCDialog::copyData(int row, int column) const
    {
        const auto item = mDataView->item(row, column);
        if (item != nullptr)
        {
            auto ok = false;
            const auto text = item->text();
            const auto value = text.toDouble(&ok);

            if (ok)
                QApplication::clipboard()->setText(QString::number(value, 'f', 2));
            else
                QApplication::clipboard()->setText(text); // Если значение не число, копируем текст как есть
        }

        QSettings settings;
        if (settings.value(PriceSettings::showInEveOnFpcKey, PriceSettings::showInEveOnFpcDefault).toBool())
        {
            const auto idItem = mDataView->item(row, 0);
            if (idItem != nullptr)
            {
                const auto id = idItem->text().toULongLong();
                if (id != EveType::invalidId)
                    emit showInEve(id);
            }
        }
    }

    // void CustomFPCDialog::copyData(int row) const
    // {
    //     const auto priceItem = mDataView->item(row, 1);
    //     if (priceItem != nullptr)
    //     {
    //         auto ok = false;
    //         const auto price = priceItem->text().toDouble(&ok);

    //         if (ok)
    //             QApplication::clipboard()->setText(QString::number(price, 'f', 2));
    //     }

    //     QSettings settings;
    //     if (settings.value(PriceSettings::showInEveOnFpcKey, PriceSettings::showInEveOnFpcDefault).toBool())
    //     {
    //         const auto id = mDataView->item(row, 0)->text().toULongLong();
    //         if (id != EveType::invalidId)
    //             emit showInEve(id);
    //     }
    // }
}
