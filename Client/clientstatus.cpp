#include "clientstatus.h"
#include "ui_clientstatus.h"

#include <format>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace
{

// Random data generator based on the English alphabet from ASCII
void random_data_handler(std::string& data)
{
    data = std::move(std::format("{:%F %T}", std::chrono::system_clock::now()));
    data += " Random data: \"";

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec);

    const auto length = (rand() % 49) + 1;
    for (auto i = 0; i < length; i++)
    {
        char c = (rand() % 25) + 'a';
        data += c;
    }

    data += "\"";
}

}

ClientStatus::ClientStatus(const std::string& id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientStatus),
    _dialog(std::make_unique<ClientDialog>(id)),
    _id(id)
{
    ui->setupUi(this);
    ui->clientStatus_dialog_button->setText(id.c_str());
    ui->disconnect_button->setEnabled(false);

    QPalette p{};
    p.setBrush(QPalette::Highlight, QBrush(Qt::darkRed));

    ui->clientStatus_progress_bar->setPalette(p);
    ui->clientStatus_progress_bar->setMaximum(0);
    ui->clientStatus_progress_bar->setMinimum(0);
    ui->clientStatus_progress_bar->setValue(0);

    connect(ui->clientStatus_dialog_button, &QPushButton::clicked, this, &ClientStatus::openDialog);
    connect(ui->connect_button, &QPushButton::clicked, this, &ClientStatus::Connect);
    connect(ui->disconnect_button, &QPushButton::clicked, this, &ClientStatus::Disconnect);
}

void ClientStatus::SetEndpoint(const QString& ip, const QString& port)
{
    _ip = ip;
    _port = port;
}

void ClientStatus::Connect()
{
    if (!_tpc_client_ptr)
    {
        try
        {
            // Every connect should make new tcp client, and deleted on disconnection
            _tpc_client_ptr = std::make_unique<TcpClient>(_ip.toStdString(), _port.toStdString(), random_data_handler);
            _tpc_client_ptr->Start();
        }
        catch (const std::runtime_error& re)
        {
            _error_msg = std::make_unique<QErrorMessage>();

            auto msg = std::move(std::format("Cannot resolve connection. Error: {}", re.what()));
            _error_msg->showMessage(QString(msg.c_str()));
            return;
        }

        connect(_tpc_client_ptr.get(), &TcpClient::ReadData, _dialog.get(), &ClientDialog::AddLine2RecievedListView);
        connect(_tpc_client_ptr.get(), &TcpClient::WroteData, _dialog.get(), &ClientDialog::AddLine2SentListView);
        connect(_tpc_client_ptr.get(), &TcpClient::StatusChanged, this, &ClientStatus::SetClientStatusColor);
    }

    ui->connect_button->setEnabled(false);
    ui->disconnect_button->setEnabled(true);
}

void ClientStatus::Disconnect()
{
    _tpc_client_ptr.reset();
    ui->connect_button->setEnabled(true);
    ui->disconnect_button->setEnabled(false);
}

void ClientStatus::SetClientStatusColor(const int status)
{
    Qt::GlobalColor color{};
    switch (static_cast<TcpClientStatus>(status))
    {
    case TcpClientStatus::idle:
        color = Qt::darkBlue;
        break;
    case TcpClientStatus::connected:
        color = Qt::darkGreen;
        break;
    case TcpClientStatus::disconnected:
    default:
        color = Qt::darkRed;
    }
    QPalette p{};
    p.setBrush(QPalette::Highlight, QBrush(color));
    ui->clientStatus_progress_bar->setPalette(p);
}

void ClientStatus::openDialog()
{
    _dialog->show();
}

ClientStatus::~ClientStatus()
{
    delete ui;
}
