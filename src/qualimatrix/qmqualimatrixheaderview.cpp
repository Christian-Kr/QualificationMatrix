//
// qualimatrixheaderview.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#include "qmqualimatrixheaderview.h"
#include "settings/qmapplicationsettings.h"
#include "model/qmdatamanager.h"

#include <QPainter>
#include <QHash>
#include <QSqlRelationalTableModel>
#include <memory>
#include <QDebug>

QMQualiMatrixHeaderView::QMQualiMatrixHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent),
    horSectionHeight(0),
    vertSectionWidth(0),
    funcGroupColorCache(new QHash<QString, QString>()),
    trainGroupColorCache(new QHash<QString, QString>()),
    trainLegallyNecessaryCache(new QHash<QString, bool>()),
    gridColor(QColor("#ffffff")),
    selectionColor(QColor("#ffffff"))
{
    setSectionResizeMode(QHeaderView::Fixed);
    setDefaultSectionSize(30);
    setStyleSheet("background-color: white;");

    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    vertSectionWidth = settings.read("QualiMatrix/VertHeaderWidth", 200).toInt();
    horSectionHeight = settings.read("QualiMatrix/HorHeaderHeight", 200).toInt();

    connect(&settings, SIGNAL(settingsChanged()), this, SLOT(updateSizeSettings()));

    updateColors();
}

QMQualiMatrixHeaderView::~QMQualiMatrixHeaderView()
{
    delete funcGroupColorCache;
    delete trainGroupColorCache;
    delete trainLegallyNecessaryCache;
}

void QMQualiMatrixHeaderView::updateTrainingGroupColors()
{
    // Get color from models.
    auto dm = QMDataManager::getInstance();

    trainGroupColorCache->clear();

    // Build cache.
    for (int i = 0; i < dm->getTrainModel()->rowCount(); i++)
    {
        // Get the function name.
        auto train = dm->getTrainModel()->data(dm->getTrainModel()->index(i, 1)).toString();

        // Get the group name and search for its color.
        auto group = dm->getTrainModel()->data(dm->getTrainModel()->index(i, 2)).toString();

        // Find color of group.
        QString color = "#ffffff";
        for (int j = 0; j < dm->getTrainGroupModel()->rowCount(); j++)
        {
            auto tmpGroup = dm->getTrainGroupModel()->data(
                dm->getTrainGroupModel()->index(j, 1)).toString();

            if (group == tmpGroup)
            {
                color = dm->getTrainGroupModel()->data(
                    dm->getTrainGroupModel()->index(j, 2)).toString();
                break;
            }
        }

        // Add to cache.
        trainGroupColorCache->insert(train, color);
    }
}

void QMQualiMatrixHeaderView::updateTrainLegallyNecessary()
{
    auto dm = QMDataManager::getInstance();

    trainLegallyNecessaryCache->clear();
    for (int i = 0; i < dm->getTrainModel()->rowCount(); i++)
    {
        auto train = dm->getTrainModel()->data(dm->getTrainModel()->index(i, 1)).toString();
        auto legallyNecessary = dm->getTrainModel()->data(dm->getTrainModel()->index(i, 4))
            .toBool();

        trainLegallyNecessaryCache->insert(train, legallyNecessary);
    }
}

void QMQualiMatrixHeaderView::updateFunctionGroupColors()
{
    // Get color from models.
    auto dm = QMDataManager::getInstance();

    funcGroupColorCache->clear();

    // Build cache.
    for (int i = 0; i < dm->getFuncModel()->rowCount(); i++)
    {
        // Get the function name.
        QString func = dm->getFuncModel()->data(dm->getFuncModel()->index(i, 1)).toString();

        // Get the group name and search for its color.
        QString group = dm->getFuncModel()->data(dm->getFuncModel()->index(i, 2)).toString();

        // Find color of group.
        QString color = "#ffffff";
        for (int j = 0; j < dm->getFuncGroupModel()->rowCount(); j++)
        {
            QString tmpGroup = dm->getFuncGroupModel()->data(dm->getFuncGroupModel()->index(j, 1))
                .toString();
            if (group == tmpGroup)
            {
                color = dm->getFuncGroupModel()->data(dm->getFuncGroupModel()->index(j, 2))
                    .toString();
                break;
            }
        }

        // Add to cache.
        funcGroupColorCache->insert(func, color);
    }
}

