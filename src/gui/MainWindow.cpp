/**
 * @file MainWindow.cpp
 * @brief Main window implementation for LSL Template GUI application
 */

#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <lsltemplate/Config.hpp>
#include <lsltemplate/Device.hpp>
#include <lsltemplate/StreamThread.hpp>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

#include <lsl_cpp.h>

MainWindow::MainWindow(const QString& config_file, QWidget* parent)
    : QMainWindow(parent)
    , ui_(std::make_unique<Ui::MainWindow>())
{
    ui_->setupUi(this);

    // Connect signals
    connect(ui_->linkButton, &QPushButton::clicked, this, &MainWindow::onLinkButtonClicked);
    connect(ui_->actionLoad_Configuration, &QAction::triggered, this, &MainWindow::onLoadConfig);
    connect(ui_->actionSave_Configuration, &QAction::triggered, this, &MainWindow::onSaveConfig);
    connect(ui_->actionQuit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui_->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // Load configuration
    QString cfg_path = config_file.isEmpty() ? findDefaultConfigFile() : config_file;
    if (!cfg_path.isEmpty()) {
        loadConfig(cfg_path);
    }
}

MainWindow::~MainWindow() {
    // Stop streaming if running
    if (stream_) {
        stream_->stop();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (stream_ && stream_->isRunning()) {
        auto result = QMessageBox::question(
            this,
            "Streaming Active",
            "Streaming is still active. Stop and quit?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (result == QMessageBox::No) {
            event->ignore();
            return;
        }

        stream_->stop();
    }

    event->accept();
}

void MainWindow::onLinkButtonClicked() {
    if (stream_ && stream_->isRunning()) {
        // Stop streaming
        stream_->stop();
        stream_.reset();
        setStreaming(false);
    } else {
        // Start streaming
        lsltemplate::MockDevice::Config device_config{
            .name = ui_->input_name->text().toStdString(),
            .type = ui_->input_type->text().toStdString(),
            .channel_count = ui_->input_channels->value(),
            .sample_rate = ui_->input_srate->value(),
            .start_value = ui_->input_device->value()
        };

        auto device = std::make_unique<lsltemplate::MockDevice>(device_config);

        // Create status callback that updates UI (must be thread-safe)
        auto callback = [this](const std::string& message, bool is_error) {
            // Use Qt's thread-safe signal mechanism
            QMetaObject::invokeMethod(this, [this, message, is_error]() {
                updateStatus(QString::fromStdString(message), is_error);
            });
        };

        stream_ = std::make_unique<lsltemplate::StreamThread>(std::move(device), callback);

        if (stream_->start()) {
            setStreaming(true);
        } else {
            stream_.reset();
            QMessageBox::warning(this, "Error", "Failed to start streaming");
        }
    }
}

void MainWindow::onLoadConfig() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Load Configuration",
        last_config_path_,
        "Configuration Files (*.cfg);;All Files (*)"
    );

    if (!filename.isEmpty()) {
        loadConfig(filename);
    }
}

void MainWindow::onSaveConfig() {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Configuration",
        last_config_path_,
        "Configuration Files (*.cfg);;All Files (*)"
    );

    if (!filename.isEmpty()) {
        saveConfig(filename);
    }
}

void MainWindow::onAbout() {
    QString info = QString(
        "<h3>LSL Template</h3>"
        "<p>Version 2.0.0</p>"
        "<p>A template application for Lab Streaming Layer.</p>"
        "<hr>"
        "<p><b>LSL Library:</b> %1</p>"
        "<p><b>Protocol:</b> %2</p>"
    ).arg(QString::number(lsl::library_version()),
          QString::fromStdString(lsl::library_info()));

    QMessageBox::about(this, "About LSL Template", info);
}

void MainWindow::loadConfig(const QString& filename) {
    auto config = lsltemplate::ConfigManager::load(filename.toStdString());

    if (config) {
        ui_->input_name->setText(QString::fromStdString(config->stream_name));
        ui_->input_type->setText(QString::fromStdString(config->stream_type));
        ui_->input_channels->setValue(config->channel_count);
        ui_->input_srate->setValue(config->sample_rate);
        ui_->input_device->setValue(config->device_param);
        last_config_path_ = filename;
        updateStatus("Loaded: " + filename, false);
    } else {
        QMessageBox::warning(
            this,
            "Load Failed",
            QString("Failed to load configuration from:\n%1").arg(filename)
        );
    }
}

void MainWindow::saveConfig(const QString& filename) {
    lsltemplate::AppConfig config{
        .stream_name = ui_->input_name->text().toStdString(),
        .stream_type = ui_->input_type->text().toStdString(),
        .channel_count = ui_->input_channels->value(),
        .sample_rate = ui_->input_srate->value(),
        .device_param = ui_->input_device->value()
    };

    if (lsltemplate::ConfigManager::save(config, filename.toStdString())) {
        last_config_path_ = filename;
        updateStatus("Saved: " + filename, false);
    } else {
        QMessageBox::warning(
            this,
            "Save Failed",
            QString("Failed to save configuration to:\n%1").arg(filename)
        );
    }
}

QString MainWindow::findDefaultConfigFile() {
    QFileInfo exe_info(QCoreApplication::applicationFilePath());
    QString default_name = exe_info.completeBaseName() + ".cfg";

    QStringList search_paths = {
        QDir::currentPath(),
        exe_info.absolutePath()
    };
    search_paths.append(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation));

    for (const auto& path : search_paths) {
        QString full_path = path + QDir::separator() + default_name;
        if (QFileInfo::exists(full_path)) {
            return full_path;
        }
    }

    return QString();
}

void MainWindow::updateStatus(const QString& message, bool is_error) {
    ui_->statusbar->showMessage(message, is_error ? 0 : 5000);

    if (is_error) {
        ui_->statusbar->setStyleSheet("color: red;");
    } else {
        ui_->statusbar->setStyleSheet("");
    }
}

void MainWindow::setStreaming(bool streaming) {
    ui_->linkButton->setText(streaming ? "Unlink" : "Link");

    // Disable config inputs while streaming
    ui_->input_name->setEnabled(!streaming);
    ui_->input_type->setEnabled(!streaming);
    ui_->input_channels->setEnabled(!streaming);
    ui_->input_srate->setEnabled(!streaming);
    ui_->input_device->setEnabled(!streaming);
}
