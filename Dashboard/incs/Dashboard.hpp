
#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>

#ifndef WIDTH
# define WIDTH 1024
#endif

#ifndef HEIGHT
# define HEIGHT 600
#endif

#ifndef MARGIN
# define MARGIN 10
#endif

#ifndef PADDING
# define PADDING 5
#endif


class Dashboard : public QObject {
    Q_OBJECT
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(int margin READ margin CONSTANT)
	Q_PROPERTY(int padding READ padding CONSTANT)
public:
    explicit Dashboard(QObject *parent = nullptr) : QObject(parent) {}
    int width() const;
    int height() const;
    int margin() const;
	int padding() const;
};