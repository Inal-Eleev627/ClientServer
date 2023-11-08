#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QWidget>
#include <QStringListModel>

namespace Ui {
class ClientDialog;
}

class ClientDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ClientDialog(const std::string& id, QWidget *parent = nullptr);
    ~ClientDialog();

public slots:
    void ClearContent();
    void AddLine2SentListView(const QString& str);
    void AddLine2RecievedListView(const QString& str);

private:
    Ui::ClientDialog *ui;
    QStringListModel _sent_model;
    QStringListModel _recieved_model;
};

#endif // CLIENTDIALOG_H
