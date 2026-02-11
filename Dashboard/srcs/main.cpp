
#include "../incs/Dashboard.hpp"
#include "../incs/CarDataController.hpp"
#include "../incs/CoverArtProvider.hpp"

static QObject* dashboardSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Dashboard();
}

static QObject* carDataSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new CarDataController();
}

int main(int argc, char *argv[]) {
    // Force GStreamer backend for proper album art support
    qputenv("QT_MEDIA_BACKEND", "gstreamer");
    
    QGuiApplication app(argc, argv);
    qDebug() << "App started";
    
    qmlRegisterSingletonType<Dashboard>("com.dashboard", 1, 0, "Dashboard", dashboardSingletonProvider);
    qmlRegisterSingletonType<CarDataController>("com.cardata", 1, 0, "CarData", carDataSingletonProvider);
    
    QQmlApplicationEngine engine;
    
    QImage *coverImage = new QImage();
    CoverArtProvider *provider = new CoverArtProvider(coverImage);
    engine.addImageProvider(QStringLiteral("coverart"), provider);
    
    CoverArtWrapper *wrapper = new CoverArtWrapper(coverImage);
    engine.rootContext()->setContextProperty("coverArt", wrapper);
    
    const QUrl url(QStringLiteral("qrc:/srcs/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}