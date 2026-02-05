
#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include "DashboardMacros.hpp"


class Dashboard : public QObject {
    Q_OBJECT
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(int margin READ margin CONSTANT)
	Q_PROPERTY(double padding_x READ padding_x CONSTANT)
    Q_PROPERTY(double padding_y READ padding_y CONSTANT)
    Q_PROPERTY(double topHeight READ topHeight CONSTANT)
    Q_PROPERTY(double centerHeight READ centerHeight CONSTANT)
    Q_PROPERTY(double bottomHeight READ bottomHeight CONSTANT)
    Q_PROPERTY(double leftWidth READ leftWidth CONSTANT)
    Q_PROPERTY(double centerWidth READ centerWidth CONSTANT)
    Q_PROPERTY(double rightWidth READ rightWidth CONSTANT)
public:
    explicit Dashboard(QObject *parent = nullptr) : QObject(parent) {}
    int width() const;
    int height() const;
    int margin() const;
	double padding_x() const;
    double padding_y() const;
    double topHeight() const;
    double centerHeight() const;
    double bottomHeight() const;
    double leftWidth() const;
    double centerWidth() const;
    double rightWidth() const;
};