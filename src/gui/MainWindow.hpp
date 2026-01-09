#pragma once
/**
 * @file MainWindow.hpp
 * @brief Main window for LSL Template GUI application
 */

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

namespace lsltemplate {
class StreamThread;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& config_file = QString(), QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLinkButtonClicked();
    void onLoadConfig();
    void onSaveConfig();
    void onAbout();

private:
    void loadConfig(const QString& filename);
    void saveConfig(const QString& filename);
    QString findDefaultConfigFile();
    void updateStatus(const QString& message, bool is_error);
    void setStreaming(bool streaming);

    std::unique_ptr<Ui::MainWindow> ui_;
    std::unique_ptr<lsltemplate::StreamThread> stream_;
    QString last_config_path_;
};
