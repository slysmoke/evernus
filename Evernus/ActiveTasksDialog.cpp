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
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QCloseEvent>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSettings>
#include <QLabel>
#include <QMovie>
#include <QFont>

#ifdef Q_OS_WIN
#   include <QWinTaskbarProgress>
#   include <QWinTaskbarButton>
#endif

#include "UISettings.h"

#include "ActiveTasksDialog.h"

namespace Evernus
{
#ifdef Q_OS_WIN
    ActiveTasksDialog::ActiveTasksDialog(QWinTaskbarButton &taskbarButton, QWidget *parent)
#else
    ActiveTasksDialog::ActiveTasksDialog(QWidget *parent)
#endif
        : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};
        auto throbberMovie = new QMovie{":/images/loader.gif", QByteArray{}, this};

        QFont font;
        font.setPixelSize(16);

        auto throbberLayout = new QHBoxLayout{};
        mainLayout->addLayout(throbberLayout);

        auto throbber = new QLabel{this};
        throbberLayout->addWidget(throbber);
        throbber->setMovie(throbberMovie);

        throbberMovie->start();

        auto throbberText = new QLabel{tr("Please wait..."), this};
        throbberLayout->addWidget(throbberText, 1, Qt::AlignLeft);
        throbberText->setFont(font);

        mTaskWidget = new QTreeWidget{this};
        mainLayout->addWidget(mTaskWidget, 1);
        mTaskWidget->setHeaderHidden(true);
        mTaskWidget->setColumnCount(2);

        auto progressLayout = new QHBoxLayout{};
        mainLayout->addLayout(progressLayout);

        mAutoCloseBtn = new QCheckBox{tr("Close automatically"), this};
        progressLayout->addWidget(mAutoCloseBtn);
        mAutoCloseBtn->setChecked(settings.value(UISettings::autoCloseTasksKey, UISettings::autoCloseTasksDefault).toBool());
        connect(mAutoCloseBtn, &QCheckBox::toggled, this, &ActiveTasksDialog::autoCloseSave);

        mTotalProgressWidget = new QProgressBar{this};
        progressLayout->addWidget(mTotalProgressWidget);
        mTotalProgressWidget->setMinimumWidth(500);
        mTotalProgressWidget->setValue(0);
        mTotalProgressWidget->setMaximum(0);

        auto btnBox = new QDialogButtonBox{QDialogButtonBox::Close, this};
        mainLayout->addWidget(btnBox);
        connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

#ifdef Q_OS_WIN
        mTaskbarProgress = taskbarButton.progress();
