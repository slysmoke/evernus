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
#include <memory>
#include <cmath>

#include <QCategory3DAxis>
#include <QBarDataProxy>
#include <QValue3DAxis>
#include <QBar3DSeries>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <Q3DCamera>
#include <Q3DScene>
#include <Q3DTheme>
#include <QSlider>
#include <Q3DBars>
#include <QLabel>

#include "MiningLedgerRepository.h"
#include "EveDataProvider.h"

#include "MiningLedgerBarGraph.h"

using namespace QtDataVisualization;

namespace Evernus
{
    MiningLedgerBarGraph::MiningLedgerBarGraph(const MiningLedgerRepository &ledgerRepo,
                                               const EveDataProvider &dataProvider,
                                               QWidget *parent)
        : QWidget{parent}
        , mLedgerRepo{ledgerRepo}
        , mDataProvider{dataProvider}
        , mTypeAxis{new QCategory3DAxis{this}}
        , mSolarSystemAxis{new QCategory3DAxis{this}}
        , mValueAxis{new QValue3DAxis{this}}
        , mQuantitySeries{new QBar3DSeries{this}}
        , mProfitSeries{new QBar3DSeries{this}}
    {
        const auto mainLayout = new QHBoxLayout{this};

        mGraph = new Q3DBars{};

        const auto container = QWidget::createWindowContainer(mGraph, this);
        mainLayout->addWidget(container, 1);

        if (!mGraph->hasContext())
        {
            QMessageBox::warning(nullptr, tr("Mining ledger"), tr("Couldn't initialize mGraph!"));
            return;
        }

        mTypeAxis->setTitle(tr("Mined type"));
        mTypeAxis->setTitleVisible(true);

        mSolarSystemAxis->setTitle(tr("Solar system"));
        mSolarSystemAxis->setTitleVisible(true);

        mValueAxis->setLabelFormat(QStringLiteral("%i"));

        mGraph->setColumnAxis(mTypeAxis);
        mGraph->setRowAxis(mSolarSystemAxis);
        mGraph->setValueAxis(mValueAxis);

        mQuantitySeries->setItemLabelFormat(QStringLiteral("(@rowLabel, @colLabel): %i"));
        mQuantitySeries->setMesh(QAbstract3DSeries::MeshBevelBar);

        mProfitSeries->setItemLabelFormat(QStringLiteral("(@rowLabel, @colLabel): %dISK"));
        mProfitSeries->setMesh(QAbstract3DSeries::MeshBevelBar);
        mProfitSeries->setVisible(false);

        mGraph->addSeries(mQuantitySeries);
        mGraph->addSeries(mProfitSeries);

        const auto camera = mGraph->scene()->activeCamera();
        camera->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);

        mCameraXAnim.setTargetObject(camera);
        mCameraYAnim.setTargetObject(camera);
        mCameraZoomAnim.setTargetObject(camera);
        mCameraTargetAnim.setTargetObject(camera);

        mCameraXAnim.setPropertyName("xRotation");
        mCameraYAnim.setPropertyName("yRotation");
        mCameraZoomAnim.setPropertyName("zoomLevel");
        mCameraTargetAnim.setPropertyName("target");

        const auto animDuration = 1700;
        mCameraXAnim.setDuration(animDuration);
        mCameraYAnim.setDuration(animDuration);
        mCameraZoomAnim.setDuration(animDuration);
        mCameraTargetAnim.setDuration(animDuration);

        const auto zoomOutFraction = 0.3;
        mCameraXAnim.setKeyValueAt(zoomOutFraction, QVariant::fromValue(0.0f));
        mCameraYAnim.setKeyValueAt(zoomOutFraction, QVariant::fromValue(90.0f));
        mCameraZoomAnim.setKeyValueAt(zoomOutFraction, QVariant::fromValue(50.0f));
        mCameraTargetAnim.setKeyValueAt(zoomOutFraction, QVariant::fromValue(QVector3D{0.0f, 0.0f, 0.0f}));

        const auto infoLayout = new QVBoxLayout{};
        mainLayout->addLayout(infoLayout);
        infoLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        const auto infoText = new QLabel{
            tr("Hold the right mouse button to rotate the camera. Use the mouse wheel to zoom, and the left button to select data."),
            this
        };
        infoText->setWordWrap(true);
        infoLayout->addWidget(infoText);

