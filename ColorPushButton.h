#ifndef COLORPUSHBUTTON_H
#define COLORPUSHBUTTON_H

#include <QPushButton>
#include <QColor>

/// \brief
///     The color push button is a command button whose background color represents the currently selected color
///     representented by this widget. The user can interact with the button to launch a color picker dialog which lets
///     the user select a different color to be represented by this widget.
class ColorPushButton : public QPushButton
{
    Q_OBJECT

public:

    /// \brief
    ///     Create a new ColorPushButton with a default QColor value.
    explicit ColorPushButton(QWidget* parent);

    /// \brief
    ///     Sets the color of the widget.
    void setColor(const QColor& color);

    /// \brief
    ///     Gets the color of the widget.
    const QColor& getColor() const;

signals:

    /// \brief
    ///     This signal is sent when the current color of the widget has changed either by the user or programatically.
    void valueChanged(QColor value);

private:

    /// Current color of the widget.
    QColor color;
};

#endif // COLORPUSHBUTTON_H
