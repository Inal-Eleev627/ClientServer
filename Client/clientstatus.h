#ifndef CLIENTSTATUS_H
#define CLIENTSTATUS_H

#include <QWidget>
#include <QErrorMessage>

#include "clientdialog.h"
#include "tcpclient.h"

namespace Ui {
class ClientStatus;
}

class ClientStatus : public QWidget
{
    Q_OBJECT

public:
    explicit ClientStatus(const std::string& id, QWidget *parent = nullptr);
    ~ClientStatus();

    void SetEndpoint(const QString& ip, const QString& port);

public slots:
    void Connect();
    void Disconnect();
    void SetClientStatusColor(const int);

private slots:
    void openDialog();

private:
    Ui::ClientStatus *ui;
    std::string _id;
    QString _ip;
    QString _port;

    std::unique_ptr<ClientDialog> _dialog;
    std::unique_ptr<TcpClient> _tpc_client_ptr;
    std::unique_ptr<QErrorMessage> _error_msg;
};

// client container with its position in layout
struct ClientCell
{
    std::unique_ptr<ClientStatus> status_ptr;
    int row{};
    int column{};
};

#endif // CLIENTSTATUS_H
