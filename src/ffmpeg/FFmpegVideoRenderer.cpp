#include "ffmpeg/FFmpegVideoRenderer.h"
#include "ffmpeg/FFmpegDecoder.h"

static const char* vertexShaderSource = R"(
    attribute vec2 a_position;
    attribute vec2 a_texcoord;
    varying vec2 v_texcoord;
    void main() {
        gl_Position = vec4(a_position, 0.0, 1.0);
        v_texcoord = a_texcoord;
    }
)";

static const char* fragmentShaderSource = R"(
    varying vec2 v_texcoord;
    uniform sampler2D u_texture;
    void main() {
        gl_FragColor = texture2D(u_texture, v_texcoord);
    }
)";

struct Vertex {
    float x, y;
    float u, v;
};

FFmpegVideoRenderer::FFmpegVideoRenderer(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_decoder(nullptr)
    , m_updateTimer(nullptr)
    , m_lastPts(0)
    , m_program(nullptr)
    , m_texture(0)
    , m_frameUpdated(false)
{
    setMinimumSize(320, 240);
    setAutoFillBackground(false);

    QSurfaceFormat format;
    format.setDepthBufferSize(0);
    format.setStencilBufferSize(0);
    format.setSamples(0);
    setFormat(format);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setTimerType(Qt::PreciseTimer);
    m_updateTimer->setInterval(8);
    connect(m_updateTimer, &QTimer::timeout, this, &FFmpegVideoRenderer::updateFrame);
    m_updateTimer->start();
}

FFmpegVideoRenderer::~FFmpegVideoRenderer()
{
    makeCurrent();
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    delete m_program;
    doneCurrent();
}

void FFmpegVideoRenderer::setDecoder(FFmpegDecoder* decoder)
{
    m_decoder = decoder;
}

void FFmpegVideoRenderer::clearFrame()
{
    QMutexLocker locker(&m_textureMutex);
    m_currentFrame = QImage();
    m_pendingFrame = QImage();
    m_frameUpdated = true;
    update();
}

void FFmpegVideoRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_program->bind();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo.bind();

    Vertex vertices[] = {
        { -1.0f, -1.0f, 0.0f, 1.0f },
        {  1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f, 1.0f, 0.0f },
    };
    m_vbo.allocate(vertices, 4 * sizeof(Vertex));

    int posLoc = m_program->attributeLocation("a_position");
    int texLoc = m_program->attributeLocation("a_texcoord");

    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));

    m_vbo.release();
    m_vao.release();
    m_program->release();
}

void FFmpegVideoRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QMutexLocker locker(&m_textureMutex);
    if (m_currentFrame.isNull()) return;

    if (m_frameUpdated) {
        updateTexture(m_currentFrame);
        m_frameUpdated = false;
    }

    if (!m_texture) return;

    m_program->bind();
    m_vao.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    m_program->setUniformValue("u_texture", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
    m_program->release();
}

void FFmpegVideoRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void FFmpegVideoRenderer::updateFrame()
{
    if (!m_decoder) return;

    qint64 currentPos = m_decoder->currentPosition();

    VideoFrame frame = m_decoder->takeVideoFrame(currentPos);
    if (!frame.image.isNull()) {
        QMutexLocker locker(&m_textureMutex);
        m_currentFrame = frame.image;
        m_frameUpdated = true;
        m_lastPts = frame.pts;
        locker.unlock();
        emit frameDisplayed(frame.pts);
        update();
    }
}

void FFmpegVideoRenderer::updateTexture(const QImage& image)
{
    if (image.isNull()) return;

    GLenum format = GL_RGB;
    QImage texImage = image;

    if (image.format() == QImage::Format_RGB888) {
        format = GL_RGB;
    } else if (image.format() == QImage::Format_RGBA8888) {
        format = GL_RGBA;
    } else {
        texImage = image.convertToFormat(QImage::Format_RGB888);
        format = GL_RGB;
    }

    glBindTexture(GL_TEXTURE_2D, m_texture);

    static int lastW = 0, lastH = 0;
    if (texImage.width() != lastW || texImage.height() != lastH) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, texImage.width(), texImage.height(),
                     0, format, GL_UNSIGNED_BYTE, texImage.bits());
        lastW = texImage.width();
        lastH = texImage.height();
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texImage.width(), texImage.height(),
                        format, GL_UNSIGNED_BYTE, texImage.bits());
    }
}
