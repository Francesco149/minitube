/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "segmentedcontrol.h"
#include "mainwindow.h"
#include "fontutils.h"
#include "iconutils.h"
#include "painterutils.h"

SegmentedControl::SegmentedControl (QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);

    hoveredAction = 0;
    checkedAction = 0;
    pressedAction = 0;

#ifdef APP_WIN
    int darkerFactor = 105;
    selectedColor = palette().color(QPalette::Base);
#else
    int darkerFactor = 115;
    selectedColor = palette().color(QPalette::Window);
#endif
    backgroundColor = selectedColor.darker(darkerFactor);
    borderColor = backgroundColor.darker(darkerFactor);
}

QAction *SegmentedControl::addAction(QAction *action) {
    QWidget::addAction(action);
    action->setCheckable(true);
    actionList.append(action);
    return action;
}

bool SegmentedControl::setCheckedAction(int index) {
    if (index < 0) {
        checkedAction = 0;
        return true;
    }
    QAction* newCheckedAction = actionList.at(index);
    return setCheckedAction(newCheckedAction);
}

bool SegmentedControl::setCheckedAction(QAction *action) {
    if (checkedAction == action) {
        return false;
    }
    if (checkedAction)
        checkedAction->setChecked(false);
    checkedAction = action;
    checkedAction->setChecked(true);
    update();
    return true;
}

QSize SegmentedControl::minimumSizeHint (void) const {
    int itemsWidth = calculateButtonWidth() * actionList.size() * 1.2;
    return(QSize(itemsWidth, QFontMetrics(font()).height() * 1.8));
}

void SegmentedControl::paintEvent (QPaintEvent * /*event*/) {
    const int height = rect().height();
    const int width = rect().width();

    QPainter p(this);
    p.fillRect(rect(), backgroundColor);

    // Calculate Buttons Size & Location
    const int buttonWidth = width / actionList.size();

    const qreal pixelRatio = IconUtils::pixelRatio();

    QPen pen(borderColor);
    const qreal penWidth = 1. / pixelRatio;
    pen.setWidthF(penWidth);
    p.setPen(pen);

    // Draw Buttons
    QRect rect(0, 0, buttonWidth, height);
    const int actionCount = actionList.size();
    for (int i = 0; i < actionCount; i++) {
        QAction *action = actionList.at(i);
        if (i + 1 == actionCount) {
            // last button
            rect.setWidth(width - buttonWidth * (actionCount-1));
            drawButton(&p, rect, action);
        } else {
            drawButton(&p, rect, action);
            qreal w = rect.x() + rect.width() - penWidth;
            p.drawLine(QPointF(w, 0), QPointF(w, height));
            rect.moveLeft(rect.x() + rect.width());
        }
    }
    const qreal y = height - penWidth;
    p.drawLine(QPointF(0, y), QPointF(width, y));
}

void SegmentedControl::mouseMoveEvent (QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

    QAction *action = findHoveredAction(event->pos());

    if (!action && hoveredAction) {
        hoveredAction = 0;
        update();
    } else if (action && action != hoveredAction) {
        hoveredAction = action;
        action->hover();
        update();

        // status tip
        MainWindow::instance()->statusBar()->showMessage(action->statusTip());
    }
}

void SegmentedControl::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (hoveredAction) {
        pressedAction = hoveredAction;
        update();
    }
}

void SegmentedControl::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    pressedAction = 0;
    if (hoveredAction) {
        bool changed = setCheckedAction(hoveredAction);
        if (changed) hoveredAction->trigger();
    }
}

void SegmentedControl::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    // status tip
    MainWindow::instance()->statusBar()->clearMessage();
    hoveredAction = 0;
    pressedAction = 0;
    update();
}

QAction *SegmentedControl::findHoveredAction(const QPoint& pos) const {
    if (pos.y() <= 0 || pos.y() >= height())
        return 0;

    int buttonWidth = width() / actionList.size();
    int buttonsWidth = width();
    int buttonsX = 0;

    if (pos.x() <= buttonsX || pos.x() >= (buttonsX + buttonsWidth))
        return 0;

    int buttonIndex = (pos.x() - buttonsX) / buttonWidth;

    if (buttonIndex >= actionList.size())
        return 0;
    return(actionList[buttonIndex]);
}

int SegmentedControl::calculateButtonWidth() const {
    QFontMetrics fontMetrics(font());
    int tmpItemWidth, itemWidth = 0;
    foreach (QAction *action, actionList) {
        tmpItemWidth = fontMetrics.width(action->text());
        if (itemWidth < tmpItemWidth) itemWidth = tmpItemWidth;
    }
    return itemWidth;
}

void SegmentedControl::drawButton (QPainter *painter,
                              const QRect& rect,
                              const QAction *action) {
    if (action == checkedAction)
        drawSelectedButton(painter, rect, action);
    else
        drawUnselectedButton(painter, rect, action);
}

void SegmentedControl::drawUnselectedButton (QPainter *painter,
                                        const QRect& rect,
                                        const QAction *action) {
    paintButton(painter, rect, action);
}

void SegmentedControl::drawSelectedButton (QPainter *painter,
                                      const QRect& rect,
                                      const QAction *action) {
    painter->save();
    painter->translate(rect.topLeft());

    const int width = rect.width();
    const int height = rect.height();
    painter->fillRect(0, 0, width, height, selectedColor);

    painter->restore();
    paintButton(painter, rect, action);
}

void SegmentedControl::paintButton(QPainter *painter, const QRect& rect, const QAction *action) {
    painter->save();
    painter->translate(rect.topLeft());

    const int height = rect.height();
    const int width = rect.width();

    painter->save();
    painter->setPen(Qt::NoPen);

    painter->drawRect(0, 0, width, height);
    painter->restore();

    const QString text = action->text();

    painter->setPen(palette().windowText().color());
    painter->drawText(0, 0, width, height, Qt::AlignCenter, text);

    if (action == pressedAction && action != checkedAction) {
        painter->fillRect(0, 0, width, height, QColor(0x00, 0x00, 0x00, 32));
    } else if (action == hoveredAction && action != checkedAction) {
        painter->fillRect(0, 0, width, height, QColor(0x00, 0x00, 0x00, 16));
    }

    if (action->property("notifyCount").isValid()) {
        QRect textBox = painter->boundingRect(rect,
                                              Qt::AlignCenter,
                                              text);
        painter->translate((width + textBox.width()) / 2 + 10, (height - textBox.height()) / 2);
        PainterUtils::paintBadge(painter, action->property("notifyCount").toString(), false, QColor(0,0,0,64));
    }

    painter->restore();
}
