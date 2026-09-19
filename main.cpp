#include <QCoreApplication>
#include <QTextStream>
#include <QString>
#include "pjsipconfigmanager.h"

// Helper to read trimmed line from stdin
static QString readInputLine(QTextStream &in)
{
    return in.readLine().trimmed();
}

// Function to display extensions in a formatted table
static void displayAllExtensions(QTextStream &out, const PjSipConfigManager &manager)
{
    const auto list = manager.extensionList();

    out << "\n----------------------------------------------------------------------\n";
    out << QString("%1 | %2 | %3\n")
               .arg("Number", -10)
               .arg("Caller ID", -25)
               .arg("Password", -20);
    out << "----------------------------------------------------------------------\n";

    if (list.isEmpty()) {
        out << " (No extensions configured)\n";
    } else {
        for (const auto &ext : list) {
            // اصلاح شده: استفاده از %1, %2, %3 و زنجیره کردن arg
            out << QString("%1 | %2 | %3\n")
                       .arg(ext.number, -10)
                       .arg(ext.callerId, -25)
                       .arg(ext.password, -20);
        }
    }
    out << "----------------------------------------------------------------------\n";
    out.flush();
}


// Handler: Add new extension
static bool handleAddExtension(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    out << "\n--- [Add New Extension] ---\n";
    out << "Enter Extension Number (e.g. 105): ";
    out.flush();
    QString number = readInputLine(in);

    if (number.isEmpty()) {
        out << "[Error] Extension number cannot be empty.\n";
        return false;
    }

    if (manager.containsExtension(number)) {
        out << QString("[Error] Extension %1 already exists.\n").arg(number);
        return false;
    }

    out << "Enter Password: ";
    out.flush();
    QString password = readInputLine(in);
    if (password.isEmpty()) {
        out << "[Error] Password cannot be empty.\n";
        return false;
    }

    out << "Enter Caller ID (leave empty for default): ";
    out.flush();
    QString callerId = readInputLine(in);

    PjSipExtension ext;
    ext.number = number;
    ext.password = password;
    ext.callerId = callerId.isEmpty() ? QString("\"Extension %1\" <%1>").arg(number) : callerId;
    ext.username = number;

    if (manager.addExtension(ext)) {
        out << QString("[Success] Extension %1 added successfully.\n").arg(number);
        return true;
    } else {
        out << QString("[Error] %1\n").arg(manager.lastError());
        return false;
    }
}

// Handler: Edit existing extension
static bool handleEditExtension(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    out << "\n--- [Edit Extension] ---\n";
    out << "Enter Extension Number to modify: ";
    out.flush();
    QString number = readInputLine(in);

    auto extOpt = manager.getExtension(number);
    if (!extOpt.has_value()) {
        out << QString("[Error] Extension %1 not found.\n").arg(number);
        return false;
    }

    PjSipExtension ext = extOpt.value();
    out << QString("Current Caller ID [%1]: ").arg(ext.callerId);
    out.flush();
    QString newCallerId = readInputLine(in);
    if (!newCallerId.isEmpty()) {
        ext.callerId = newCallerId;
    }

    out << QString("Current Password [%1]: ").arg(ext.password);
    out.flush();
    QString newPassword = readInputLine(in);
    if (!newPassword.isEmpty()) {
        ext.password = newPassword;
    }

    if (manager.editExtension(ext)) {
        out << QString("[Success] Extension %1 updated.\n").arg(number);
        return true;
    } else {
        out << QString("[Error] %1\n").arg(manager.lastError());
        return false;
    }
}

