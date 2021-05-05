#include "FractalWidget.h"

#include <algorithm>
#include <QApplication>
#include <QDir>
#include <QMouseEvent>
#include <QScreen>
#include <QtMath>
#include <QTimerEvent>
#include <QQuaternion>

static QQuaternion exp(QQuaternion q)
{
    auto x = q.x();
    auto y = q.y();
    auto z = q.z();
    auto w = q.scalar();

    auto immaginaryNorm = std::sqrt((x * x) + (y * y) + (z * z));

    auto r = std::exp(w) * std::cos(immaginaryNorm);
    auto i = 0;
    auto j = 0;
    auto k = 0;

    // Avoid division by 0
    if (immaginaryNorm == 0) {
        return QQuaternion(r, i, j, k);
    } else {
        i = std::exp(w) * (x * std::sin(immaginaryNorm)) / immaginaryNorm;
        j = std::exp(w) * (y * std::sin(immaginaryNorm)) / immaginaryNorm;
        k = std::exp(w) * (z * std::sin(immaginaryNorm)) / immaginaryNorm;

        return QQuaternion(r, i, j, k);
    }
}

/// \note
///     This function assumes a branch cut (-inf, 0]
static QQuaternion log(QQuaternion q)
{
    auto x = q.x();
    auto y = q.y();
    auto z = q.z();
    auto w = q.scalar();

    auto immaginaryNorm = std::sqrt((x * x) + (y * y) + (z * z));

    // Avoid division by 0
    if (immaginaryNorm == 0) {
        auto r = std::log(q.length());
        auto i = x * std::atan2(immaginaryNorm, w);
        auto j = y * std::atan2(immaginaryNorm, w);
        auto k = z * std::atan2(immaginaryNorm, w);

        return QQuaternion(r, i, j, k);
    } else {
        auto r = std::log(q.length());
        auto i = (x * std::atan2(immaginaryNorm, w)) / immaginaryNorm;
        auto j = (y * std::atan2(immaginaryNorm, w)) / immaginaryNorm;
        auto k = (z * std::atan2(immaginaryNorm, w)) / immaginaryNorm;

        return QQuaternion(r, i, j, k);
    }
}

static QMatrix3x3 toRotationMatrix(QVector3D r)
{
    auto rx = QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, r.x() / (M_PI / 180.0f));
    auto ry = QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, r.y() / (M_PI / 180.0f));
    auto rz = QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, r.z() / (M_PI / 180.0f));

    return (rx * ry * rz).toRotationMatrix();
}

void FractalWidget::blend()
{
    // https://en.wikipedia.org/wiki/Gaussian_quadrature
    auto const gaussianQuadrature = [this](float a, float b) -> float
    {
        // Precalculated 5th order Gauss–Legendre quadrature coefficients
        static constexpr std::pair<float,float> coefficients[] =
        {
            {  0.00000000f, 0.56888890f },
            { -0.53846930f, 0.47862867f },
            {  0.53846930f, 0.47862867f },
            { -0.90617985f, 0.23692688f },
            {  0.90617985f, 0.23692688f },
        };

        float s = 0.0f;

        // Change of interval formula
        for (auto [xi, wi] : coefficients) {
            s += wi * interpolatePosition(((b - a) / 2 * xi) + ((b + a) / 2), true).length();
        }

        return s * ((b - a) / 2);
    };

    s2uTable.clear();

    float s = 0.0f;
    float u = 0.0f;

    while (u < positionWaypoints.size() - 1) {
        s2uTable.append({s, u});

        s += gaussianQuadrature(u, u + 0.01f);
        u += 0.01f;
    }
}

float FractalWidget::s2u(float s)
{
    auto const comp = [](const decltype(s2uTable)::value_type& a, const decltype(s2uTable)::value_type& b) -> bool
    {
        return a.first < b.first;
    };

    auto i1 = std::upper_bound(s2uTable.begin(), s2uTable.end() - 1, QPair(s, 0.0f), comp);
    auto i0 = i1--;

    auto u0 = i0->second;
    auto u1 = i1->second;

    auto s0 = i0->first;
    auto s1 = i1->first;

    // https://en.wikipedia.org/wiki/Linear_interpolation#Linear_interpolation_between_two_known_points
    auto a = (s - s0) / (s1 - s0);

    return (1 - a) * u0 + a * u1;
}

