#include <gtest/gtest.h>
#include <QImage>
#include <QSize>
#include <QColor>
#include "../incs/CoverArtProvider.hpp"

// When no shared image is provided, requestImage should return a non-null placeholder.
TEST(CoverArtProvider, ReturnsPlaceholderWhenMissing) {
    // Provider constructed with nullptr -> no shared image available
    CoverArtProvider p(nullptr);
    QSize requested(64, 64);
    // Do not pass an uninitialized QSize* — provider may not populate it.
    QImage img = p.requestImage(QStringLiteral("non-existent-id"), nullptr, requested);

    EXPECT_FALSE(img.isNull());
    // implementation may ignore requested size; ensure placeholder is valid
    EXPECT_GT(img.width(), 0);
    EXPECT_GT(img.height(), 0);
}

// When a shared image is populated, requestImage should return the same pixels.
TEST(CoverArtProvider, SharedImageRoundTrip) {
    // Provider backed by a shared QImage pointer — populating 'shared' should be visible via requestImage
    QImage shared;
    CoverArtProvider p(&shared);

    QImage src(16, 16, QImage::Format_ARGB32);
    src.fill(QColor(Qt::red));
    shared = src; // simulate "set cover art"

    QSize returnedSize;
    QImage out = p.requestImage(QStringLiteral("album-1"), &returnedSize, QSize());
    EXPECT_EQ(out.size(), src.size());
    EXPECT_EQ(out.pixel(0, 0), src.pixel(0, 0));
}

// Initially returns a placeholder for an empty shared image, then returns the shared image once populated.
TEST(CoverArtProvider, EmptyThenSetSharedImageBehavior) {
    QImage shared;
    CoverArtProvider p(&shared);

    // initially shared is empty -> requestImage returns a placeholder (non-null)
    QSize req(8,8);
    QImage placeholder = p.requestImage(QStringLiteral("missing"), nullptr, req);
    EXPECT_FALSE(placeholder.isNull());
    EXPECT_EQ(shared.isNull(), true);

    // populate shared and expect provider to return the shared image
    QImage src(8, 8, QImage::Format_ARGB32);
    src.fill(QColor(Qt::blue));
    shared = src;

    QImage out = p.requestImage(QStringLiteral("missing"), nullptr, QSize());
    EXPECT_EQ(out.size(), src.size());
    EXPECT_EQ(out.pixel(0,0), src.pixel(0,0));
}

// Requesting with an empty id should be handled gracefully and return a valid image.
TEST(CoverArtProvider, HandlesEmptyIdGracefully) {
    QImage shared;
    CoverArtProvider p(&shared);

    QImage src(4, 4, QImage::Format_ARGB32);
    src.fill(QColor(Qt::green));
    shared = src;

    QSize requested(32, 32);
    QImage out = p.requestImage(QString(), nullptr, requested);
    EXPECT_FALSE(out.isNull());
    // returned image should at least contain the same top-left color as the shared image
    EXPECT_EQ(QColor(out.pixel(0,0)).rgb(), QColor(src.pixel(0,0)).rgb());
}

// CoverArtWrapper::setImage should copy the provided image into the shared QImage.
TEST(CoverArtWrapper, SetImageCopiesToShared) {
    QImage shared;
    CoverArtWrapper wrapper(&shared);

    QImage src(10, 10, QImage::Format_ARGB32);
    src.fill(QColor(Qt::yellow));

    wrapper.setImage(src);

    EXPECT_EQ(shared.size(), src.size());
    EXPECT_EQ(QColor(shared.pixel(0,0)).rgb(), QColor(src.pixel(0,0)).rgb());
}

// Calling setImage when CoverArtWrapper was constructed with nullptr must be safe (no crash / no throw).
TEST(CoverArtWrapper, SetImageNoopWhenNullptr) {
    CoverArtWrapper wrapper(nullptr);

    QImage src(2, 2, QImage::Format_ARGB32);
    src.fill(QColor(Qt::magenta));

    EXPECT_NO_THROW(wrapper.setImage(src));
    SUCCEED();
}