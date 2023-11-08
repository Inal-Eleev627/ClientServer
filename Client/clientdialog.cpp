#include "clientdialog.h"
#include "ui_clientdialog.h"

ClientDialog::ClientDialog(const std::string& id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientDialog),
    _sent_model(this),
    _recieved_model(this)
{
    ui->setupUi(this);
    setWindowTitle(id.c_str());

    // Setup list viewes to be non-editable
    ui->sent_list_view->setModel(&_sent_model);
    ui->sent_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->recieved_list_view->setModel(&_recieved_model);
    ui->recieved_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ClientDialog::ClearContent()
{
    _sent_model.removeRows(0, _sent_model.rowCount());
    _recieved_model.removeRows(0, _recieved_model.rowCount());
}

void ClientDialog::AddLine2SentListView(const QString& str)
{
    const auto row = _sent_model.rowCount();
    _sent_model.insertRow(row);

    const auto index = _sent_model.index(row, 0);
    _sent_model.setData(index, str);
    ui->sent_list_view->setCurrentIndex(index);
}

void ClientDialog::AddLine2RecievedListView(const QString& str)
{
    const auto row = _recieved_model.rowCount();
    _recieved_model.insertRow(row);

    const auto index = _recieved_model.index(row, 0);
    _recieved_model.setData(index, str);
    ui->recieved_list_view->setCurrentIndex(index);
}


ClientDialog::~ClientDialog()
{
    delete ui;
}