QVector3D FractalWidget::interpolatePosition(float t, bool takeDerivative)
{
    if (t <= 0) {
        return positionWaypoints.first();
    }

    if (t >= positionWaypoints.size() - 1) {
        return positionWaypoints.last();
    }

    const int32_t i = std::floor(t);

    float u_0 = 1.0f;
    float u_1 = t - i;
    float u_2 = u_1 * u_1;
    float u_3 = u_2 * u_1;

    if (takeDerivative) {
        u_0 = 0.0f;
        u_1 = 1.0f;
        u_2 = 2 * (t - i);
        u_3 = 3 * (t - i) * (t - i);
    }

    QVector4D u(u_0, u_1, u_2, u_3);

    QMatrix4x4 B(0, -1, 2, -1, 2, 0, -5, 3, 0, 1, 4, -3, 0, 0, -1, 1);
    QMatrix4x4 G;

    // Catmull-Rom splines require at least four points for interpolation. In reality we should be able to interpolate
    // between two points in 3D space, i.e. the interpolation should be a straight line. To handle this situation we
    // use the recorded look direction to compute two additional points; one at the start and one at the end, which we
    // will use as the interpolation control points. Using the look directions ensures that the tangent at the start
    // and end points is identical to the look direction, which will ensure we end up at the same positions and
    // rotations recorded.

    QVector3D column0 = (i != 0) ?
        positionWaypoints[i - 1] :
        positionWaypoints[i + 0] - getLookDirectionFromRotation(rotationWaypoints[i + 0]);

    QVector3D column1 = positionWaypoints[i + 0];
    QVector3D column2 = positionWaypoints[i + 1];

    QVector3D column3 = (i != positionWaypoints.size() - 2) ?
        positionWaypoints[i + 2] :
        positionWaypoints[i + 1] + getLookDirectionFromRotation(rotationWaypoints[i + 1]);

    G.setColumn(0, {column0, 0});
    G.setColumn(1, {column1, 0});
    G.setColumn(2, {column2, 0});
    G.setColumn(3, {column3, 0});

    const float tau = 0.5f;

    return (G * B * tau * u).toVector3D();
}

QVector3D FractalWidget::interpolateRotation(float t)
{
    if (t <= 0) {
        return rotationWaypoints.first();
    }

    if (t >= rotationWaypoints.size() - 1) {
        return rotationWaypoints.last();
    }

    const int32_t i = std::floor(t);

    // fromEulerAngles
    QQuaternion qi0 =
        QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, rotationWaypoints[i].y() / (M_PI / 180.0f)) *
        QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, rotationWaypoints[i].x() / (M_PI / 180.0f)) *
        QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, rotationWaypoints[i].z() / (M_PI / 180.0f));

    QQuaternion qi1 =
        QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, rotationWaypoints[i + 1].y() / (M_PI / 180.0f)) *
        QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, rotationWaypoints[i + 1].x() / (M_PI / 180.0f)) *
        QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, rotationWaypoints[i + 1].z() / (M_PI / 180.0f));

    QQuaternion si0;

    if (i <= 1) {
        si0 = qi0;
    } else {
        QQuaternion qim1 =
            QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, rotationWaypoints[i - 1].y() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, rotationWaypoints[i - 1].x() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, rotationWaypoints[i - 1].z() / (M_PI / 180.0f));

        // Section 6.2.1, Definition 17, (6.15) pg. 51 of [1]
        si0 = qi0 * exp(-(log(qi0.inverted() * qi1) + log(qi0.inverted() * qim1)) / 4);
    }

    QQuaternion si1;

    if (i >= rotationWaypoints.size() - 3) {
        si1 =
            QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, rotationWaypoints.last().y() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, rotationWaypoints.last().x() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, rotationWaypoints.last().z() / (M_PI / 180.0f));
    } else {
        QQuaternion qip2 =
            QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, rotationWaypoints[i + 2].y() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, rotationWaypoints[i + 2].x() / (M_PI / 180.0f)) *
            QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, rotationWaypoints[i + 2].z() / (M_PI / 180.0f));

        // Section 6.2.1, Definition 17, (6.15) pg. 51 of [1]
        si1 = qi1 * exp(-(log(qi1.inverted() * qip2) + log(qi1.inverted() * qi0)) / 4);
    }

    auto h = t - i;

    // Section 6.2.1, Definition 17, (6.14) pg. 51 of [1]
    auto squad = QQuaternion::slerp(QQuaternion::slerp(qi0, qi1, h), QQuaternion::slerp(si0, si1, h), 2 * h * (1 - h));

    return squad.toEulerAngles() * static_cast<float>(M_PI / 180.0f);
}

