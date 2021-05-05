#ifndef FRACTALWIDGET_H
#define FRACTALWIDGET_H

#include <QBasicTimer>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QWidget>

class FractalWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

public:

    static constexpr float ANIMATION_SIN_INNER_FACTOR = 0.0003f;
    static constexpr float ANIMATION_SIN_OUTER_FACTOR = 0.3f;

public:

    /// \brief
    ///     Create a new unconfigured FractalWidget.
    explicit FractalWidget(QWidget* parent = nullptr);

    /// \brief
    ///     Begins the animation which renders keyframes to the screen using the specified waypoints and outputs the
    ///     keyframes as a series of PNG images to the configured output directory.
    void animateKeyframes();

    /// \brief
    ///     Begins the animation which renders keyframes to the screen using the specified waypoints.
    void previewKeyframes();

    /// \brief
    ///     Adds a waypoint using the current camera postion and rotation.
    void addWaypoint();

    /// \brief
    ///     Adds a waypoint using the specified camera position and rotation.
    /// \param position
    ///     The position of the camera in X, Y, Z coordinates.
    /// \param rotation
    ///     The rotation of the camera in yaw, pitch, roll coordinates.
    void addWaypoint(QVector3D position, QVector3D rotation);

    /// \brief
    ///     Clears all the saved waypoints.
    void clearWayPoints();

    /// \brief
    ///     Gets the look direction from the camera.
    /// \return
    ///     The Z-axis vector of the rotation matrix generated from the current camera look direction.
    const QVector3D getLookDirectionFromCamera() const;

    /// \brief
    ///     Gets the look direction from a rotation.
    /// \param rotation
    ///     A rotation vector representing Euler angles in radians.
    /// \return
    ///     The Z-axis vector of the rotation matrix generated from the given rotation.
    const QVector3D getLookDirectionFromRotation(QVector3D rotation) const;

    /// \brief
    ///     Gets the camera's current position.
    /// \return
    ///     A vector representing the position in arbitrary units.
    const QVector3D getCameraPosition() const;

    /// \brief
    ///     Gets the camera's current rotation.
    /// \return
    ///     A vector representing Euler angles in radians.
    const QVector3D getCameraRotation() const;

signals:

    /// \brief
    ///     This signal is sent when the state or status of this widget is changed. It informs the user of certain
    ///     events such as errors, warnings, current frame being previewed/animated, etc.
    void statusChanged(const QString& message);

    /// \brief
    ///     This signal is sent when the cameras position is changed either by the user or programatically.
    void cameraPositionChaged(const QVector3D& value);

    /// \brief
    ///     This signal is sent when the cameras rotation is changed either by the user or programatically.
    void cameraRotationChaged(const QVector3D& value);

    /// \brief
    ///     This signal is sent when the keyframe value has changed either by the user or programatically.
    void fractalKeyframeChanged(const int32_t& value);

    /// \brief
    ///     This signal is sent when the user cancels the current animation.
    void animateKeyframesCancelled();

    /// \brief
    ///     This signal is sent when the animation of the current set of waypoints has finished.
    void animateKeyframesFinished();

    /// \brief
    ///     This signal is sent when the user cancels the current preview.
    void previewKeyframesCancelled();

    /// \brief
    ///     This signal is sent when the preview of the current set of waypoints has finished.
    void previewKeyframesFinished();

