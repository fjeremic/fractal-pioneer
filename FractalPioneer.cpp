#include "FractalPioneer.h"

#include <QDir>
#include <QFileDialog>
#include <QOpenGLShaderProgram>
#include <QtMath>

/// Current index into `preloadedWaypointLambdas` being animated/previewed
static size_t preloadedWaypointIndex = 0;

/// A hardcoded sequence of lambdas which when executed will setup the fractal scene with waypoints, durations,
/// keyframe start/end, etc. to be animated or previewed
static std::vector<std::function<void(Ui::FractalPioneer&)>> preloadedWaypointLambdas =
{
    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(0);
        ui.fractal->setOutputTargetDuration(30.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -0.4263f, 3.8537f,  0.0012f }, { -1.5708f, 1.5241f, 0.0227f });
        ui.fractal->addWaypoint({ -0.6596f, 2.9323f, -0.0066f }, { -0.9658f, 1.5591f, 0.0227f });
        ui.fractal->addWaypoint({ -0.9293f, 2.7172f, -0.0032f }, { -0.4158f, 1.5741f, 0.0227f });
        ui.fractal->addWaypoint({ -1.1480f, 2.6489f, -0.0021f }, { -0.2908f, 1.5791f, 0.0227f });

        ui.fractalColor->setColor(QColor(107, 97, 49));
    },
    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(15200);
        ui.fractal->setOutputTargetDuration(80.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -1.2249f, 2.6549f,  4.9257f }, { -0.5145f, -0.2654f, -0.0001f });
        ui.fractal->addWaypoint({ -0.7853f, 2.0185f,  3.3638f }, { -0.0995f, -0.2754f, -0.0001f });
        ui.fractal->addWaypoint({ -0.6051f, 2.2410f,  2.5274f }, {  0.4505f, -0.1954f, -0.0001f });
        ui.fractal->addWaypoint({ -0.5785f, 2.5718f,  2.0423f }, {  0.6855f,  0.0896f, -0.0001f });
        ui.fractal->addWaypoint({ -0.6372f, 2.7899f,  1.6986f }, {  0.2355f,  0.0696f, -0.0001f });
        ui.fractal->addWaypoint({ -0.5848f, 2.7727f,  1.0036f }, { -0.2445f, -0.1354f, -0.0001f });
        ui.fractal->addWaypoint({ -0.5437f, 2.6545f,  0.5407f }, { -0.3145f, -0.1504f, -0.0001f });
        ui.fractal->addWaypoint({ -0.2516f, 2.5000f,  0.1323f }, { -0.2845f, -0.9104f, -0.0001f });
        ui.fractal->addWaypoint({  0.3408f, 2.4094f, -0.0148f }, { -0.0045f, -1.6104f, -0.0001f });
        ui.fractal->addWaypoint({  0.8788f, 2.4490f, -0.0034f }, {  0.1555f, -1.5504f, -0.0001f });
        ui.fractal->addWaypoint({  1.1429f, 2.5484f,  0.0511f }, {  0.4905f, -2.2954f, -0.0001f });

        ui.fractalColor->setColor(QColor(255, 233, 228));
    },

    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(13400);
        ui.fractal->setOutputTargetDuration(60.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ 2.9407f, 1.7401f,  1.5511f }, {  0.1000f, -0.0400f, -1.0000f });
        ui.fractal->addWaypoint({ 2.8531f, 2.0577f,  1.1114f }, { -0.6050f, -0.5700f, -1.0000f });
        ui.fractal->addWaypoint({ 3.0181f, 2.3891f,  0.6425f }, { -1.5708f, -1.9800f, -1.0000f });
        ui.fractal->addWaypoint({ 2.8726f, 2.7665f,  0.4462f }, { -1.4008f, -3.2450f, -1.0000f });
        ui.fractal->addWaypoint({ 2.3794f, 2.3435f,  0.1638f }, { -0.2158f, -5.1800f,  0.0000f });
        ui.fractal->addWaypoint({ 1.9404f, 2.3968f, -0.0743f }, {  0.3092f, -5.2800f,  0.0000f });
        ui.fractal->addWaypoint({ 1.6924f, 2.4949f, -0.2324f }, {  0.3242f, -5.2850f,  0.0000f });

        ui.fractalColor->setColor(QColor(255, 233, 228));
    },

    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(10600);
        ui.fractal->setOutputTargetDuration(19.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -0.9148f, 2.3194f, 2.6425f }, {  0.3288f, -0.2672f,  0.0000f });
        ui.fractal->addWaypoint({ -0.9890f, 2.3783f, 2.4988f }, { -0.0212f, -1.6872f, -0.8300f });
        ui.fractal->addWaypoint({ -1.0620f, 2.4562f, 2.4283f }, { -0.2912f, -1.8522f, -0.8300f });

        ui.fractalColor->setColor(QColor(107, 97, 49));
    },

    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(9800);
        ui.fractal->setOutputTargetDuration(20.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -3.1948f, 1.9987f, -0.7018f }, { -0.3510f, -1.7920f, -0.0081f });
        ui.fractal->addWaypoint({ -3.5543f, 2.1470f, -0.9175f }, { -0.3160f, -2.5670f,  0.0000f });

        ui.fractalColor->setColor(QColor(107, 97, 49));
    },

    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(9300);
        ui.fractal->setOutputTargetDuration(80.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -2.5021f, 3.4674f, -1.9231f }, { -1.027f, 3.8919f,  0.0000f });
        ui.fractal->addWaypoint({ -2.1855f, 2.7128f, -1.5945f }, { -1.017f, 3.9269f,  0.0000f });
        ui.fractal->addWaypoint({ -2.0827f, 2.5088f, -1.5200f }, { -0.962f, 3.8469f,  0.0000f });
        ui.fractal->addWaypoint({ -2.0675f, 2.3624f, -1.4011f }, { -0.802f, 3.1169f,  0.0000f });
        ui.fractal->addWaypoint({ -2.1030f, 2.2254f, -1.2335f }, { -0.507f, 2.7569f,  0.0000f });
        ui.fractal->addWaypoint({ -2.1985f, 2.1652f, -1.0946f }, { -0.192f, 2.5019f,  0.0000f });
        ui.fractal->addWaypoint({ -2.3125f, 2.1607f, -0.9566f }, {  0.093f, 2.4719f,  0.0000f });
        ui.fractal->addWaypoint({ -2.6032f, 2.1634f, -0.7882f }, { -0.042f, 1.9719f, -0.3800f });

        ui.fractalColor->setColor(QColor(107, 97, 49));
    },

    [](Ui::FractalPioneer& ui)
    {
        ui.fractal->setFractalKeyframe(11300);
        ui.fractal->setOutputTargetDuration(60.0);

        ui.fractal->clearWayPoints();
        ui.fractal->addWaypoint({ -2.9284f, 1.8557f, 1.1644f }, { 0.1992f, 0.3609f, 0.0000f });
        ui.fractal->addWaypoint({ -2.6099f, 1.6737f, 2.0016f }, { 0.1992f, 0.3609f, 0.0000f });

        ui.fractalColor->setColor(QColor(255, 233, 228));
    },
};