// Handler: Remove extension
static bool handleRemoveExtension(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    out << "\n--- [Remove Extension] ---\n";
    out << "Enter Extension Number to remove: ";
    out.flush();
    QString number = readInputLine(in);

    if (!manager.containsExtension(number)) {
        out << QString("[Error] Extension %1 not found.\n").arg(number);
        return false;
    }

    out << QString("Are you sure you want to delete extension %1? (y/N): ").arg(number);
    out.flush();
    QString confirm = readInputLine(in).toLower();

    if (confirm == "y" || confirm == "yes") {
        if (manager.removeExtension(number)) {
            out << QString("[Success] Extension %1 removed.\n").arg(number);
            return true;
        } else {
            out << QString("[Error] %1\n").arg(manager.lastError());
        }
    } else {
        out << "[Cancelled] Removal aborted.\n";
    }
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream cinStream(stdin);
    QTextStream coutStream(stdout);

    // Default configuration path
    QString configPath = "/etc/asterisk/pjsip.conf";
    if (argc > 1) {
        configPath = QString::fromLocal8Bit(argv[1]);
    }

    PjSipConfigManager sipManager(configPath);

    coutStream << "===============================================\n";
    coutStream << "       Asterisk PJSIP Extension Manager        \n";
    coutStream << "===============================================\n";
    coutStream << "Target Config File: " << configPath << "\n";
    coutStream.flush();

    // Load initial configuration
    if (!sipManager.load()) {
        coutStream << "[Fatal Error] " << sipManager.lastError() << "\n";
        coutStream << "Hint: Ensure read permissions or run with sudo.\n";
        coutStream.flush();
        return 1;
    }

    coutStream << "[OK] Configuration loaded successfully.\n";

    bool hasUnsavedChanges = false;
    bool running = true;

    while (running) {
        coutStream << "\n=============== MAIN MENU ===============";
        if (hasUnsavedChanges) {
            coutStream << " [UNSAVED CHANGES *]";
        }
        coutStream << "\n";
        coutStream << "1) List all extensions\n";
        coutStream << "2) Add new extension\n";
        coutStream << "3) Edit an extension\n";
        coutStream << "4) Remove an extension\n";
        coutStream << "5) Reload from file (Discard changes)\n";
        coutStream << "6) Save changes to file\n";
        coutStream << "7) Save & Exit\n";
        coutStream << "0) Exit without saving\n";
        coutStream << "-----------------------------------------\n";
        coutStream << "Select an option [0-7]: ";
        coutStream.flush();

        QString choice = readInputLine(cinStream);

        if (choice == "1") {
            displayAllExtensions(coutStream, sipManager);
        }
        else if (choice == "2") {
            if (handleAddExtension(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "3") {
            if (handleEditExtension(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "4") {
            if (handleRemoveExtension(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "5") {
            if (hasUnsavedChanges) {
                coutStream << "Discard all unsaved changes and reload? (y/N): ";
                coutStream.flush();
                if (readInputLine(cinStream).toLower() != "y") {
                    continue;
                }
            }
            if (sipManager.load()) {
                hasUnsavedChanges = false;
                coutStream << "[Success] Configuration reloaded from disk.\n";
            } else {
                coutStream << "[Error] Failed to reload: " << sipManager.lastError() << "\n";
            }
        }
        else if (choice == "6") {
            if (sipManager.save()) {
                hasUnsavedChanges = false;
                coutStream << "[Success] File saved successfully.\n";
            } else {
                coutStream << "[Error] " << sipManager.lastError() << "\n";
            }
        }
        else if (choice == "7") {
            if (hasUnsavedChanges) {
                if (sipManager.save()) {
                    coutStream << "[Success] File saved. Goodbye!\n";
                    running = false;
                } else {
                    coutStream << "[Error] Save failed: " << sipManager.lastError() << "\n";
                }
            } else {
                coutStream << "No changes to save. Goodbye!\n";
                running = false;
            }
        }
        else if (choice == "0") {
            if (hasUnsavedChanges) {
                coutStream << "You have unsaved changes! Really exit without saving? (y/N): ";
                coutStream.flush();
                if (readInputLine(cinStream).toLower() == "y") {
                    running = false;
                }
            } else {
                running = false;
            }
        }
        else {
            coutStream << "[Invalid choice] Please enter a number between 0 and 7.\n";
        }
    }

    return 0;
}
