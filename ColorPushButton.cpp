#include "ColorPushButton.h"

#include <QColorDialog>

ColorPushButton::ColorPushButton(QWidget*)
{
    QObject::connect(this, &ColorPushButton::clicked,
        [=]()
        {
            QColor newColor = QColorDialog::getColor(color, parentWidget());
            if (newColor != color) {
                setColor(newColor);
            }
        });
}

void ColorPushButton::setColor(const QColor& value)
{
    if (color != value) {
        color = value;
        setStyleSheet("border: 20px; background-color: " + color.name() + ";");

        emit valueChanged(color);
    }
}

const QColor& ColorPushButton::getColor() const
{
    return color;
}