public slots:

    /// \brief
    ///     Sets the cameras position in arbitrary units.
    void setCameraPosition(QVector3D position);

    /// \brief
    ///     Sets the cameras rotation in Euler angles.
    void setCameraRotation(QVector3D rotation);

    /// \brief
    ///     Sets the fractal scale in arbitrary units.
    void setFractalScale(float scale);

    /// \brief
    ///     Sets the fractal position in arbitrary coordinates.
    void setFractalPosition(QVector3D position);

    /// \brief
    ///     Sets the fractal rotation in arbitrary units.
    void setFractalRotation(QVector3D rotation);

    /// \brief
    ///     Sets the fractal exposure which is the amount of light that reaches the camera.
    void setFractalExposure(float exposure);

    /// \brief
    ///     Sets the fractal colour which will be used for the orbit traps.
    void setFractalColor(QColor color);

    /// \brief
    ///     Sets the fractal animation keyframe.
    void setFractalKeyframe(int32_t value);

    /// \brief
    ///     Sets the ambient occlusion delta used for global background shading.
    void setSceneAmbientOcclusionDelta(float value);

    /// \brief
    ///     Sets the ambient occlusion strength used for global background shading.
    void setSceneAmbientOcclusionStrength(float value);

    /// \brief
    ///     Sets the number of anti-aliasing samples to compute.
    void setSceneAntiAliasingSamples(float value);

    /// \brief
    ///     Sets the scene (space) background colour.
    void setSceneBackgroundColor(QColor value);

    /// \brief
    ///     Sets whether scene diffuse lighting is enabled.
    void setSceneDiffuseLighting(bool enable);

    /// \brief
    ///     Sets whether scene filtering is enabled.
    void setSceneFiltering(bool value);

    /// \brief
    ///     Sets the scene focal distance, which is the angle of view.
    void setSceneFocalDistance(float value);

    /// \brief
    ///     Sets whether scene fog is enabled.
    void setSceneFog(bool value);

    /// \brief
    ///     Sets the colour of the scene light source.
    void setSceneLightColor(QColor value);

    /// \brief
    ///     Sets the direction of the scene light source.
    void setSceneLightDirection(QVector3D value);

    /// \brief
    ///     Sets whether the scene shadows are enabled.
    void setSceneShadows(bool enable);

    /// \brief
    ///     Sets the scene shadow darkness.
    void setSceneShadowDarkness(float value);

    /// \brief
    ///     Sets the scene shadow sharpness.
    void setSceneShadowSharpness(float value);

    /// \brief
    ///     Sets the scene specular highlight amount.
    void setSceneSpecularHighlight(float value);

    /// \brief
    ///     Sets the scene specular highlight multiplier.
    void setSceneSpecularMultiplier(float value);

    /// \brief
    ///     Sets the animation keyframe image output resolution.
    void setOutputResultion(QVector2D value);

    /// \brief
    ///     Sets the animation frames per second (FPS) target.
    void setOutputTargetFPS(float value);

    /// \brief
    ///     Sets the duration of the current animation defined by the set of waypoints recorded.
    void setOutputTargetDuration(float value);

    /// \brief
    ///     Sets the output directory where keyframe images will be saved.
    void setOutputDirectory(QString value);

protected:

    /// \brief
    ///     Initializes the shaders and creates the vertex buffer objects.
    void initializeGL() override;

    /// \brief
    ///     Implements fractal navigation using the following hotkeys:
    ///         - W: Move forward
    ///         - A: Move sideways (strafe) to the left
    ///         - S: Move backward
    ///         - D: Move sideways (strafe) to the right
    ///         - E: Rotate camera counterclockwise
    ///         - Q: Rotate camera clockwise
    ///         - Space: Record waypoint
    ///         - Backspace: Remove last waypoint
    ///         - Delete: Clear waypoints
    ///         - Escape: Stop animation/preview or stop mouse/keyboard capture
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    /// \brief
    ///     Implements camera orientation in a first person view.
    void mousePressEvent(QMouseEvent* e) override;

    void resizeGL(int w, int h) override;
    void paintGL() override;

    /// The fractal vertex buffer which is defined by two triangles forming a rectanble the size of our viewport.
    QOpenGLBuffer fractalVBO;

    /// The fractal vertex array object which saves the state of the VBO.
    QOpenGLVertexArrayObject fractalVAO;

    /// The fractal shader which will draw the fractal to the VBO.
    QOpenGLShaderProgram fractalOSP;

