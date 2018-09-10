/*: # mainwindow.cpp ([source](../appskeleton/mainwindows.cpp))
 * mainwindow.cpp contains the implementations of everything your window does.
 *
 * `ui_mainwindow.h` is automatically generated from `mainwindow.ui`.
 * It defines the `Ui::MainWindow` class with the widgets as members. */
#include "ui_mainwindow.h"
/*: The next two includes are our own headers that define the interfaces for
 * our window class and the recording device */
#include "mainwindow.h"
#include "sophisticated_recording_device.h"


//: standard C++ headers
#include <fstream>
#include <string>
#include <vector>
//: Qt headers
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
//: the liblsl header
#include <lsl_cpp.h>


/*: The constructor mainly sets up the `Ui::MainWindow` class and creates the
 * connections between signals (e.g. 'button X was clicked') and slots
 * (e.g. 'close the window') or functions (e.g. 'save the configuration')
 */
MainWindow::MainWindow(QWidget* parent, const char* config_file)
    : QMainWindow(parent), recording_thread(nullptr), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	/*: C++11 has anonymous functions [lambdas](http://en.cppreference.com/w/cpp/language/lambda)
	 * that can get defined once where they are needed. They are mainly useful
	 * for simple actions as a result of an event */
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


	//: At the end of the constructor, we load the supplied config file
	load_config(config_file);
}


/*: ## Loading / saving the configuration
 * Most apps have some kind of configuration parameters, e.g. which device to
 * use, how to name the channels, ...
 *
 * The settings are mostly saved in `.ini` files. Qt ships with a parser and
 * writer for these kinds of files ([QSettings](http://doc.qt.io/qt-5/qsettings.html)).
 * The general format is `settings.value("key", "default value").toType()`*/
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


/*: ## The close event
 * to avoid accidentally closing the window, we can ignore the close event
 * when there's a recording in progress */
void MainWindow::closeEvent(QCloseEvent* ev) {
	if (recording_thread) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}


/*: ## The recording thread
 *
 * We run the recording in a separate thread to keep the UI responsive.
 * The recording thread function generally gets called with
 *
 * - the configuration parameters (here `name`, `device_param`)
 * - a reference to an `std::atomic<bool>`
 *
 * the shutdown flag indicates that the recording should stop as soon as possible */
void recording_thread_function(std::string name, int32_t device_param,
                               std::atomic<bool>& shutdown) {
	//: create an outlet and a send buffer
	lsl::stream_info info(name, "Counter", 1, 10, lsl::cf_int32);
	lsl::stream_outlet outlet(info);
	std::vector<int32_t> buffer(1, 20);


	//: Connect to the device, depending on the SDK you might also have to
	//: create a device object and connect to it via a method call
	sophisticated_recording_device device(device_param);


	/*: the recording loop. The logic here is as follows:
	 * - acquire some data
	 * - copy it to the buffer (here in one step)
	 * - push it to the outlet
	 * - do that again unless the shutdown flag was set in the meantime */
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


//: ## Toggling the recording state
//: Our record button has two functions: start a recording and
//: stop it if a recording is running already.
void MainWindow::toggleRecording() {
	/*: the `std::unique_ptr` evaluates to false if it doesn't point to an object,
	 * so we need to start a recording.
	 * First, we load the configuration from the UI fields, set the shutdown flag
	 * to false so the recording thread doesn't quit immediately and create the
	 * recording thread. */
	if (!recording_thread) {
		// read the configuration from the UI fields
		std::string name = ui->nameField->text().toStdString();
		int32_t device_param = (int32_t)ui->deviceField->value();

		shutdown = false;
		/*: `make_unique` allocates a new `std::thread` with our recording
		 * thread function as first parameters and all parameters to the
		 * function after that.
		 * Reference parameters have to be wrapped as a `std::ref`. */
		recording_thread = std::make_unique<std::thread>(&recording_thread_function, name,
		                                                 device_param, std::ref(shutdown));
		ui->linkButton->setText("Unlink");
	}
	/*: Shutting a thread involves 3 things:
	 * - setting the shutdown flag so the thread doesn't continue acquiring data
	 * - wait for the thread to complete (`join()`)
	 * - delete the thread object and set the variable to nullptr */
	else {
		shutdown = true;
		recording_thread->join();
		recording_thread.reset();
		ui->linkButton->setText("Link");
	}
}


//: Tell the compiler to put the default destructor in this object file
MainWindow::~MainWindow() noexcept = default;
