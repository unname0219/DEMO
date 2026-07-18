#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QObject>
#include <QIcon>
#include <QPixmap>
#include <QHash>
#include <QString>

/**
 * 图标管理器
 *
 * 职责：
 * 1. 从 Qt 资源系统加载 SVG 图标
 * 2. 根据当前主题色对使用 currentColor 的图标动态染色
 *    - 深色模式 → 浅色图标
 *    - 浅色模式 → 深色图标
 *    - 主题切换时通过 ThemeManager::themeChanged 触发刷新
 * 3. 保留原图颜色的图标不染色（如 play.svg / pause.svg 的品牌渐变）
 *
 * 使用：
 *   QIcon icon = IconManager::instance()->icon("play");
 */
class IconManager : public QObject
{
    Q_OBJECT

public:
    static IconManager* instance();

    /**
     * 获取已染色的图标
     * @param name 图标名（不含扩展名，对应 resources.qrc 中的 alias）
     * @param tint 是否根据主题色染色（默认 true）
     */
    QIcon icon(const QString& name, bool tint = true) const;

    /** 获取一个指定颜色的图标（用于特殊场景，如悬停/按下状态） */
    QIcon coloredIcon(const QString& name, const QColor& color) const;

    /** 主题变化时清空缓存，下次访问重新生成 */
    void refresh();

private:
    IconManager();

    QIcon buildIcon(const QString& svgContent, const QColor& color) const;
    QString loadResourceSvg(const QString& alias) const;

    mutable QHash<QString, QIcon> m_cache;
    mutable QHash<QString, QString> m_rawSvgCache; // 原始 SVG 内容缓存
};

#endif // ICONMANAGER_H