FractalPioneer::FractalPioneer(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QObject::connect(ui.fractal, &FractalWidget::statusChanged,
        [=](const QString& message)
        {
            statusBar()->showMessage(message);
        });

    QObject::connect(ui.fractal, &FractalWidget::cameraPositionChaged,
        [=](const QVector3D& value)
        {
            ui.cameraPositionX->setValue(value.x());
            ui.cameraPositionY->setValue(value.y());
            ui.cameraPositionZ->setValue(value.z());
        });

    QObject::connect(ui.fractal, &FractalWidget::cameraRotationChaged,
        [=](const QVector3D& value)
        {
            ui.cameraRotationX->setValue(value.x());
            ui.cameraRotationY->setValue(value.y());
            ui.cameraRotationZ->setValue(value.z());
        });

    QObject::connect(ui.fractal, &FractalWidget::fractalKeyframeChanged,
        [=](const int32_t& value)
        {
            ui.fractalKeyframeSlider->setValue(value);

            auto text = QString::number(value);
            ui.fractalKeyframeText->setText(text);
        });

    QObject::connect(ui.fractal, &FractalWidget::animateKeyframesCancelled,
        [=]()
        {
            statusBar()->showMessage("Animation cancelled");
        });

    QObject::connect(ui.fractal, &FractalWidget::animateKeyframesFinished,
        [=]()
        {
            statusBar()->showMessage("Animation complete");
        });

    QObject::connect(ui.fractal, &FractalWidget::previewKeyframesCancelled,
        [=]()
        {
            statusBar()->showMessage("Preview cancelled");
        });

    QObject::connect(ui.fractal, &FractalWidget::previewKeyframesFinished,
        [=]()
        {
            if (ui.outputUsePreloadedWaypoints->isChecked()) {
                if (preloadedWaypointIndex < preloadedWaypointLambdas.size()) {
                    preloadedWaypointLambdas[preloadedWaypointIndex++](ui);
                    ui.fractal->previewKeyframes();
                }
            }
        });

    QObject::connect(ui.cameraPositionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = value;
            float y = ui.fractal->getCameraPosition().y();
            float z = ui.fractal->getCameraPosition().z();

            ui.fractal->setCameraPosition({x, y, z});
        });

    QObject::connect(ui.cameraPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractal->getCameraPosition().x();
            float y = value;
            float z = ui.fractal->getCameraPosition().z();

            ui.fractal->setCameraPosition({x, y, z});
        });

    QObject::connect(ui.cameraPositionZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractal->getCameraPosition().x();
            float y = ui.fractal->getCameraPosition().y();
            float z = value;

            ui.fractal->setCameraPosition({x, y, z});
        });

    QObject::connect(ui.cameraRotationX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = value;
            float y = ui.fractal->getCameraRotation().y();
            float z = ui.fractal->getCameraRotation().z();

            ui.fractal->setCameraRotation({x, y, z});
        });

    QObject::connect(ui.cameraRotationY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractal->getCameraRotation().x();
            float y = value;
            float z = ui.fractal->getCameraRotation().z();

            ui.fractal->setCameraRotation({x, y, z});
        });

    QObject::connect(ui.cameraRotationZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractal->getCameraRotation().x();
            float y = ui.fractal->getCameraRotation().y();
            float z = value;

            ui.fractal->setCameraRotation({x, y, z});
        });

    QObject::connect(ui.fractalScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setFractalScale(value);
        });

    QObject::connect(ui.fractalShiftX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = value;
            float y = ui.fractalShiftY->value();
            float z = ui.fractalShiftZ->value();

            ui.fractal->setFractalPosition({x, y, z});
        });

    QObject::connect(ui.fractalShiftY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractalShiftZ->value();
            float y = value;
            float z = ui.fractalShiftZ->value();

            ui.fractal->setFractalPosition({x, y, z});
        });

    QObject::connect(ui.fractalShiftZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractalShiftZ->value();
            float y = ui.fractalShiftY->value();
            float z = value;

            ui.fractal->setFractalPosition({x, y, z});
        });

    QObject::connect(ui.fractalRotationX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = value;
            float y = ui.fractalRotationY->value();
            float z = ui.fractalRotationZ->value();

            ui.fractal->setFractalRotation({x, y, z});
        });

    QObject::connect(ui.fractalRotationY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractalRotationX->value();
            float y = value;
            float z = ui.fractalRotationZ->value();

            ui.fractal->setFractalRotation({x, y, z});
        });

    QObject::connect(ui.fractalRotationZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            float x = ui.fractalRotationX->value();
            float y = ui.fractalRotationY->value();
            float z = value;

            ui.fractal->setFractalRotation({x, y, z});
        });

    QObject::connect(ui.fractalExposure, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setFractalExposure(value);
        });

    QObject::connect(ui.fractalColor, &ColorPushButton::valueChanged,
        [=](const QColor& value)
        {
            ui.fractal->setFractalColor(value);
        });

    QObject::connect(ui.fractalKeyframeSlider, QOverload<int32_t>::of(&QSlider::valueChanged),
        [=](const int32_t& value)
        {
            ui.fractal->setFractalKeyframe(value);
        });

    QObject::connect(ui.sceneAmbientOcclusionDelta, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneAmbientOcclusionDelta(value);
        });

    QObject::connect(ui.sceneAmbientOcclusionStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneAmbientOcclusionStrength(value);
        });

    QObject::connect(ui.sceneAntiAliasingSamples, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneAntiAliasingSamples(value);
        });

    QObject::connect(ui.sceneBackgroundColor, &ColorPushButton::valueChanged,
        [=](const QColor& value)
        {
            ui.fractal->setSceneBackgroundColor(value);
        });

    QObject::connect(ui.sceneDiffuseLighting, &QCheckBox::stateChanged,
        [=](const int32_t& value)
        {
            if (value == Qt::Checked) {
                ui.fractal->setSceneDiffuseLighting(true);
                ui.sceneDiffuseLighting->setText("Enabled");
            } else {
                ui.fractal->setSceneDiffuseLighting(false);
                ui.sceneDiffuseLighting->setText("Disabled");
            }
        });

    QObject::connect(ui.sceneFiltering, &QCheckBox::stateChanged,
        [=](const int32_t& value)
        {
            if (value == Qt::Checked) {
                ui.fractal->setSceneFiltering(true);
                ui.sceneFiltering->setText("Enabled");
            } else {
                ui.fractal->setSceneFiltering(false);
                ui.sceneFiltering->setText("Disabled");
            }
        });

    QObject::connect(ui.sceneFocalDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneFocalDistance(value);
        });

    QObject::connect(ui.sceneFog, &QCheckBox::stateChanged,
        [=](const int32_t& value)
        {
            if (value == Qt::Checked) {
                ui.fractal->setSceneFog(true);
                ui.sceneFog->setText("Enabled");
            } else {
                ui.fractal->setSceneFog(false);
                ui.sceneFog->setText("Disabled");
            }
        });

    QObject::connect(ui.sceneLightColor, &ColorPushButton::valueChanged,
        [=](const QColor& value)
        {
            ui.fractal->setSceneLightColor(value);
        });

    QObject::connect(ui.sceneLightDirection, &QPushButton::clicked,
        [=](const bool&)
        {
            auto lookDirection = ui.fractal->getLookDirectionFromCamera();
            ui.fractal->setSceneLightDirection(lookDirection);
        });

    QObject::connect(ui.sceneShadows, &QCheckBox::stateChanged,
        [=](const int32_t& value)
        {
            if (value == Qt::Checked) {
                ui.fractal->setSceneShadows(true);
                ui.sceneShadows->setText("Enabled");
            } else {
                ui.fractal->setSceneShadows(false);
                ui.sceneShadows->setText("Disabled");
            }
        });

    QObject::connect(ui.sceneShadowDarkness, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneShadowDarkness(value);
        });

    QObject::connect(ui.sceneShadowSharpness, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneShadowSharpness(value);
        });

    QObject::connect(ui.sceneSpecularHighlight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneSpecularHighlight(value);
        });

    QObject::connect(ui.sceneSpecularMultiplier, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setSceneSpecularMultiplier(value);
        });

    QObject::connect(ui.outputResultion, &QComboBox::textActivated,
        [=](const QString& text)
        {
            QStringList resolutionSplit = text.split(' ', Qt::SkipEmptyParts);

            auto w = resolutionSplit[0].toFloat();
            auto h = resolutionSplit[2].toFloat();

            ui.fractal->setOutputResultion({w, h});
        });

    QObject::connect(ui.outputTargetFPS, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setOutputTargetFPS(value);
        });

    QObject::connect(ui.outputTargetDuration, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](const double& value)
        {
            ui.fractal->setOutputTargetDuration(value);
        });

    QObject::connect(ui.outputDirectoryBrowse, &QPushButton::clicked,
        [=](const bool&)
        {
            QString directory = QFileDialog::getExistingDirectory(this, "Output Directory", QDir::currentPath());

            if (!directory.isEmpty()) {
                auto index = ui.outputDirectory->findText(directory);
                if (ui.outputDirectory->findText(directory) == -1) {
                    ui.outputDirectory->addItem(directory);

                    index = ui.outputDirectory->count() - 1;
                }

                ui.outputDirectory->setCurrentIndex(index);
            }
        });

    QObject::connect(ui.outputDirectory, QOverload<int32_t>::of(&QComboBox::currentIndexChanged),
        [=](const int32_t& value)
        {
            auto text = ui.outputDirectory->itemText(value);
            ui.fractal->setOutputDirectory(text);
        });

    QObject::connect(ui.outputUsePreloadedWaypoints, &QCheckBox::stateChanged,
        [=](const int32_t& value)
        {
            if (value == Qt::Checked) {
                ui.outputUsePreloadedWaypoints->setText("Enabled");
            } else {
                ui.outputUsePreloadedWaypoints->setText("Disabled");
            }
        });

    QObject::connect(ui.outputAnimateKeyframes, &QPushButton::clicked,
        [=](const bool&)
        {
            if (ui.outputUsePreloadedWaypoints->isChecked()) {
                preloadedWaypointIndex = 0;

                if (preloadedWaypointIndex < preloadedWaypointLambdas.size()) {
                    preloadedWaypointLambdas[preloadedWaypointIndex++](ui);
                }
            }

            ui.fractal->animateKeyframes();
        });

    QObject::connect(ui.outputPreviewKeyframes, &QPushButton::clicked,
        [=](const bool&)
        {
            if (ui.outputUsePreloadedWaypoints->isChecked()) {
                preloadedWaypointIndex = 0;

                if (preloadedWaypointIndex < preloadedWaypointLambdas.size()) {
                    preloadedWaypointLambdas[preloadedWaypointIndex++](ui);
                }
            }

            ui.fractal->previewKeyframes();
        });

    // Initialize some aesthetically pleasing initial values
    ui.cameraPositionY->setValue(1.32);
    ui.cameraPositionZ->setValue(3.46);
    ui.cameraPositionX->setValue(2.80);

    ui.fractalScale->setValue(1.77);
    ui.fractalShiftX->setValue(-2.08);
    ui.fractalShiftY->setValue(-1.42);
    ui.fractalShiftZ->setValue(-1.93);
    ui.fractalRotationX->setValue(5.52);
    ui.fractalRotationY->setValue(0.00);
    ui.fractalRotationZ->setValue(-0.22);
    ui.fractalExposure->setValue(1.0);
    ui.fractalColor->setColor(QColor(107, 97, 49));
    ui.fractalKeyframeSlider->setMinimum(0);
    ui.fractalKeyframeSlider->setMaximum(2 * M_PI / FractalWidget::ANIMATION_SIN_INNER_FACTOR);
    ui.fractalKeyframeSlider->setValue(0);

    ui.sceneAmbientOcclusionDelta->setValue(0.7);
    ui.sceneAmbientOcclusionStrength->setValue(0.008);
    ui.sceneAntiAliasingSamples->setValue(2);
    ui.sceneBackgroundColor->setColor(QColor(31, 31, 31));
    ui.sceneDiffuseLighting->setCheckState(Qt::Checked);
    ui.sceneFiltering->setCheckState(Qt::Checked);
    ui.sceneFocalDistance->setValue(1.73205080757);
    ui.sceneFog->setCheckState(Qt::Checked);
    ui.sceneLightColor->setColor(QColor(255, 255, 126));
    ui.sceneShadows->setCheckState(Qt::Checked);
    ui.sceneShadowDarkness->setValue(0.9);
    ui.sceneShadowSharpness->setValue(10.0);
    ui.sceneSpecularHighlight->setValue(40);
    ui.sceneSpecularMultiplier->setValue(0.25);

    ui.outputTargetFPS->setValue(60);
    ui.outputTargetDuration->setValue(10);

    ui.fractal->setSceneLightDirection({-0.36f, 0.8f, 0.48f});
}