private:

    /// \brief
    ///     See https://github.com/fjeremic/fractal-pioneer for an explanation of how interpolation is implemented.
    QVector3D interpolatePosition(float t, bool takeDerivative);

    /// \brief
    ///     See https://github.com/fjeremic/fractal-pioneer for an explanation of how interpolation is implemented.
    QVector3D interpolateRotation(float t);

    /// \brief
    ///     See https://github.com/fjeremic/fractal-pioneer for an explanation of how interpolation is implemented.
    void blend();

    /// \brief
    ///     See https://github.com/fjeremic/fractal-pioneer for an explanation of how interpolation is implemented.
    float s2u(float s);

    void updatePhysics();
    void updateVisuals();

private:

    /// A kep map which determines whether a keyboard key is currently pressed.
    QMap<Qt::Key, bool> keyMap;

    /// Determines whether we are currently animating the waypoints and saving the keyframe images to disk.
    bool animateKeyframesActive = false;

    /// Determines whether we are currently previewing the waypoints.
    bool previewKeyframesActive = false;

    /// The keyframe at which animation/preview began.
    int32_t fractalKeyframeBegin = 0;

    /// The current keyfram being animated/previewed.
    int32_t fractalKeyframeCurrent = 0;

    /// The camera position in arbitary coordinates.
    QVector3D cameraPosition;

    /// The camera rotation in Euler angles.
    QVector3D cameraRotation;

    /// The camera rotation matrix in 3D space.
    QMatrix3x3 cameraRotationMatrix;

    /// The list of position waypoints recorded by the user.
    QList<QVector3D> positionWaypoints;

    /// The list of rotation waypoints recorded by the user.
    QList<QVector3D> rotationWaypoints;

    /// Maps arc length of the spline generated by the waypoints to interpolation parameters at those arc lengths.
    QVector<QPair<float, float>> s2uTable;

    /// The fractal scale in arbitrary units.
    float fractalScale = 0.0f;

    /// The fractal position in arbitrary coordinates.
    QVector3D fractalPosition;

    /// The fractal rotation in arbitrary coordinates.
    QVector3D fractalRotation;

    /// The fractal exposure which is the amount of light that reaches the camera.
    float fractalExposure = 0.0f;

    /// The fractal colour which will be used for the orbit traps.
    QVector3D fractalColor;

    /// The ambient occlusion delta used for global background shading.
    float sceneAmbientOcclusionDelta = 0.0f;

    /// The ambient occlusion strength used for global background shading.
    float sceneAmbientOcclusionStrength = 0.0f;

    /// The number of anti-aliasing samples to compute.
    float sceneAntiAliasingSamples = 0.0f;

    /// The scene (space) background colour.
    QVector3D sceneBackgroundColor;

    /// Determines whether scene diffuse lighting is enabled.
    bool sceneDiffuseLighting = false;

    /// Determines whether scene filtering is enabled.
    bool sceneFiltering = false;

    /// The scene focal distance, which is the angle of view.
    float sceneFocalDistance = 0.0f;

    /// Determines whether scene fog is enabled.
    bool sceneFog = false;

    /// The colour of the scene light source in RGB.
    QVector3D sceneLightColor;

    /// The direction of the scene light source.
    QVector3D sceneLightDirection;

    /// Determines whether the scene shadows are enabled.
    bool sceneShadows = false;

    /// The scene shadow darkness in range [0, inf)
    float sceneShadowDarkness = 0.0f;

    /// The scene shadow sharpness in range [0, inf)
    float sceneShadowSharpness = 0.0f;

    /// The scene specular highlight amount.
    float sceneSpecularHighlight = 0.0f;

    /// The scene specular highlight multiplier.
    float sceneSpecularMultiplier = 0.0f;

    /// The animation keyframe image output resolution.
    QVector2D outputResolution;

    /// The animation frames per second (FPS) target.
    float outputTargetFPS = 0.0f;

    /// The duration of the current animation defined by the set of waypoints recorded.
    float outputTargetDuration = 0.0f;

    /// The output directory where keyframe images will be saved.
    QString outputDirectory;

    /// The numbered index of the image that was last saved to the disk.
    int64_t outputLastDrawnFrame = 0;
};

#endif // FRACTALWIDGET_H
