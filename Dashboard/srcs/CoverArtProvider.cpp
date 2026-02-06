#include "../incs/CoverArtProvider.hpp"

CoverArtProvider::CoverArtProvider(QImage *sharedImage)
    : QQuickImageProvider(QQuickImageProvider::Image), m_image(sharedImage)
{
}

QImage CoverArtProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);
    Q_UNUSED(requestedSize);
    
    if (!m_image || m_image->isNull()) {
        return QImage(256, 256, QImage::Format_ARGB32);
    }
    
    if (size) {
        *size = m_image->size();
    }
    
    return *m_image;
}