FractalWidget::FractalWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    setFormat(QSurfaceFormat::defaultFormat());
}

void FractalWidget::animateKeyframes()
{
    if (!animateKeyframesActive && !previewKeyframesActive) {
        grabKeyboard();

        if (positionWaypoints.size() > 1) {
            blend();

            fractalKeyframeBegin = fractalKeyframeCurrent;
            animateKeyframesActive = true;
        }

        auto drawnFrames = QDir(outputDirectory).entryInfoList({ "*.png" }, QDir::Files, QDir::SortFlag::Time);
        if (drawnFrames.size() > 0) {
            outputLastDrawnFrame = drawnFrames.last().baseName().toLong();
        } else {
            outputLastDrawnFrame = 0;
        }
    }
}

void FractalWidget::previewKeyframes()
{
    if (!animateKeyframesActive && !previewKeyframesActive) {
        grabKeyboard();

        if (positionWaypoints.size() > 1) {
            blend();

            fractalKeyframeBegin = fractalKeyframeCurrent;
            previewKeyframesActive = true;
        }
    }
}

void FractalWidget::addWaypoint()
{
    addWaypoint(cameraPosition, cameraRotation);
}

void FractalWidget::addWaypoint(QVector3D position, QVector3D rotation)
{
    if (!animateKeyframesActive && !previewKeyframesActive) {
        positionWaypoints.append(position);
        rotationWaypoints.append(rotation);
    }
}

void FractalWidget::clearWayPoints()
{
    if (!animateKeyframesActive && !previewKeyframesActive) {
        positionWaypoints.clear();
        rotationWaypoints.clear();
    }
}

const QVector3D FractalWidget::getLookDirectionFromCamera() const
{
    return getLookDirectionFromRotation(cameraRotation);
}

const QVector3D FractalWidget::getLookDirectionFromRotation(QVector3D rotation) const
{
    auto rotationMatrix = toRotationMatrix(rotation);
    return QVector3D(rotationMatrix(0, 2), rotationMatrix(1, 2), rotationMatrix(2, 2));
}

const QVector3D FractalWidget::getCameraPosition() const
{
    return cameraPosition;
}

const QVector3D FractalWidget::getCameraRotation() const
{
    return cameraRotation;
}

void FractalWidget::setCameraPosition(QVector3D value)
{
    auto x = QString::number(value.x(), 'f', 4).toFloat();
    auto y = QString::number(value.y(), 'f', 4).toFloat();
    auto z = QString::number(value.z(), 'f', 4).toFloat();

    value.setX(x);
    value.setY(y);
    value.setZ(z);

    if (cameraPosition != value) {
        cameraPosition = value;
        emit cameraPositionChaged(cameraPosition);
    }
}

