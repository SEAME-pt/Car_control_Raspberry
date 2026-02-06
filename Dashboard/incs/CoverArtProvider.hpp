#ifndef COVERARTPROVIDER_HPP
#define COVERARTPROVIDER_HPP

#include <QObject>
#include <QQuickImageProvider>
#include <QImage>

class CoverArtProvider : public QQuickImageProvider
{
public:
    CoverArtProvider(QImage *sharedImage);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    
private:
    QImage *m_image;
};

class CoverArtWrapper : public QObject
{
    Q_OBJECT
public:
    explicit CoverArtWrapper(QImage *image, QObject *parent = nullptr)
        : QObject(parent), m_image(image) {}
    
    Q_INVOKABLE void setImage(const QImage &img) {
        if (m_image) *m_image = img;
    }
    
private:
    QImage *m_image;
};

#endif // COVERARTPROVIDER_HPP
