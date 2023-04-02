/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QSGFlatColorMaterial>
#include <QSGNode>

#include "BezierCurve.h"

namespace Evernus
{
    BezierCurve::BezierCurve(QQuickItem *parent)
        : QQuickItem(parent)
        , mP1(0, 0)
        , mP2(1, 0)
        , mP3(0, 1)
        , mP4(1, 1)
    {
        setFlag(ItemHasContents, true);
    }

    void BezierCurve::setP1(const QPointF &p)
    {
        if (p == mP1)
            return;

        mP1 = p;
        emit p1Changed(p);
        update();
    }

    void BezierCurve::setP2(const QPointF &p)
    {
        if (p == mP2)
            return;

        mP2 = p;
        emit p2Changed(p);
        update();
    }

    void BezierCurve::setP3(const QPointF &p)
    {
        if (p == mP3)
            return;

        mP3 = p;
        emit p3Changed(p);
        update();
    }

    void BezierCurve::setP4(const QPointF &p)
    {
        if (p == mP4)
            return;

        mP4 = p;
        emit p4Changed(p);
        update();
    }

    void BezierCurve::setSegmentCount(int count)
    {
        if (mSegmentCount == count)
            return;

        mSegmentCount = count;
        emit segmentCountChanged(count);
        update();
    }

    void BezierCurve::setColor(const QColor &color)
    {
        if (mColor == color)
            return;

        mColor = color;
        emit colorChanged(color);
        update();
    }

    QSGNode *BezierCurve::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
    {
        QSGGeometryNode *node = nullptr;
        QSGGeometry *geometry = nullptr;

        if (oldNode == nullptr)
        {
            node = new QSGGeometryNode;
            geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mSegmentCount);
            geometry->setLineWidth(2);
            geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
            node->setGeometry(geometry);
            node->setFlag(QSGNode::OwnsGeometry);

            const auto material = new QSGFlatColorMaterial;
            material->setColor(mColor);
            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
        }
        else
        {
            node = static_cast<QSGGeometryNode *>(oldNode);
            geometry = node->geometry();
            geometry->allocate(mSegmentCount);
        }

        const auto bounds = boundingRect();
        const auto vertices = geometry->vertexDataAsPoint2D();

        for (auto i = 0; i < mSegmentCount; ++i)
        {
            const auto t = i / qreal(mSegmentCount - 1);
            const auto invt = 1 - t;

            const auto pos = invt * invt * invt * mP1
                             + 3 * invt * invt * t * mP2
                             + 3 * invt * t * t * mP3
                             + t * t * t * mP4;

            float x = bounds.x() + pos.x() * bounds.width();
            float y = bounds.y() + pos.y() * bounds.height();

            vertices[i].set(x, y);
        }

        node->markDirty(QSGNode::DirtyGeometry);
        return node;
    }
}
