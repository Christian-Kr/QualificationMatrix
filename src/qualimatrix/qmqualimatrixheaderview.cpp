// qmqualimatrixheaderview.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmqualimatrixheaderview.h"
#include "settings/qmapplicationsettings.h"
#include "model/view/qmtrainingviewmodel.h"
#include "model/view/qmtraininggroupviewmodel.h"
#include "model/view/qmfunctionviewmodel.h"
#include "model/view/qmfunctiongroupviewmodel.h"

#include <QPainter>
#include <QHash>
#include <memory>
#include <QMouseEvent>
#include <QLabel>
#include <QGuiApplication>

#include <QDebug>

QMQualiMatrixHeaderView::QMQualiMatrixHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , horSectionHeight(0)
    , vertSectionWidth(0)
    , funcGroupColorCache(new QHash<QString, QString>())
    , trainGroupColorCache(new QHash<QString, QString>())
    , trainLegallyNecessaryCache(new QHash<QString, bool>())
    , gridColor(QColor("#ffffff"))
    , selectionColor(QColor("#ffffff"))
    , laInfo(new QLabel(parent))
{
    setSectionResizeMode(QHeaderView::Fixed);
    setMinimumSectionSize(30);
    setDefaultSectionSize(30);
    setStyleSheet("background-color: white;");

    setSectionsClickable(true);

    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    vertSectionWidth = settings.read("QualiMatrix/VertHeaderWidth", 200).toInt();
    horSectionHeight = settings.read("QualiMatrix/HorHeaderHeight", 200).toInt();

    updateColors();

    laInfo->setAttribute(Qt::WA_TransparentForMouseEvents);
    laInfo->setText(tr("HallO"));

    if (orientation == Qt::Horizontal)
    {
        laInfo->setGeometry(0, 0, 200, sizeHint().width());
    }

    laInfo->setStyleSheet("background-color: #FFFFFF; border: 1px solid #000000;");
    laInfo->setVisible(false);
}

QMQualiMatrixHeaderView::~QMQualiMatrixHeaderView()
{
    delete funcGroupColorCache;
    delete trainGroupColorCache;
    delete trainLegallyNecessaryCache;
    delete laInfo;
}

void QMQualiMatrixHeaderView::updateTrainingGroupColors()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    std::unique_ptr<QSqlTableModel> trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    std::unique_ptr<QSqlTableModel> trainGroupViewModel = std::make_unique<QMTrainingGroupViewModel>(this, db);
    trainGroupViewModel->select();

    trainGroupColorCache->clear();

    for (int i = 0; i < trainViewModel->rowCount(); i++)
    {
        // Get the function name.
        auto train = trainViewModel->data(trainViewModel->index(i, 1)).toString();

        // Get the group name and search for its color.
        auto group = trainViewModel->data(trainViewModel->index(i, 2)).toString();

        // Find color of group.
        QString color = "#ffffff";
        for (int j = 0; j < trainGroupViewModel->rowCount(); j++)
        {
            auto tmpGroup = trainGroupViewModel->data(trainGroupViewModel->index(j, 1)).toString();

            if (group == tmpGroup)
            {
                color = trainGroupViewModel->data(trainGroupViewModel->index(j, 2)).toString();
                break;
            }
        }

        // Add to cache.
        trainGroupColorCache->insert(train, color);
    }
}

void QMQualiMatrixHeaderView::showHeaderLabel(int section, QString text)
{
    laInfo->setVisible(true);
    laInfo->setText(text);

    QFontMetricsF metrics(laInfo->font());
    double width = metrics.boundingRect(metrics.boundingRect(text), 0, text).width() + 10;

    laInfo->setFixedWidth(width);

    QPoint point = mapToParent(QPoint(section * sectionSize(section), sizeHint().height() - laInfo->height()));
    laInfo->move(point.x(), point.y());
}

void QMQualiMatrixHeaderView::hideHeaderLabel()
{
    laInfo->setVisible(false);
}

void QMQualiMatrixHeaderView::updateTrainLegallyNecessary()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    std::unique_ptr<QSqlTableModel> trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    trainLegallyNecessaryCache->clear();

    for (int i = 0; i < trainViewModel->rowCount(); i++)
    {
        auto train = trainViewModel->data(trainViewModel->index(i, 1)).toString();
        auto legallyNecessary = trainViewModel->data(trainViewModel->index(i, 4)).toBool();

        trainLegallyNecessaryCache->insert(train, legallyNecessary);
    }
}

void QMQualiMatrixHeaderView::updateFunctionGroupColors()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    std::unique_ptr<QSqlTableModel> funcViewModel = std::make_unique<QMFunctionViewModel>(this, db);
    funcViewModel->select();

    std::unique_ptr<QSqlTableModel> funcGroupViewModel = std::make_unique<QMFunctionGroupViewModel>(this, db);
    funcGroupViewModel->select();

    funcGroupColorCache->clear();

    // Build cache.
    for (int i = 0; i < funcViewModel->rowCount(); i++)
    {
        // Get the function name.
        QString func = funcViewModel->data(funcViewModel->index(i, 1)).toString();

        // Get the group name and search for its color.
        QString group = funcViewModel->data(funcViewModel->index(i, 2)).toString();

        // Find color of group.
        QString color = "#ffffff";
        for (int j = 0; j < funcGroupViewModel->rowCount(); j++)
        {
            QString tmpGroup = funcGroupViewModel->data(funcGroupViewModel->index(j, 1)).toString();
            if (group == tmpGroup)
            {
                color = funcGroupViewModel->data(funcGroupViewModel->index(j, 2)).toString();
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

void QMQualiMatrixHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
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
                    model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString(), "");

            if (!trainGroupColor.isEmpty())
            {
                painter->setBrush(QBrush(QColor(trainGroupColor)));
                painter->drawRect(rect);
                painter->setBrush(QBrush(QColor(Qt::white)));
            }
        }

        painter->setPen(QPen(QColor(Qt::black)));

        // Draw an information if the training is legally necessary. For now, this is just a short text note.
        auto trainLegallyNecessary = trainLegallyNecessaryCache->value(
                model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString(), false);

        if (trainLegallyNecessary)
        {
            // painter->drawRect(rect.x() + 1, rect.y(), rect.width() - 2, rect.width() - 2);
            painter->drawText(rect.x() + 1, rect.y(), rect.width() - 2, rect.width() - 2, Qt::AlignCenter, tr("RN"));
        }

        // Draw the name of the training in the right direction.
        painter->translate(rect.x(), rect.y());
        painter->setFont(font);

        if (settings.read("QualiMatrix/TrainBottomTop", false).toBool())
        {
            painter->rotate(-90);
            painter->drawText(-1 * rect.height() + 10, 0, rect.height() - 50, rect.width(),
                    Qt::AlignVCenter, model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString());
        }
        else
        {
            painter->rotate(90);
            painter->drawText(40, -rect.width(), rect.height() - 20, rect.width(),
                    Qt::AlignVCenter, model()->headerData(logicalIndex, Qt::Orientation::Horizontal).toString());
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
                    model()->headerData(logicalIndex, Qt::Orientation::Vertical).toString(), "");

            if (!funcGroupColor.isEmpty())
            {
                painter->setBrush(QBrush(QColor(funcGroupColor)));
                painter->drawRect(rect);
                painter->setBrush(QBrush(QColor(Qt::white)));
            }
        }

        painter->setPen(QPen(QColor(Qt::black)));

        painter->translate(rect.x(), rect.y());
        painter->setFont(font);
        painter->drawText(10, 0, rect.width() - 1, rect.height(), Qt::AlignVCenter,
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

