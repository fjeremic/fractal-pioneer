#ifndef FRACTALPIONEER_H
#define FRACTALPIONEER_H

#include <QMainWindow>

#include "ui_FractalPioneer.h"

class FractalPioneer : public QMainWindow
{
    Q_OBJECT

public:
    /// \brief
    ///     Create a window showing the FractalWidget along with all control inputs.
    explicit FractalPioneer(QWidget* parent = nullptr);

private:

    /// The main window instance
    Ui::FractalPioneer ui;
};
#endif // FRACTALPIONEER_H