void FractalWidget::setCameraRotation(QVector3D value)
{
    auto x = QString::number(value.x(), 'f', 4).toFloat();
    auto y = QString::number(value.y(), 'f', 4).toFloat();
    auto z = QString::number(value.z(), 'f', 4).toFloat();

    // Clamp the rotation to within (-2pi, 2pi) radians
    x = std::fmod(x, static_cast<float>(2 * M_PI));
    y = std::fmod(y, static_cast<float>(2 * M_PI));
    z = std::fmod(z, static_cast<float>(2 * M_PI));

    value.setX(x);
    value.setY(y);
    value.setZ(z);

    if (cameraRotation != value) {
        cameraRotation = value;

        auto rx = QQuaternion::fromAxisAndAngle({1.0f, 0.0f, 0.0f}, cameraRotation.x() / (M_PI / 180.0f));
        auto ry = QQuaternion::fromAxisAndAngle({0.0f, 1.0f, 0.0f}, cameraRotation.y() / (M_PI / 180.0f));
        auto rz = QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, cameraRotation.z() / (M_PI / 180.0f));

        // Quaternion multiplication is not commutative. We want our camera to be a first person view and not a flight
        // simulator camera. As such we want to rotate through the y-axis first and then through the x-axis. That is,
        // we want to fix the y-axis to be the natural gravitational y-axis.
        //
        // We can make a simple example with head rotations. Pretend you had a virtual stick going through your ears
        // which represents the x-axis. Similarly pretend you had a virtual stick going through the top of your head
        // and through your neck which represents the y-axis. Rotate the y-axis stick to the right. The x-axis stick
        // will rotate with your head. Now rotate the x-axis stick to the right. This will make your head tilt down.
        //
        // Now let's do the opposite. Rotate the x-axis stick to the right. Your head should tilt down. The y-axis
        // stick will rotate with your head and should no longer be in the gravitational vertical position. Now rotate
        // the y-axis stick to the right. Your neck should tilt in an awkward way. The rotation you end up with will
        // not be the same as the previous exercise.
        //
        // As an exercise, switch the order of multiplication in the line below and test how the camera behaves.
        cameraRotationMatrix = (ry * rx * rz).toRotationMatrix();

        emit cameraRotationChaged(cameraRotation);
    }
}

void FractalWidget::setFractalScale(float value)
{
    fractalScale = value;
}

void FractalWidget::setFractalPosition(QVector3D value)
{
    fractalPosition = value;
}

void FractalWidget::setFractalRotation(QVector3D value)
{
    fractalRotation = value;
}

void FractalWidget::setFractalExposure(float value)
{
    fractalExposure = value;
}

void FractalWidget::setFractalColor(QColor value)
{
    fractalColor = QVector3D(value.redF(), value.greenF(), value.blueF());
}

void FractalWidget::setFractalKeyframe(int32_t value)
{
    fractalKeyframeCurrent = value % static_cast<int32_t>(2 * M_PI / ANIMATION_SIN_INNER_FACTOR);

    emit fractalKeyframeChanged(fractalKeyframeCurrent);
}

void FractalWidget::setSceneAmbientOcclusionDelta(float value)
{
    sceneAmbientOcclusionDelta = value;
}

void FractalWidget::setSceneAmbientOcclusionStrength(float value)
{
    sceneAmbientOcclusionStrength = value;
}

void FractalWidget::setSceneAntiAliasingSamples(float value)
{
    if (value >= 0) {
        sceneAntiAliasingSamples = value;
    } else {
        emit statusChanged("Cannot set scene anti-aliasing to a negative value");
    }
}

void FractalWidget::setSceneBackgroundColor(QColor value)
{
    sceneBackgroundColor = QVector3D(value.redF(), value.greenF(), value.blueF());
}

void FractalWidget::setSceneDiffuseLighting(bool value)
{
    sceneDiffuseLighting = value;
}

void FractalWidget::setSceneFiltering(bool value)
{
    sceneFiltering = value;
}

void FractalWidget::setSceneFocalDistance(float value)
{
    sceneFocalDistance = value;
}

void FractalWidget::setSceneFog(bool value)
{
    sceneFog = value;
}

void FractalWidget::setSceneLightColor(QColor value)
{
    sceneLightColor = QVector3D(value.redF(), value.greenF(), value.blueF());
}

void FractalWidget::setSceneLightDirection(QVector3D value)
{
    sceneLightDirection = value;
}

void FractalWidget::setSceneShadows(bool value)
{
    sceneShadows = value;
}

void FractalWidget::setSceneShadowDarkness(float value)
{
    sceneShadowDarkness = value;
}

void FractalWidget::setSceneShadowSharpness(float value)
{
    sceneShadowSharpness = value;
}

void FractalWidget::setSceneSpecularHighlight(float value)
{
    sceneSpecularHighlight = value;
}

void FractalWidget::setSceneSpecularMultiplier(float value)
{
    sceneSpecularMultiplier = value;
}