#endif

        setWindowTitle(tr("Active Tasks"));
    }

    void ActiveTasksDialog::done(int r)
    {
        if (mTaskItems.empty())
            mTaskWidget->clear();

        QDialog::done(r);
    }

    void ActiveTasksDialog::addNewTaskInfo(uint taskId, const QString &description)
    {
        if (mTaskItems.empty())
        {
            mHadError = false;
            mTotalProgressWidget->setValue(0);
            mTotalProgressWidget->setMaximum(1);
#ifdef Q_OS_WIN
            mTaskbarProgress->setValue(0);
            mTaskbarProgress->setMaximum(1);
            mTaskbarProgress->setVisible(true);
#endif
        }
        else
        {
            mTotalProgressWidget->setMaximum(mTotalProgressWidget->maximum() + 1);
#ifdef Q_OS_WIN
            mTaskbarProgress->setMaximum(mTaskbarProgress->maximum() + 1);
#endif
        }

        fillTaskItem(taskId, new QTreeWidgetItem{mTaskWidget}, description);
        mTaskWidget->resizeColumnToContents(0);

        emit taskCountChanged(mTaskItems.size());
    }

    void ActiveTasksDialog::addNewSubTaskInfo(uint taskId, uint parentTask, const QString &description)
    {
        auto item = mTaskItems.find(parentTask);
        if (item == std::end(mTaskItems))
        {
            QMetaObject::invokeMethod(this, "addNewSubTaskInfo", Qt::QueuedConnection, Q_ARG(uint, taskId), Q_ARG(uint, parentTask), Q_ARG(QString, description));
            return;
        }

        mTotalProgressWidget->setMaximum(mTotalProgressWidget->maximum() + 1);
#ifdef Q_OS_WIN
        mTaskbarProgress->setMaximum(mTaskbarProgress->maximum() + 1);
#endif

        fillTaskItem(taskId, new QTreeWidgetItem{item->second}, description);
        mTaskWidget->resizeColumnToContents(0);

        ++mSubTaskInfo[parentTask].mCount;

        emit taskCountChanged(mTaskItems.size());
    }

    void ActiveTasksDialog::setTaskInfo(uint taskId, const QString &description)
    {
        auto item = mTaskItems.find(taskId);
        if (item == std::end(mTaskItems))
        {
            QMetaObject::invokeMethod(this, "setTaskInfo", Qt::QueuedConnection, Q_ARG(uint, taskId), Q_ARG(QString, description));
            return;
        }

        item->second->setText(1, description);
    }

    void ActiveTasksDialog::endTask(uint taskId, const QString &error)
    {
        auto item = mTaskItems.find(taskId);
        if (item == std::end(mTaskItems))
        {
            QMetaObject::invokeMethod(this, "endTask", Qt::QueuedConnection, Q_ARG(uint, taskId), Q_ARG(QString, error));
            return;
        }

        const auto subTaskInfo = mSubTaskInfo.find(taskId);
        const auto success = error.isEmpty() && (subTaskInfo == std::end(mSubTaskInfo) || !subTaskInfo->second.mError);

        mHadError = mHadError || !success;

        const auto parent = item->second->parent();

        item->second->setIcon(0, QIcon{(success) ? (":/images/accept.png") : (":/images/exclamation.png")});
        if (!success && !error.isEmpty())
        {
            const auto individualErrors = error.split('\n', QString::SkipEmptyParts);
            for (const auto &individualError : individualErrors)
            {
                auto errorItem = new QTreeWidgetItem{item->second};
                errorItem->setText(1, individualError);
            }
        }

        mTaskItems.erase(item);
        mTotalProgressWidget->setValue(mTotalProgressWidget->value() + 1);
#ifdef Q_OS_WIN
        mTaskbarProgress->setValue(mTaskbarProgress->value() + 1);
#endif

        emit taskCountChanged(mTaskItems.size());

        if (parent != nullptr)
        {
            const auto it = mSubTaskInfo.find(parent->data(0, Qt::UserRole).toUInt());
            Q_ASSERT(it != std::end(mSubTaskInfo));

            it->second.mError = it->second.mError || !success;

            --it->second.mCount;
            if (it->second.mCount == 0)
            {
                endTask(it->first);
                mSubTaskInfo.erase(it);
            }
        }
        else if (mTaskItems.empty())
        {
#ifdef Q_OS_WIN
            mTaskbarProgress->setVisible(false);
#endif

            if (mAutoCloseBtn->isChecked() && !mHadError)
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        }
    }

    void ActiveTasksDialog::closeEvent(QCloseEvent *event)
    {
        if (mTaskItems.empty())
        {
            mTaskWidget->clear();
            QDialog::closeEvent(event);
        }
        else
        {
            event->ignore();
        }
    }

    void ActiveTasksDialog::autoCloseSave(bool enabled)
    {
        QSettings settings;
        settings.setValue(UISettings::autoCloseTasksKey, enabled);
    }

    void ActiveTasksDialog::fillTaskItem(uint taskId, QTreeWidgetItem *item, const QString &description)
    {
        item->setIcon(0, QIcon{":/images/information.png"});
        item->setData(0, Qt::UserRole, taskId);
        item->setText(1, description);
        item->setExpanded(true);

        mTaskItems[taskId] = item;
    }
}
