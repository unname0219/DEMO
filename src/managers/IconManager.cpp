#include "managers/IconManager.h"
#include "managers/ThemeManager.h"
#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QPixmap>
#include <QByteArray>
#include <QRegularExpression>

IconManager* IconManager::s_instance = nullptr;

IconManager* IconManager::instance()
{
    if (!s_instance) {
        s_instance = new IconManager();
    }
    return s_instance;
}

IconManager::IconManager()
    : QObject()
{
    // 主题变化时清空缓存，下次访问自动重新染色
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &IconManager::refresh);
}

QString IconManager::loadResourceSvg(const QString& alias) const
{
    if (m_rawSvgCache.contains(alias)) {
        return m_rawSvgCache[alias];
    }

    QString path = ":/icons/" + alias + ".svg";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    m_rawSvgCache[alias] = content;
    return content;
}

QIcon IconManager::buildIcon(const QString& svgContent, const QColor& color) const
{
    if (svgContent.isEmpty()) return QIcon();

    // 把 currentColor 替换为目标颜色（兼容 fill="currentColor" / stroke="currentColor"）
    QString colored = svgContent;
    colored.replace(QStringLiteral("currentColor"), color.name(), Qt::CaseInsensitive);

    QSvgRenderer renderer(colored.toUtf8());
    if (!renderer.isValid()) return QIcon();

    const int px = 64; // 高分辨率渲染，缩放时保持清晰
    QPixmap pixmap(px, px);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    renderer.render(&painter);
    painter.end();

    return QIcon(pixmap);
}

QIcon IconManager::icon(const QString& name, bool tint) const
{
    if (tint) {
        QColor color = ThemeManager::instance()->textColor();
        QString cacheKey = name + "|" + color.name();
        if (m_cache.contains(cacheKey)) {
            return m_cache[cacheKey];
        }
        QString svg = loadResourceSvg(name);
        QIcon ic = buildIcon(svg, color);
        m_cache[cacheKey] = ic;
        return ic;
    }
    // 不染色（如 play/pause/logo，保留原图颜色）
    QString cacheKey = name + "|raw";
    if (m_cache.contains(cacheKey)) {
        return m_cache[cacheKey];
    }
    QString svg = loadResourceSvg(name);
    // 用一个透明占位色，渲染时不会替换任何 currentColor
    QIcon ic = buildIcon(svg, QColor(0, 0, 0, 0));
    // 上面会把 currentColor 替换为透明，这对保留原色的图标无影响
    // 但为避免 currentColor 仍然存在导致渲染异常，再渲染一次原始内容
    {
        QSvgRenderer renderer(svg.toUtf8());
        if (renderer.isValid()) {
            const int px = 128;
            QPixmap pixmap(px, px);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            renderer.render(&painter);
            painter.end();
            ic = QIcon(pixmap);
        }
    }
    m_cache[cacheKey] = ic;
    return ic;
}

QIcon IconManager::coloredIcon(const QString& name, const QColor& color) const
{
    QString cacheKey = name + "|" + color.name();
    if (m_cache.contains(cacheKey)) {
        return m_cache[cacheKey];
    }
    QString svg = loadResourceSvg(name);
    QIcon ic = buildIcon(svg, color);
    m_cache[cacheKey] = ic;
    return ic;
}

void IconManager::refresh()
{
    m_cache.clear();
}