        const auto dataTypeGroup = new QGroupBox{tr("Show"), this};
        infoLayout->addWidget(dataTypeGroup);

        const auto dataTypeLayout = new QHBoxLayout{dataTypeGroup};

        const auto showQuantityBtn = new QCheckBox{tr("Quantity"), this};
        dataTypeLayout->addWidget(showQuantityBtn);
        showQuantityBtn->setChecked(true);
        connect(showQuantityBtn, &QCheckBox::stateChanged, this, [=](auto state) {
            mQuantitySeries->setVisible(state == Qt::Checked);
        });

        const auto showProfitBtn = new QCheckBox{tr("Profit"), this};
        dataTypeLayout->addWidget(showProfitBtn);
        connect(showProfitBtn, &QCheckBox::stateChanged, this, [=](auto state) {
            mProfitSeries->setVisible(state == Qt::Checked);
        });

        infoLayout->addWidget(new QLabel{tr("Camera preset"), this});

        const auto cameraPresetCombo = new QComboBox{this};
        infoLayout->addWidget(cameraPresetCombo);
        cameraPresetCombo->addItem(tr("Front Low"), Q3DCamera::CameraPresetFrontLow);
        cameraPresetCombo->addItem(tr("Front"), Q3DCamera::CameraPresetFront);
        cameraPresetCombo->addItem(tr("Front High"), Q3DCamera::CameraPresetFrontHigh);
        cameraPresetCombo->addItem(tr("Left Low"), Q3DCamera::CameraPresetLeftLow);
        cameraPresetCombo->addItem(tr("Left"), Q3DCamera::CameraPresetLeft);
        cameraPresetCombo->addItem(tr("Left High"), Q3DCamera::CameraPresetLeftHigh);
        cameraPresetCombo->addItem(tr("Right Low"), Q3DCamera::CameraPresetRightLow);
        cameraPresetCombo->addItem(tr("Right"), Q3DCamera::CameraPresetRight);
        cameraPresetCombo->addItem(tr("Right High"), Q3DCamera::CameraPresetRightHigh);
        cameraPresetCombo->addItem(tr("Behind Low"), Q3DCamera::CameraPresetBehindLow);
        cameraPresetCombo->addItem(tr("Behind"), Q3DCamera::CameraPresetBehind);
        cameraPresetCombo->addItem(tr("Behind High"), Q3DCamera::CameraPresetBehindHigh);
        cameraPresetCombo->addItem(tr("Isometric Left"), Q3DCamera::CameraPresetIsometricLeft);
        cameraPresetCombo->addItem(tr("Isometric Left High"), Q3DCamera::CameraPresetIsometricLeftHigh);
        cameraPresetCombo->addItem(tr("Isometric Right"), Q3DCamera::CameraPresetIsometricRight);
        cameraPresetCombo->addItem(tr("Isometric Right High"), Q3DCamera::CameraPresetIsometricRightHigh);
        cameraPresetCombo->addItem(tr("Directly Above"), Q3DCamera::CameraPresetDirectlyAbove);
        cameraPresetCombo->addItem(tr("Directly Above CW45"), Q3DCamera::CameraPresetDirectlyAboveCW45);
        cameraPresetCombo->addItem(tr("Directly Above CCW45"), Q3DCamera::CameraPresetDirectlyAboveCCW45);
        cameraPresetCombo->setCurrentIndex(15);
        connect(cameraPresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [=] {
            stopCameraAnimations();

            const auto camera = mGraph->scene()->activeCamera();

            camera->setTarget({ 0.f, 0.f, 0.f });
            camera->setCameraPreset(static_cast<Q3DCamera::CameraPreset>(cameraPresetCombo->currentData().toInt()));
        });

        infoLayout->addWidget(new QLabel{tr("Bar style"), this});

