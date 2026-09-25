#include <QCoreApplication>
#include <QTextStream>
#include <QString>
#include "../../QtLibraries/pjsipConfigManager/pjsipconfigmanager.h"

// Helper to read trimmed line from stdin
static QString readInputLine(QTextStream &in)
{
    return in.readLine().trimmed();
}

// Function to display extensions in a formatted table
static void displayAllExtensions(QTextStream &out, const PjSipConfigManager &manager)
{
    const auto list = manager.extensionList();

    out << "\n===================================================================================================\n";
    out << QString("%1 | %2 | %3 | %4 | %5 | %6\n")
               .arg("Number", -8)
               .arg("Caller ID", -22)
               .arg("Password", -15)
               .arg("Status", -10)
               .arg("Video", -8)
               .arg("Context", -20);
    out << "---------------------------------------------------------------------------------------------------\n";

    if (list.isEmpty()) {
        out << " (No extensions configured)\n";
    } else {
        for (const auto &ext : list) {
            QString statusStr = ext.enabled ? "[ACTIVE]" : "[DISABLED]";
            QString videoStr = ext.videoEnabled ? "YES" : "NO";
            out << QString("%1 | %2 | %3 | %4 | %5 | %6\n")
                       .arg(ext.number, -8)
                       .arg(ext.callerId, -22)
                       .arg(ext.password, -15)
                       .arg(statusStr, -10)
                       .arg(videoStr, -8)
                       .arg(ext.context, -20);
        }
    }
    out << "===================================================================================================\n";
    out.flush();
}