void FractalWidget::setOutputResultion(QVector2D value)
{
    if (value.x() > 0 && value.y() > 0) {
        outputResolution = value;
    } else {
        emit statusChanged("Cannot set output resolution to a negative value");
    }
}

void FractalWidget::setOutputTargetFPS(float value)
{
    if (value >= 0) {
        outputTargetFPS = value;
    } else {
        emit statusChanged("Cannot set output target FPS to a negative value");
    }
}

void FractalWidget::setOutputTargetDuration(float value)
{
    if (value >= 0) {
        outputTargetDuration = value;
    } else {
        emit statusChanged("Cannot set output target duration to a negative value");
    }
}

void FractalWidget::setOutputDirectory(QString value)
{
    const QFileInfo directory(value);
    if (directory.exists() && directory.isDir() && directory.isWritable()) {
        outputDirectory = value;
    } else {
        emit statusChanged("Cannot set directory to \"" + value + "\" because it does not exist or it is not writable");
    }
}

void FractalWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // Set global information
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create shaders
    fractalOSP.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vert.glsl");
    fractalOSP.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/frag.glsl");
    fractalOSP.link();
    fractalOSP.bind();

    // Create Vertex Buffer Object (VBO)
    fractalVBO.create();
    fractalVBO.bind();
    fractalVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);

    // Create Vertex Array Object (VAO)
    fractalVAO.create();

    fractalVBO.release();
    fractalOSP.release();
}

void FractalWidget::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_W:
    case Qt::Key_Up:
        keyMap[Qt::Key_W] = true;
        break;

    case Qt::Key_A:
    case Qt::Key_Left:
        keyMap[Qt::Key_A] = true;
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        keyMap[Qt::Key_S] = true;
        break;

    case Qt::Key_D:
    case Qt::Key_Right:
        keyMap[Qt::Key_D] = true;
        break;

    case Qt::Key_Q:
        keyMap[Qt::Key_Q] = true;
        break;

    case Qt::Key_E:
        keyMap[Qt::Key_E] = true;
        break;

    case Qt::Key_Space:
        addWaypoint();
        break;

    case Qt::Key_Backspace:
        if (!animateKeyframesActive && !previewKeyframesActive) {
            if (!positionWaypoints.isEmpty()) {
                positionWaypoints.removeLast();
            }

            if (!positionWaypoints.isEmpty()) {
                rotationWaypoints.removeLast();
            }
        }
        break;

    case Qt::Key_Delete:
        clearWayPoints();
        break;

    case Qt::Key_Escape:
        setMouseTracking(false);
        releaseMouse();
        releaseKeyboard();

        if (animateKeyframesActive) {
            animateKeyframesActive = false;
            emit animateKeyframesCancelled();
        }

        if (previewKeyframesActive) {
            previewKeyframesActive = false;
            emit previewKeyframesCancelled();
        }
        break;

    default:
        QOpenGLWidget::keyPressEvent(e);
    }
}

void FractalWidget::keyReleaseEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_W:
    case Qt::Key_Up:
        keyMap[Qt::Key_W] = false;
        break;

    case Qt::Key_A:
    case Qt::Key_Left:
        keyMap[Qt::Key_A] = false;
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        keyMap[Qt::Key_S] = false;
        break;

    case Qt::Key_D:
    case Qt::Key_Right:
        keyMap[Qt::Key_D] = false;
        break;

    case Qt::Key_Q:
        keyMap[Qt::Key_Q] = false;
        break;

    case Qt::Key_E:
        keyMap[Qt::Key_E] = false;
        break;

    default:
        QOpenGLWidget::keyReleaseEvent(e);
    }
}

void FractalWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        setMouseTracking(true);
        grabMouse(Qt::BlankCursor);
        grabKeyboard();

        auto widgetCenterInGlobalCoords = mapToGlobal({width() / 2, height() / 2});
        QCursor::setPos(widgetCenterInGlobalCoords);
    }
}

