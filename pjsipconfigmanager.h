#ifndef PJSIPCONFIGMANAGER_H
#define PJSIPCONFIGMANAGER_H

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <optional>

struct PjSipExtension
{
    QString number;
    QString password;
    QString callerId; // Optional: e.g. "\"Extension 101\" <101>"
    QString username; // Optional: defaults to number if empty
};

class PjSipConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit PjSipConfigManager(QObject *parent = nullptr);
    explicit PjSipConfigManager(const QString &filePath, QObject *parent = nullptr);

    // File I/O
    bool load(const QString &filePath = QString());
    bool save(const QString &filePath = QString());

    // CRUD operations for Asterisk PJSIP extensions
    bool addExtension(const PjSipExtension &extension);
    bool addExtension(const QString &number, const QString &password, const QString &callerId = QString());
    bool editExtension(const PjSipExtension &extension);
    bool removeExtension(const QString &number);

    // Query methods
    bool containsExtension(const QString &number) const;
    std::optional<PjSipExtension> getExtension(const QString &number) const;
    QList<PjSipExtension> extensionList() const;
    QString lastError() const;

signals:
    void loaded();
    void saved();
    void extensionAdded(const QString &number);
    void extensionModified(const QString &number);
    void extensionRemoved(const QString &number);
    void errorOccurred(const QString &error);

private:
    struct Section
    {
        QString rawHeader;
        QString name;
        QString templateName;
        QList<QPair<QString, QString>> keyValuePairs;
        QList<QString> rawLines;
    };

    QString m_filePath;
    QString m_lastError;
    QList<Section> m_sections;

    void parseContent(const QString &content);
    QString serializeContent() const;
    int findSectionIndex(const QString &name, const QString &templateName) const;
};


#endif // PJSIPCONFIGMANAGER_H