        const auto barStyleCombo = new QComboBox{this};
        infoLayout->addWidget(barStyleCombo);
        barStyleCombo->addItem(tr("Bar"), QAbstract3DSeries::MeshBar);
        barStyleCombo->addItem(tr("Pyramid"), QAbstract3DSeries::MeshPyramid);
        barStyleCombo->addItem(tr("Cone"), QAbstract3DSeries::MeshCone);
        barStyleCombo->addItem(tr("Cylinder"), QAbstract3DSeries::MeshCylinder);
        barStyleCombo->addItem(tr("Bevel Bar"), QAbstract3DSeries::MeshBevelBar);
        barStyleCombo->addItem(tr("Sphere"), QAbstract3DSeries::MeshSphere);
        barStyleCombo->setCurrentIndex(4);
        connect(barStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [=] {
            const auto mesh = static_cast<QAbstract3DSeries::Mesh>(barStyleCombo->currentData().toInt());
            mQuantitySeries->setMesh(mesh);
            mProfitSeries->setMesh(mesh);
        });

        infoLayout->addWidget(new QLabel{tr("Font size"), this});

        const auto fontSizeSlider = new QSlider{Qt::Horizontal, this};
        infoLayout->addWidget(fontSizeSlider);
        fontSizeSlider->setRange(1, 100);
        fontSizeSlider->setValue(mGraph->activeTheme()->font().pointSize());
        connect(fontSizeSlider, &QSlider::valueChanged, this, &MiningLedgerBarGraph::setFontSize);

        const auto gridBtn = new QCheckBox{tr("Show grid"), this};
        infoLayout->addWidget(gridBtn);
        gridBtn->setChecked(mGraph->activeTheme()->isGridEnabled());
        connect(gridBtn, &QCheckBox::stateChanged, this, &MiningLedgerBarGraph::setGridEnabled);

        const auto smoothBtn = new QCheckBox{tr("Smooth bars"), this};
        infoLayout->addWidget(smoothBtn);
        smoothBtn->setChecked(mQuantitySeries->isMeshSmooth());
        connect(smoothBtn, &QCheckBox::stateChanged, this, &MiningLedgerBarGraph::setMeshSmooth);