void FractalWidget::paintGL()
{
    updatePhysics();
    updateVisuals();

    glClear(GL_COLOR_BUFFER_BIT);

    // Render using our shader
    fractalOSP.bind();
    fractalVAO.bind();

    glDrawArrays(GL_TRIANGLES, 0, 12);

    fractalVAO.release();
    fractalOSP.release();

    if (animateKeyframesActive) {
        resizeGL(outputResolution.x(), outputResolution.y());

        QOpenGLFramebufferObject fractalFBO (outputResolution.x(), outputResolution.y());

        // Render using our shader
        fractalOSP.bind();
        fractalVAO.bind();
        fractalFBO.bind();

        glDrawArrays(GL_TRIANGLES, 0, 12);

        QFileInfo frameFile(outputDirectory, QString::number(outputLastDrawnFrame++) + QString(".png"));
        fractalFBO.toImage().save(frameFile.absoluteFilePath());

        fractalFBO.release();
        fractalVAO.release();
        fractalOSP.release();

        resizeGL(width(), height());
    }
}

void FractalWidget::resizeGL(int w, int h)
{
    const qreal retinaScale = devicePixelRatioF();
    const qreal retinaW = w * retinaScale;
    const qreal retinaH = h * retinaScale;

    glViewport(0, 0, retinaW, retinaH);

    fractalOSP.bind();
    fractalVBO.bind();
    fractalVAO.bind();

    // Create a rectangle using two triangles which covers the entire viewport
    GLfloat vertices[] =
    {
        // 1st triangle
        static_cast<GLfloat>(-retinaW), static_cast<GLfloat>(+retinaH),
        static_cast<GLfloat>(+retinaW), static_cast<GLfloat>(+retinaH),
        static_cast<GLfloat>(+retinaW), static_cast<GLfloat>(-retinaH),

        // 2nd triangle
        static_cast<GLfloat>(+retinaW), static_cast<GLfloat>(-retinaH),
        static_cast<GLfloat>(-retinaW), static_cast<GLfloat>(-retinaH),
        static_cast<GLfloat>(-retinaW), static_cast<GLfloat>(+retinaH),
    };

    fractalVBO.allocate(vertices, sizeof(vertices));

    fractalOSP.enableAttributeArray(0);
    fractalOSP.setAttributeBuffer(0, GL_FLOAT, sizeof(GLfloat) * 0, 2, sizeof(GLfloat) * 2);

    fractalOSP.setUniformValue("in_resolution", QVector2D(retinaW, retinaH));

    // Release (unbind) all
    fractalVAO.release();
    fractalVBO.release();
    fractalOSP.release();
}

void FractalWidget::updatePhysics()
{
    // Mouse tracking determines whether the user has clicked on the fractal widget and wants to move and rotate the camera
    if (hasMouseTracking()) {
        float dx = 0.0f;
        float dy = 0.0f;

        if (keyMap[Qt::Key_W]) {
            dx += 1.0f;
        }

        if (keyMap[Qt::Key_A]) {
            dy -= 1.0f;
        }

        if (keyMap[Qt::Key_S]) {
            dx -= 1.0f;
        }

        if (keyMap[Qt::Key_D]) {
            dy += 1.0f;
        }

        // Normalize force if too big
        const float mag2 = dx * dx + dy * dy;
        if (mag2 > 1.0f) {
            const float mag = std::sqrt(mag2);
            dx /= mag;
            dy /= mag;
        }

        auto xAxis = QVector3D(cameraRotationMatrix(0, 0), cameraRotationMatrix(1, 0), cameraRotationMatrix(2, 0));
        auto zAxis = QVector3D(cameraRotationMatrix(0, 2), cameraRotationMatrix(1, 2), cameraRotationMatrix(2, 2));

        auto newCameraPosition = cameraPosition;
        newCameraPosition += (xAxis * (dy * +0.01f));
        newCameraPosition += (zAxis * (dx * -0.01f));

        setCameraPosition(newCameraPosition);

        auto widgetCenterInGlobalCoords = mapToGlobal({width() / 2, height() / 2});
        float rx = (widgetCenterInGlobalCoords - QCursor::pos()).x() * 0.005f;
        float ry = (widgetCenterInGlobalCoords - QCursor::pos()).y() * 0.005f;

        QCursor::setPos(widgetCenterInGlobalCoords);

        float rz = 0.0f;

        if (keyMap[Qt::Key_Q]) {
            rz += 0.01f;
        }

        if (keyMap[Qt::Key_E]) {
            rz -= 0.01f;
        }

        auto newCameraRotation = cameraRotation + QVector3D(ry, rx, rz);

        // Restrict x-axis rotation to 180 degrees
        auto x = std::clamp(newCameraRotation.x(), static_cast<float>(-M_PI_2), static_cast<float>(M_PI_2));
        newCameraRotation.setX(x);

        setCameraRotation(newCameraRotation);
    } else if (animateKeyframesActive || previewKeyframesActive) {
        float elapsed = (fractalKeyframeCurrent - fractalKeyframeBegin) * (1000.0f / outputTargetFPS);

        float arclength = s2uTable.last().first;
        float arclengthPerSecond = arclength / outputTargetDuration;
        float arclengthPerMillisecond = arclengthPerSecond / 1000.0f;

        float u = s2u(elapsed * arclengthPerMillisecond);

        auto interpolatedPosition = interpolatePosition(u, false);
        setCameraPosition(interpolatedPosition);

        auto interpolatedRotation = interpolateRotation(u);
        setCameraRotation(interpolatedRotation);

        if (elapsed > outputTargetDuration * 1000.0f) {
            if (animateKeyframesActive) {
                animateKeyframesActive = false;
                emit animateKeyframesFinished();
            }

            if (previewKeyframesActive) {
                previewKeyframesActive = false;
                emit previewKeyframesFinished();
            }
        } else {
            auto status = QString("Animating keyframes: %1 / %2 (s)")
                .arg(QString::number(elapsed / 1000.0f, 'f', 2))
                .arg(QString::number(outputTargetDuration, 'f', 2));

            emit statusChanged(status);
        }
    }
}

