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

#include <unordered_map>

#include <QDialog>

class QTreeWidgetItem;
class QProgressBar;
class QTreeWidget;
class QCheckBox;

#ifdef Q_OS_WIN
class QWinTaskbarProgress;
class QWinTaskbarButton;
#endif

namespace Evernus
{
    class ActiveTasksDialog
        : public QDialog
    {
        Q_OBJECT

    public:
#ifdef Q_OS_WIN
        explicit ActiveTasksDialog(QWinTaskbarButton &taskbarButton, QWidget *parent = nullptr);
#else
        explicit ActiveTasksDialog(QWidget *parent = nullptr);
#endif

        virtual ~ActiveTasksDialog() = default;

    signals:
        void taskCountChanged(size_t remaining);

    public slots:
        virtual void done(int r) override;

        void addNewTaskInfo(uint taskId, const QString &description);
        void addNewSubTaskInfo(uint taskId, uint parentTask, const QString &description);
        void setTaskInfo(uint taskId, const QString &description);
        void endTask(uint taskId, const QString &error = QString{});

    protected:
        virtual void closeEvent(QCloseEvent *event) override;

    private slots:
        void autoCloseSave(bool enabled);

    private:
        struct SubTaskInfo
        {
            size_t mCount = 0;
            bool mError = false;
        };

#ifdef Q_OS_WIN
        QWinTaskbarProgress *mTaskbarProgress = nullptr;
#endif

        QTreeWidget *mTaskWidget = nullptr;
        QProgressBar *mTotalProgressWidget = nullptr;
        QCheckBox *mAutoCloseBtn = nullptr;

        std::unordered_map<uint, QTreeWidgetItem *> mTaskItems;
        std::unordered_map<uint, SubTaskInfo> mSubTaskInfo;

        bool mHadError = false;

        void fillTaskItem(uint taskId, QTreeWidgetItem *item, const QString &description);
    };
}
