#ifndef FFMPEGVIDEORENDERER_H
#define FFMPEGVIDEORENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMutex>
#include <QTimer>
#include <QImage>

class FFmpegDecoder;
struct VideoFrame;

class FFmpegVideoRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit FFmpegVideoRenderer(QWidget* parent = nullptr);
    ~FFmpegVideoRenderer();

    void setDecoder(FFmpegDecoder* decoder);
    void clearFrame();

signals:
    void frameDisplayed(qint64 pts);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private slots:
    void updateFrame();

private:
    FFmpegDecoder* m_decoder;
    QTimer* m_updateTimer;
    qint64 m_lastPts;

    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer m_vbo;
    QOpenGLVertexArrayObject m_vao;
    GLuint m_texture;

    QMutex m_textureMutex;
    QImage m_currentFrame;
    QImage m_pendingFrame;
    bool m_frameUpdated;

    void updateTexture(const QImage& image);
};

#endif
