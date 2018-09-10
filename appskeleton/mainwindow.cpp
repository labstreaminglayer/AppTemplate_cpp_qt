#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "sophisticated_recording_device.h"

#include <fstream>
#include <string>
#include <vector>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <lsl_cpp.h>

MainWindow::MainWindow(QWidget* parent, const char* config_file)
    : QMainWindow(parent), recording_thread(nullptr), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	connect(ui->actionLoad_Configuration, &QAction::triggered, [this]() {
		load_config(QFileDialog::getOpenFileName(this, "Load Configuration File", "",
		                                         "Configuration Files (*.cfg)"));
	});
	connect(ui->actionSave_Configuration, &QAction::triggered, [this]() {
		save_config(QFileDialog::getSaveFileName(this, "Save Configuration File", "",
		                                         "Configuration Files (*.cfg)"));
	});
	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actionAbout, &QAction::triggered, [this]() {
		QString infostr = QStringLiteral("LSL library version: ") +
		                  QString::number(lsl::library_version()) +
		                  "\nLSL library info:" + lsl::lsl_library_info();
		QMessageBox::about(this, "About LabRecorder", infostr);
	});
	connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

	load_config(config_file);
}

void MainWindow::load_config(const QString& filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	ui->nameField->setText(settings.value("BPG/name", "Default name").toString());
	ui->deviceField->setValue(settings.value("BPG/device", 0).toInt());
}
void MainWindow::save_config(const QString& filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("BPG");
	settings.setValue("name", ui->nameField->text());
	settings.setValue("device", ui->deviceField->value());
	settings.sync();
}


void MainWindow::closeEvent(QCloseEvent* ev) {
	if (recording_thread) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}

void recording_thread_function(std::string name, int32_t device_param,
                               std::atomic<bool>& shutdown) {
	lsl::stream_info info(name, "Counter", 1, 10, lsl::cf_int32);
	lsl::stream_outlet outlet(info);
	std::vector<int32_t> buffer(1, 20);

	sophisticated_recording_device device(device_param);

	while (!shutdown) {
		// "Acquire data"
		if (device.getData(buffer)) {
			outlet.push_chunk_multiplexed(buffer);
		} else {
			// Acquisition was unsuccessful? -> Quit
			break;
		}
	}
}

void MainWindow::toggleRecording() {
	if (!recording_thread) {
		// read the configuration from the UI fields
		std::string name = ui->nameField->text().toStdString();
		int32_t device_param = (int32_t)ui->deviceField->value();
		shutdown = false;
		recording_thread = std::make_unique<std::thread>(&recording_thread_function, name,
		                                                 device_param, std::ref(shutdown));
		ui->linkButton->setText("Unlink");
	}
	else {
		shutdown = true;
		recording_thread->join();
		recording_thread.reset();
		ui->linkButton->setText("Link");
	}
}

MainWindow::~MainWindow() noexcept = default;