// Function to display transports in a formatted table
static void displayAllTransports(QTextStream &out, const PjSipConfigManager &manager)
{
    const auto list = manager.transportList();

    out << "\n----------------------------------------------------------------------\n";
    out << QString("%1 | %2 | %3 | %4\n")
               .arg("Name", -18)
               .arg("Protocol", -10)
               .arg("Bind Address", -22)
               .arg("TLS Method", -12);
    out << "----------------------------------------------------------------------\n";

    if (list.isEmpty()) {
        out << " (No transports configured)\n";
    } else {
        for (const auto &t : list) {
            out << QString("%1 | %2 | %3 | %4\n")
            .arg(t.name, -18)
                .arg(t.protocol.toUpper(), -10)
                .arg(t.bind, -22)
                .arg(t.tlsMethod.isEmpty() ? "-" : t.tlsMethod, -12);
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

    out << "Enable Video Support (h264/vp8)? (y/N): ";
    out.flush();
    QString videoChoice = readInputLine(in).toLower();
    bool videoEnabled = (videoChoice == "y" || videoChoice == "yes");

    out << "Initial Status: Enabled? (Y/n): ";
    out.flush();
    QString statusChoice = readInputLine(in).toLower();
    bool enabled = (statusChoice != "n" && statusChoice != "no");

    PjSipExtension ext;
    ext.number = number;
    ext.username = number;
    ext.password = password;
    ext.callerId = callerId.isEmpty() ? QString("\"Extension %1\" <%1>").arg(number) : callerId;
    ext.videoEnabled = videoEnabled;
    ext.enabled = enabled;

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
    out << QString("Current Caller ID [%1] (Press Enter to keep): ").arg(ext.callerId);
    out.flush();
    QString newCallerId = readInputLine(in);
    if (!newCallerId.isEmpty()) {
        ext.callerId = newCallerId;
    }

    out << QString("Current Password [%1] (Press Enter to keep): ").arg(ext.password);
    out.flush();
    QString newPassword = readInputLine(in);
    if (!newPassword.isEmpty()) {
        ext.password = newPassword;
    }

    out << QString("Current Context [%1] (Press Enter to keep): ").arg(ext.context);
    out.flush();
    QString newContext = readInputLine(in);
    if (!newContext.isEmpty()) {
        ext.context = newContext;
    }

    out << QString("Video Support currently [%1]. Enable? (y/n, Enter to keep): ")
               .arg(ext.videoEnabled ? "YES" : "NO");
    out.flush();
    QString vidChoice = readInputLine(in).toLower();
    if (vidChoice == "y" || vidChoice == "yes") {
        ext.videoEnabled = true;
    } else if (vidChoice == "n" || vidChoice == "no") {
        ext.videoEnabled = false;
    }

    if (manager.editExtension(ext)) {
        out << QString("[Success] Extension %1 updated.\n").arg(number);
        return true;
    } else {
        out << QString("[Error] %1\n").arg(manager.lastError());
        return false;
    }
}

// Handler: Toggle Extension Enabled/Disabled
static bool handleToggleExtensionStatus(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    out << "\n--- [Toggle Extension Status (Enable / Disable)] ---\n";
    out << "Enter Extension Number: ";
    out.flush();
    QString number = readInputLine(in);

    auto extOpt = manager.getExtension(number);
    if (!extOpt.has_value()) {
        out << QString("[Error] Extension %1 not found.\n").arg(number);
        return false;
    }

    bool currentStatus = extOpt->enabled;
    bool newStatus = !currentStatus;

    out << QString("Extension %1 is currently %2.\n")
               .arg(number, currentStatus ? "ENABLED" : "DISABLED");
    out << QString("Do you want to %1 it? (y/N): ")
               .arg(newStatus ? "ENABLE" : "DISABLE");
    out.flush();

    QString confirm = readInputLine(in).toLower();
    if (confirm == "y" || confirm == "yes") {
        if (manager.setExtensionEnabled(number, newStatus)) {
            out << QString("[Success] Extension %1 is now %2.\n")
            .arg(number, newStatus ? "ENABLED" : "DISABLED");
            return true;
        } else {
            out << QString("[Error] %1\n").arg(manager.lastError());
        }
    } else {
        out << "[Cancelled] Status unchanged.\n";
    }
    return false;
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

// Handler: Transport Management Menu
static bool handleTransportMenu(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    while (true) {
        out << "\n--- [Transport Management] ---\n";
        displayAllTransports(out, manager);
        out << "1) Add New Transport (UDP / TCP / TLS / WS / WSS)\n";
        out << "2) Remove Transport\n";
        out << "0) Back to Main Menu\n";
        out << "Select: ";
        out.flush();

        QString choice = readInputLine(in);
        if (choice == "0") {
            return false;
        }

        if (choice == "1") {
            out << "Enter Transport Section Name (e.g. transport-tcp, transport-tls): ";
            out.flush();
            QString name = readInputLine(in);
            if (name.isEmpty()) {
                out << "[Error] Name cannot be empty.\n";
                continue;
            }

            out << "Enter Protocol (udp, tcp, tls, ws, wss) [default: udp]: ";
            out.flush();
            QString proto = readInputLine(in).toLower();
            if (proto.isEmpty()) proto = "udp";

            out << "Enter Bind Address (e.g. 0.0.0.0:5060): ";
            out.flush();
            QString bind = readInputLine(in);
            if (bind.isEmpty()) bind = "0.0.0.0:5060";

            PjSipTransport transport;
            transport.name = name;
            transport.protocol = proto;
            transport.bind = bind;

            if (proto == "tls" || proto == "wss") {
                out << "Enter Certificate File Path (cert_file): ";
                out.flush();
                transport.certFile = readInputLine(in);

                out << "Enter Private Key File Path (priv_key_file): ";
                out.flush();
                transport.privKeyFile = readInputLine(in);

                out << "Enter CA List File Path (leave empty if none): ";
                out.flush();
                transport.caListFile = readInputLine(in);

                out << "Enter TLS Method (e.g. tlsv1_2, tlsv1_3) [default: tlsv1_2]: ";
                out.flush();
                QString method = readInputLine(in);
                transport.tlsMethod = method.isEmpty() ? "tlsv1_2" : method;
            }

            if (manager.addTransport(transport)) {
                out << QString("[Success] Transport %1 created.\n").arg(name);
                return true;
            } else {
                out << QString("[Error] %1\n").arg(manager.lastError());
            }
        }
        else if (choice == "2") {
            out << "Enter Transport Name to remove: ";
            out.flush();
            QString name = readInputLine(in);
            if (manager.removeTransport(name)) {
                out << QString("[Success] Transport %1 removed.\n").arg(name);
                return true;
            } else {
                out << QString("[Error] %1\n").arg(manager.lastError());
            }
        }
    }
}

// Handler: Edit System/Global Configuration & Templates
static bool handleSystemConfig(QTextStream &in, QTextStream &out, PjSipConfigManager &manager)
{
    out << "\n--- [System & Template Configuration] ---\n";
    out << "1) Edit [global] Setting\n";
    out << "2) Edit [endpoint-template](!) Setting\n";
    out << "3) Edit [auth-template](!) Setting\n";
    out << "4) Edit [aor-template](!) Setting\n";
    out << "5) Add/Edit Custom Section or Template\n";
    out << "0) Back\n";
    out << "Select: ";
    out.flush();

    QString choice = readInputLine(in);
    if (choice == "0") return false;

    QString sectionName;
    QString templateName;

    if (choice == "1") {
        sectionName = "global";
    } else if (choice == "2") {
        sectionName = "endpoint-template";
        templateName = "!";
    } else if (choice == "3") {
        sectionName = "auth-template";
        templateName = "!";
    } else if (choice == "4") {
        sectionName = "aor-template";
        templateName = "!";
    } else if (choice == "5") {
        out << "Enter Section Name: ";
        out.flush();
        sectionName = readInputLine(in);
        out << "Enter Template Name (leave empty for regular, '!' for template): ";
        out.flush();
        templateName = readInputLine(in);
    } else {
        return false;
    }

    out << "Enter Key (e.g. allow, dtmf_mode, direct_media): ";
    out.flush();
    QString key = readInputLine(in);
    if (key.isEmpty()) {
        out << "[Error] Key cannot be empty.\n";
        return false;
    }

    out << QString("Current value: %1\n").arg(manager.getConfigValue(sectionName, key, templateName, "(not set)"));
    out << "Enter New Value: ";
    out.flush();
    QString value = readInputLine(in);

    if (manager.setConfigValue(sectionName, key, value, templateName)) {
        out << "[Success] Configuration updated.\n";
        return true;
    } else {
        out << "[Error] Failed to update configuration.\n";
        return false;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream cinStream(stdin);
    QTextStream coutStream(stdout);

    QString configPath = "/etc/asterisk/pjsip.conf";
    if (argc > 1) {
        configPath = QString::fromLocal8Bit(argv[1]);
    }

    PjSipConfigManager sipManager(configPath);

    coutStream << "=======================================================\n";
    coutStream << "         Asterisk PJSIP Configuration Console          \n";
    coutStream << "=======================================================\n";
    coutStream << "Target Config File: " << configPath << "\n";
    coutStream.flush();

    if (!sipManager.load()) {
        coutStream << "[Fatal Error] " << sipManager.lastError() << "\n";
        coutStream << "Hint: Ensure read permissions or verify the path.\n";
        coutStream.flush();
        return 1;
    }

    coutStream << "[OK] Configuration loaded successfully.\n";

    bool hasUnsavedChanges = false;
    bool running = true;

    while (running) {
        coutStream << "\n====================== MAIN MENU ======================";
        if (hasUnsavedChanges) {
            coutStream << " [UNSAVED CHANGES *]";
        }
        coutStream << "\n";
        coutStream << "1) List all extensions\n";
        coutStream << "2) Add new extension\n";
        coutStream << "3) Edit extension details\n";
        coutStream << "4) Enable / Disable extension (Context routing)\n";
        coutStream << "5) Remove extension\n";
        coutStream << "6) Transport Management (UDP / TCP / TLS / WSS)\n";
        coutStream << "7) Edit Templates & Global Settings\n";
        coutStream << "8) Reload from disk (Discard unsaved changes)\n";
        coutStream << "9) Save changes to disk\n";
        coutStream << "10) Apply Live Reload to Asterisk (asterisk -rx 'pjsip reload')\n";
        coutStream << "0) Exit\n";
        coutStream << "-------------------------------------------------------\n";
        coutStream << "Select an option [0-10]: ";
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
            if (handleToggleExtensionStatus(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "5") {
            if (handleRemoveExtension(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "6") {
            if (handleTransportMenu(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "7") {
            if (handleSystemConfig(cinStream, coutStream, sipManager)) {
                hasUnsavedChanges = true;
            }
        }
        else if (choice == "8") {
            if (hasUnsavedChanges) {
                coutStream << "Discard unsaved changes and reload from file? (y/N): ";
                coutStream.flush();
                if (readInputLine(cinStream).toLower() != "y") {
                    continue;
                }
            }
            if (sipManager.load()) {
                hasUnsavedChanges = false;
                coutStream << "[Success] Configuration reloaded from disk.\n";
            } else {
                coutStream << "[Error] Reload failed: " << sipManager.lastError() << "\n";
            }
        }
        else if (choice == "9") {
            if (sipManager.save()) {
                hasUnsavedChanges = false;
                coutStream << "[Success] Configuration written to disk successfully.\n";
            } else {
                coutStream << "[Error] Failed to save: " << sipManager.lastError() << "\n";
            }
        }
        else if (choice == "10") {
            if (hasUnsavedChanges) {
                coutStream << "[Notice] You have unsaved changes. Save before reloading Asterisk? (Y/n): ";
                coutStream.flush();
                QString autoSave = readInputLine(cinStream).toLower();
                if (autoSave != "n" && autoSave != "no") {
                    if (!sipManager.save()) {
                        coutStream << "[Error] Save failed, aborting Asterisk reload.\n";
                        continue;
                    }
                    hasUnsavedChanges = false;
                    coutStream << "[OK] Saved.\n";
                }
            }
            coutStream << "[Executing] Reloading Asterisk PJSIP module...\n";
            if (sipManager.reloadAsterisk()) {
                coutStream << "[Success] Asterisk PJSIP reloaded successfully!\n";
            } else {
                coutStream << QString("[Error] Asterisk reload failed: %1\n").arg(sipManager.lastError());
            }
        }
        else if (choice == "0") {
            if (hasUnsavedChanges) {
                coutStream << "You have unsaved changes! Save before exiting? (y/N/Cancel): ";
                coutStream.flush();
                QString exitChoice = readInputLine(cinStream).toLower();
                if (exitChoice == "y" || exitChoice == "yes") {
                    if (sipManager.save()) {
                        coutStream << "[Success] Saved. Goodbye!\n";
                        running = false;
                    } else {
                        coutStream << "[Error] " << sipManager.lastError() << "\n";
                    }
                } else if (exitChoice == "n" || exitChoice == "no") {
                    running = false;
                }
            } else {
                running = false;
            }
        }
        else {
            coutStream << "[Invalid choice] Please enter a valid number from 0 to 10.\n";
        }
    }

    return 0;
}