void QMQualiMatrixHeaderView::updateSizeSettings()
{
    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    int tmpVertSectionWidth = settings.read("QualiMatrix/VertHeaderWidth", 200).toInt();
    int tmpHorSectionHeight = settings.read("QualiMatrix/HorHeaderHeight", 200).toInt();

    if (vertSectionWidth != tmpVertSectionWidth || horSectionHeight != tmpHorSectionHeight)
    {
        vertSectionWidth = tmpVertSectionWidth;
        horSectionHeight = tmpHorSectionHeight;
        emit geometriesChanged();
    }

    updateColors();
}

void QMQualiMatrixHeaderView::updateColors()
{
    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    selectionColor = QColor(settings.read("QualiMatrix/SelectionColor", "#ffffff").toString());
    gridColor = QColor(settings.read("QualiMatrix/GridColor", "#ffffff").toString());
}

void QMQualiMatrixHeaderView::paintSection(
    QPainter *painter, const QRect &rect, int logicalIndex) const
{
    auto &settings = QMApplicationSettings::getInstance();

    QFont font;
    font.setPointSize(10);

    if (orientation() == Qt::Orientation::Horizontal)
    {
        painter->setPen(gridColor);

        if (selectionModel()->currentIndex().column() == logicalIndex)
        {
            painter->setBrush(QBrush(QColor(selectionColor)));
            painter->drawRect(rect);
            painter->setBrush(QBrush(QColor(Qt::white)));
        }
        else
        {
            auto trainGroupColor = trainGroupColorCache->value(
                model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString(),
                "no");

            if (trainGroupColor != "no")
            {
                painter->setBrush(QBrush(QColor(trainGroupColor)));
                painter->drawRect(rect);
                painter->setBrush(QBrush(QColor(Qt::white)));
            }
        }

        painter->setPen(QPen(QColor(Qt::black)));

        auto trainLegallyNecessary = trainLegallyNecessaryCache->value(
            model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString(), false);

        if (trainLegallyNecessary)
        {
            painter->setBrush(QBrush(QColor("#F0F0F0")));
            painter->drawRect(rect.x(), rect.y(), rect.width(), 30);
            painter->drawText(
                rect.x() + 1, rect.y(), rect.width() - 2, 30, Qt::AlignCenter, tr("RN"));
            painter->setBrush(QBrush(QColor(Qt::white)));
        }

        painter->translate(rect.x(), rect.y());
        painter->setFont(font);

        if (settings.read("QualiMatrix/TrainBottomTop", false).toBool())
        {
            painter->rotate(-90);
            painter->drawText(
                -1 * sizeHint().height() + 10, 0, sizeHint().height() - 50, sizeHint().width(),
                Qt::AlignVCenter,
                model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString());
        }
        else
        {
            painter->rotate(90);
            painter->drawText(
                40, -sizeHint().width(), sizeHint().height() - 20, sizeHint().width(),
                Qt::AlignVCenter,
                model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString());
        }
    }
    else
    {
        painter->setPen(gridColor);

        if (selectionModel()->currentIndex().row() == logicalIndex)
        {
            painter->setBrush(QBrush(QColor(selectionColor)));
            painter->drawRect(rect);
            painter->setBrush(QBrush(QColor(Qt::white)));
        }
        else
        {
            QString funcGroupColor = funcGroupColorCache->value(
                model()->headerData(logicalIndex, Qt::Orientation::Vertical).toString(), "no"
            );

            if (funcGroupColor != "no")
            {
                painter->setBrush(QBrush(QColor(funcGroupColor)));
                painter->drawRect(rect);
                painter->setBrush(QBrush(QColor(Qt::white)));
            }
        }

        painter->setPen(QPen(QColor(Qt::black)));

        painter->translate(rect.x(), rect.y());
        painter->setFont(font);
        painter->drawText(
            10, 0, sizeHint().width() - 1, sizeHint().height(), Qt::AlignVCenter,
            model()->headerData(logicalIndex, Qt::Orientation::Vertical).toString());
    }
}

QSize QMQualiMatrixHeaderView::sizeHint() const
{
    if (orientation() == Qt::Orientation::Horizontal)
    {
        return {30, horSectionHeight};
    }
    else
    {
        return {vertSectionWidth, 30};
    }
}
