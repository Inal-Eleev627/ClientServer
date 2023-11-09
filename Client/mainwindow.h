#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>

#include <thread>
#include <vector>
#include <array>

#include "clientstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setConnectionWidgetsEnable(const bool enable);
    void setupClientsComboBox();
    void setupClientsStatuses(const QString& value);

private slots:
    void try_connect();
    void disconnect();
    void update_endpoint(const QString&);

private:
    // number of clients depends on hardware concurrency
    const std::uint32_t _jobs = std::thread::hardware_concurrency();

    // TODO: Replace it with TableView.
    QGridLayout _grid_layout_clients;

    // TODO: Make TableModel and Delegate for ClientCell.
    using rowClients_t = std::array<ClientCell, 5>;
    using clients2dim_t = std::vector<rowClients_t>;

    // all clients container with their position in layout
    clients2dim_t _clients;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
