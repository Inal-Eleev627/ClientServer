#include "mainwindow.h"
#include "clientstatus.h"
#include "./ui_mainwindow.h"

#include <string>
#include <QHBoxLayout>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->clients_area_widget_contents->setLayout(&_grid_layout_clients);

    setWindowFlags(Qt::WindowMinimizeButtonHint);
    setupClientsComboBox();
    setupClientsStatuses("1");

    connect(ui->clients_count_combo_box, &QComboBox::currentTextChanged, this, &MainWindow::setupClientsStatuses);
    connect(ui->line_edit_ip, &QLineEdit::textEdited, this, &MainWindow::update_endpoint);
    connect(ui->line_edit_port, &QLineEdit::textEdited, this, &MainWindow::update_endpoint);
    connect(ui->connect_all_button, &QPushButton::clicked, this, &MainWindow::try_connect);
    connect(ui->disconnect_all_button, &QPushButton::clicked, this, &MainWindow::disconnect);
}

void MainWindow::setConnectionWidgetsEnable(const bool enable)
{
    ui->connect_all_button->setEnabled(enable);
    ui->line_edit_ip->setEnabled(enable);
    ui->line_edit_port->setEnabled(enable);
    ui->clients_count_combo_box->setEnabled(enable);

    ui->disconnect_all_button->setEnabled(!enable);
}

void MainWindow::setupClientsComboBox()
{
    ui->clients_count_combo_box->setMinimumContentsLength(1);
    ui->clients_count_combo_box->setMaxCount(_jobs);
    for (auto i = 1u; i <= _jobs; ++i)
        ui->clients_count_combo_box->addItem(QString(std::to_string(i).c_str()));
}

void MainWindow::setupClientsStatuses(const QString& clients_count)
{
    for (auto& row : _clients)
    {
        for (auto& cell : row)
            _grid_layout_clients.removeWidget(cell.status_ptr.get());
    }

    _clients.clear();

    auto count = clients_count.toInt();
    int i = 0;
    do
    {
        rowClients_t row;
        for (auto j = 0; j < row.size() && count > 0; ++j)
        {
            ClientCell cell{};
            std::string id = "Client " + std::to_string(i) + "." + std::to_string(j);
            cell.status_ptr = std::make_unique<ClientStatus>(id, this);
            cell.row = i;
            cell.column = j;

            _grid_layout_clients.addWidget(
                        cell.status_ptr.get(),
                        cell.row, cell.column,
                        Qt::AlignLeft | Qt::AlignTop);

            cell.status_ptr->SetEndpoint(ui->line_edit_ip->text(), ui->line_edit_port->text());
            connect(ui->connect_all_button, &QPushButton::clicked, cell.status_ptr.get(), &ClientStatus::Connect);
            connect(ui->disconnect_all_button, &QPushButton::clicked, cell.status_ptr.get(), &ClientStatus::Disconnect);

            row[j] = std::move(cell);
            count--;
        }

        _clients.push_back(std::move(row));
        i++;
    }
    while (count > 0);
}

void MainWindow::try_connect()
{
    setConnectionWidgetsEnable(false);
}

void MainWindow::disconnect()
{
    setConnectionWidgetsEnable(true);
}

void MainWindow::update_endpoint(const QString&)
{
    for (auto& row : _clients)
    for (auto& client : row)
    {
        if (client.status_ptr)
        {
            client.status_ptr->SetEndpoint(
                ui->line_edit_ip->text(),
                ui->line_edit_port->text());
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