        const auto zoomBtn = new QPushButton{tr("Zoom to selection"), this};
        infoLayout->addWidget(zoomBtn);
        connect(zoomBtn, &QPushButton::clicked, this, &MiningLedgerBarGraph::zoomToSelectedBar);
    }

    void MiningLedgerBarGraph::refresh(Character::IdType charId, const QDate &from, const QDate &to)
    {
        const auto data = mLedgerRepo.fetchAggregatedDataForCharacter(charId, from, to);

        for (const auto &datum : data)
        {
            const auto typeName = mDataProvider.getTypeName(datum.mTypeId);

            auto &info = mMappedData[mDataProvider.getSolarSystemName(datum.mSolarSystemId)][typeName];
            info.mQunatity += datum.mQuantity;
            info.mTypeId = datum.mTypeId;

            mAllTypes.insert(typeName);
        }

        auto quantityDataSet = std::make_unique<QBarDataArray>();
        quantityDataSet->reserve(static_cast<int>(mMappedData.size()));

        QStringList rowLabels;
        rowLabels.reserve(quantityDataSet->size());

        for (auto &systemData : mMappedData)
        {
            rowLabels << systemData.first;

            auto quantityRow = std::make_unique<QBarDataRow>(static_cast<int>(mAllTypes.size()));
            auto curQuantityCol = std::begin(*quantityRow);

            for (const auto &typeName : mAllTypes)
            {
                const auto &info = systemData.second[typeName];

                curQuantityCol->setValue(info.mQunatity);
                ++curQuantityCol;
            }

            quantityDataSet->append(quantityRow.release());
        }

        QStringList colLabels;
        colLabels.reserve(static_cast<int>(mAllTypes.size()));

        for (const auto &typeName : mAllTypes)
            colLabels << typeName;

        mQuantitySeries->dataProxy()->resetArray(quantityDataSet.release(), rowLabels, colLabels);

        refreshProfit();
    }

    void MiningLedgerBarGraph::setOrders(OrderList orders)
    {
        mPriceResolver.setOrders(std::move(orders));
        refreshProfit();
    }

    void MiningLedgerBarGraph::setSellPriceType(PriceType type)
    {
        mPriceResolver.setSellPriceType(type);
        refreshProfit();
    }

    void MiningLedgerBarGraph::setSellStation(quint64 stationId)
    {
        mPriceResolver.setSellStation(stationId);
        refreshProfit();
    }

    void MiningLedgerBarGraph::setFontSize(int size)
    {
        const auto theme = mGraph->activeTheme();

        auto font = theme->font();
        font.setPointSize(size);

        theme->setFont(font);
    }

    void MiningLedgerBarGraph::setGridEnabled(int state)
    {
        mGraph->activeTheme()->setGridEnabled(state == Qt::Checked);
    }

    void MiningLedgerBarGraph::setMeshSmooth(int state)
    {
        const auto enabled = state == Qt::Checked;

        mQuantitySeries->setMeshSmooth(enabled);
        mProfitSeries->setMeshSmooth(enabled);
    }

    void MiningLedgerBarGraph::zoomToSelectedBar()
    {
        const auto selectedBar = (mGraph->selectedSeries()) ?
                                 (mGraph->selectedSeries()->selectedBar()) :
                                 (QBar3DSeries::invalidSelectionPosition());
        if (selectedBar == QBar3DSeries::invalidSelectionPosition())
            return;

        stopCameraAnimations();

        const auto camera = mGraph->scene()->activeCamera();

        const auto currentX = camera->xRotation();
        const auto currentY = camera->yRotation();
        const auto currentZoom = camera->zoomLevel();
        const auto currentTarget = camera->target();

        mCameraXAnim.setStartValue(QVariant::fromValue(currentX));
        mCameraYAnim.setStartValue(QVariant::fromValue(currentY));
        mCameraZoomAnim.setStartValue(QVariant::fromValue(currentZoom));
        mCameraTargetAnim.setStartValue(QVariant::fromValue(currentTarget));

        // Normalize selected bar position within axis range to determine target coordinates
        QVector3D endTarget;
        const auto xMin = mGraph->columnAxis()->min();
        const auto xRange = mGraph->columnAxis()->max() - xMin;
        const auto zMin = mGraph->rowAxis()->min();
        const auto zRange = mGraph->rowAxis()->max() - zMin;
        endTarget.setX((selectedBar.y() - xMin) / xRange * 2.0f - 1.0f);
        endTarget.setZ((selectedBar.x() - zMin) / zRange * 2.0f - 1.0f);

        const auto pi = 3.14159265359;

        // Rotate the camera so that it always points approximately to the graph center
        auto endAngleX = std::atan(static_cast<qreal>(endTarget.z() / endTarget.x())) / pi * -180.0 + 90.0;
        if (endTarget.x() > 0.0f)
            endAngleX -= 180.0f;
        const auto barValue = mGraph->selectedSeries()->dataProxy()->itemAt(selectedBar.x(), selectedBar.y())->value();
        auto endAngleY = (barValue >= 0.0f) ? (30.0f) : (-30.0f);
        if (mGraph->valueAxis()->reversed())
            endAngleY *= -1.0f;

        mCameraXAnim.setEndValue(QVariant::fromValue(float(endAngleX)));
        mCameraYAnim.setEndValue(QVariant::fromValue(endAngleY));
        mCameraZoomAnim.setEndValue(QVariant::fromValue(250));
        mCameraTargetAnim.setEndValue(QVariant::fromValue(endTarget));

        mCameraXAnim.start();
        mCameraYAnim.start();
        mCameraZoomAnim.start();
        mCameraTargetAnim.start();
    }

    void MiningLedgerBarGraph::stopCameraAnimations()
    {
        mCameraXAnim.stop();
        mCameraYAnim.stop();
        mCameraZoomAnim.stop();
        mCameraTargetAnim.stop();
    }

    void MiningLedgerBarGraph::refreshProfit()
    {
        auto profitDataSet = std::make_unique<QBarDataArray>();
        profitDataSet->reserve(static_cast<int>(mMappedData.size()));

        for (auto &systemData : mMappedData)
        {
            auto profitRow = std::make_unique<QBarDataRow>(static_cast<int>(mAllTypes.size()));
            auto curProfitCol = std::begin(*profitRow);

            for (const auto &typeName : mAllTypes)
            {
                const auto &info = systemData.second[typeName];

                curProfitCol->setValue(mPriceResolver.getPrice(info.mTypeId, info.mQunatity));
                ++curProfitCol;
            }

            profitDataSet->append(profitRow.release());
        }

        mProfitSeries->dataProxy()->resetArray(profitDataSet.release());
    }
}
