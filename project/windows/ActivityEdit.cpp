#include "ActivityEdit.h"
#include "ui_Activity_edit.h"
#include <QMessageBox>
#include <QString>
#include <QRegExpValidator>
#include <QLocale>

ActivityEdit::ActivityEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActivityEdit)
{
    ui->setupUi(this);

    ui->lineEdit_activityName->setValidator(new QRegExpValidator(QRegExp("[A-Z/a-z/а-я/A-Я]{1,}\[A-Z/a-z/а-я/A-Я\\s]{1,}")));
    ui->lineEdit_calories->setValidator(new QDoubleValidator(0.0, 100000.0, 2));

    connect(ui->pushButton_save, SIGNAL(pressed()), SLOT(onPushButtonSave()));
    connect(ui->pushButton_cancel, SIGNAL(pressed()), SLOT(onPushButtonCancel()));
}

void ActivityEdit::setInformation(const ActivityEntity &a)
{
    _isEditingMod = true;

    _activity.setId(a.id());

    ui->lineEdit_activityName->setText(a.type());
    ui->lineEdit_calories->setText(QLocale::system().toString(a.kkm()));

    this->repaint();
}

void ActivityEdit::onPushButtonSave()
{
    QString errorLog;

    if (!ui->lineEdit_activityName->hasAcceptableInput()){
        errorLog += "Неправильный формат названия\n";
    }
    if (!ui->lineEdit_calories->hasAcceptableInput()){
        errorLog += "Неправильный формат Ккал/кг/мин\n";
    }
    if (!errorLog.isEmpty()){
        QMessageBox::critical(this, "Ошибка заполнения", errorLog);
        return;
    }

    QString activityName = ui->lineEdit_activityName->text();
    float kkm = QLocale::system().toDouble(ui->lineEdit_calories->text());

    _activity = ActivityEntity(_activity.id(), activityName, kkm);

    if (_isEditingMod) {
        emit formEditedActivityReady();
    } else {
        emit formNewActivityReady();
    }

    this->close();
}

void ActivityEdit::onPushButtonCancel()
{
    this->parent()->deleteLater();
}

ActivityEntity ActivityEdit::activity() const
{
    return _activity;
}

ActivityEdit::~ActivityEdit()
{
    delete ui;
}