void FractalWidget::updateVisuals()
{
    // Update animated fractals
    QVector3D animatedRotation = fractalRotation;

    if (true || animateKeyframesActive || previewKeyframesActive) {
        animatedRotation.setX(animatedRotation.x() + ANIMATION_SIN_OUTER_FACTOR * std::sin(fractalKeyframeCurrent * ANIMATION_SIN_INNER_FACTOR));

        setFractalKeyframe(fractalKeyframeCurrent + 1);
    }

    fractalOSP.bind();
    fractalOSP.setUniformValue("in_camera_position", cameraPosition);
    fractalOSP.setUniformValue("in_camera_rotation", cameraRotationMatrix);

    fractalOSP.setUniformValue("in_fractal_scale", fractalScale);
    fractalOSP.setUniformValue("in_fractal_rotation", animatedRotation);
    fractalOSP.setUniformValue("in_fractal_shift", fractalPosition);
    fractalOSP.setUniformValue("in_fractal_exposure", fractalExposure);
    fractalOSP.setUniformValue("in_fractal_color", fractalColor);

    fractalOSP.setUniformValue("in_scene_ambient_occlusion_delta", sceneAmbientOcclusionDelta);
    fractalOSP.setUniformValue("in_scene_ambient_occlusion_strength", sceneAmbientOcclusionStrength);
    fractalOSP.setUniformValue("in_scene_anti_aliasing_samples", sceneAntiAliasingSamples);
    fractalOSP.setUniformValue("in_scene_background_color", sceneBackgroundColor);
    fractalOSP.setUniformValue("in_scene_diffuse_lighting", sceneDiffuseLighting);
    fractalOSP.setUniformValue("in_scene_filtering", sceneFiltering);
    fractalOSP.setUniformValue("in_scene_focal_distance", sceneFocalDistance);
    fractalOSP.setUniformValue("in_scene_fog", sceneFog);
    fractalOSP.setUniformValue("in_scene_light_color", sceneLightColor);
    fractalOSP.setUniformValue("in_scene_light_direction", sceneLightDirection);
    fractalOSP.setUniformValue("in_scene_shadows", sceneShadows);
    fractalOSP.setUniformValue("in_scene_shadow_darkness", sceneShadowDarkness);
    fractalOSP.setUniformValue("in_scene_shadow_sharpness", sceneShadowSharpness);
    fractalOSP.setUniformValue("in_scene_specular_highlight", sceneSpecularHighlight);
    fractalOSP.setUniformValue("in_scene_specular_multiplier", sceneSpecularMultiplier);
    fractalOSP.release();

    update();
}
