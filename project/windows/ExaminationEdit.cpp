#include "ExaminationEdit.h"
#include <QDebug>
#include <QMessageBox>

ExaminationEdit::ExaminationEdit(QWidget *wgt)
    :QWidget(wgt)
{
    _ui.setupUi(this);

    _ui.pushButton_saveForm->setEnabled(false);
    _ui.pushButton_previousPage->setEnabled(false);
    m_isEditingMod = false;

    setupValidators();

    connect(_ui.pushButton_nextPage, SIGNAL(pressed()), SLOT(onNextPage()));
    connect(_ui.pushButton_previousPage, SIGNAL(pressed()), SLOT(onPrevPage()));
    connect(_ui.pushButton_saveForm, SIGNAL(pressed()), SLOT(onSaveForm()));
    connect(_ui.pushButton_calculate_formfield_62, SIGNAL(pressed()), SLOT(onPushButtonCalculate_65()));
    connect(_ui.pushButton_calculate_formfield_66_74, SIGNAL(pressed()), SLOT(onPushButtonCalculate_69_77()));
    connect(_ui.formfield_87, SIGNAL(currentIndexChanged(int)), SLOT(onComboBoxChanged_87(int)));
}

void ExaminationEdit::setInformation(const Client &client, bool isFullExamination)
{
    m_isEditingMod = false;
    _examination.setIsFullExamination(isFullExamination);
    _examination.setClient(client);

    QString clientInfo = QString("%1 %2 %3 / %4 (%5)")
            .arg(client.surname())
            .arg(client.name())
            .arg(client.patronymic())
            .arg(client.age())
            .arg(client.birthDate().year());
    _ui.label_client_info->setText(clientInfo);
    m_gender = client.gender();
    if (m_gender == 'm') {
        _ui.page_3->setEnabled(false);
    }

    QString examinationType = isFullExamination
            ? tr("Прием")
            : tr("Консультация");
    _ui.label_examination_type->setText(examinationType);

    if (!isFullExamination) {
        foreach (FormField field, _examination.fields()) {
            QWidget* widgetField = this->findChild<QWidget*>(field.name());
            if (field.isMayBeEmpty()) {
                widgetField->setEnabled(false);
                QLineEdit *tmpLine =  dynamic_cast<QLineEdit*>(widgetField);
                if (tmpLine != nullptr){
                    tmpLine->setText("--пусто--");
                }
            }
        }
        _ui.pushButton_calculate_formfield_62->setEnabled(false);
    }

    this->repaint();
}

void ExaminationEdit::setInformation(const Examination& examination)
{
    m_isEditingMod = true;
    _examination = examination;
    Client client = _examination.client();
    bool isFullExamination = _examination.isFullExamination();

    QString clientInfo = QString("%1 %2 %3 / %4 (%5)")
            .arg(client.surname())
            .arg(client.name())
            .arg(client.patronymic())
            .arg(client.age())
            .arg(client.birthDate().year());
    _ui.label_client_info->setText(clientInfo);
    m_gender = client.gender();
    if (m_gender == 'm') {
        _ui.page_3->setEnabled(false);
    }

    QString examinationType = isFullExamination
            ? tr("Прием")
            : tr("Консультация");
    _ui.label_examination_type->setText(examinationType);

    if (!isFullExamination) {
        _ui.pushButton_calculate_formfield_62->setEnabled(false);
    }

    foreach (FormField field, _examination.fields()) {
        QWidget* widgetField = this->findChild<QWidget*>(field.name());

        if (field.isMayBeEmpty() && !isFullExamination) {
            widgetField->setEnabled(false);
            QLineEdit *tmpLine =  dynamic_cast<QLineEdit*>(widgetField);
            if (tmpLine != nullptr){
                tmpLine->setText("--пусто--");
            }
            continue;
        }

        switch (field.type()) {
        case FormField::String : {
            ((QTextEdit*)widgetField)->setText(field.value());
        } break;
        case FormField::Date : {
            ((QDateEdit*)widgetField)->setDate(QDate::fromString(field.value(), "ddMMyyyy"));
        } break;
        case FormField::Float :
        case FormField::UShort : {
            ((QLineEdit*)widgetField)->setText(field.value());
        } break;
        case FormField::ComboB : {
            ((QComboBox*)widgetField)->setCurrentText(field.value());
        } break;
        }
    }

    this->repaint();
}

void ExaminationEdit::onPrevPage()
{
    int index = _ui.stackedWidget->currentIndex() - 1;
    if (((index == 3 || index == 6) && !_examination.isFullExamination()) || (_examination.isFullExamination() && m_gender == 'm' && index == 3)){
        index--;
    }
    setPage(index);
}

void ExaminationEdit::onNextPage()
{
    QWidget *page = _ui.stackedWidget->currentWidget();
    QObjectList lst = page->children();
    QObject *scrollArea = lst.at(1);
    QObjectList lst2 = scrollArea->children();
    QObject *scrollarea_viewport = lst2.at(0);
    QObjectList lst3 = scrollarea_viewport->children();
    QObject *scrollAreaWidgetContents = lst3.at(0);


    QString errStyle = "QWidget { background: rgb(255, 179, 179); }";
    bool isOpenErrDialog = false;


    foreach (FormField field, _examination.fields()) {
        bool isBlankField = false;

       QWidget* widgetField = scrollAreaWidgetContents->findChild<QWidget*>(field.name());
       if (!widgetField) {
           continue;
       }
       switch (field.type()) {
       case FormField::String : {
           QTextEdit* textField = (QTextEdit*)widgetField;
           if (textField->toPlainText().isEmpty() && textField != _ui.formfield_90 && textField != _ui.formfield_84) {
               isBlankField = true;
           }
       } break;
       case FormField::Date : {
           QDateEdit* dateField = (QDateEdit*)widgetField;
           if (dateField->date() == QDate(1800, 1, 1)) {
               isBlankField = true;
           }
       } break;
       case FormField::Float :
       case FormField::UShort : {
           QLineEdit* lineField = (QLineEdit*)widgetField;
           if (!lineField->hasAcceptableInput()) {
               isBlankField = true;
           }
       } break;
       case FormField::ComboB : {
           QComboBox* comboField = (QComboBox*)widgetField;
           if (comboField->currentIndex() == -1) {
               isBlankField = true;
           }
       } break;
       }

       if (isBlankField && widgetField->isEnabled()) {
           widgetField->setStyleSheet(errStyle);
           isOpenErrDialog = true;
       } else {
           widgetField->setStyleSheet("");
       }
    }
    this->repaint();

    if(isOpenErrDialog) {
        qDebug()<<"Ошибка заполнения"<<"Данные заполнены некорректно"<<endl;
        return;
    }
    int index = _ui.stackedWidget->currentIndex() + 1;
    if (((index == 3 || index == 6) && !_examination.isFullExamination()) || (_examination.isFullExamination() && m_gender == 'm' && index == 3)){
        index++;
    }
    setPage(index);
}

void ExaminationEdit::onSaveForm()
{
    /// Checking form values
    ///
    QString errStyle = "QWidget { background: rgb(255, 179, 179); }";
    bool isOpenErrDialog = false;

    foreach (FormField field, _examination.fields()) {
        bool isBlankField = false;

        QWidget* widgetField = this->findChild<QWidget*>(field.name());
        if (!widgetField) {
            qDebug() << "Error: ExaminationEdit::onSaveForm()"
                     << "Invalid conversion - " + field.name();
            return;
        }

        switch (field.type()) {
        case FormField::String : {
            QTextEdit* textField = (QTextEdit*)widgetField;
            if (textField->toPlainText().isEmpty() && textField != _ui.formfield_90 && textField != _ui.formfield_84) {
                isBlankField = true;
            }
        } break;
        case FormField::Date : {
            QDateEdit* dateField = (QDateEdit*)widgetField;
            if (dateField->date() == QDate(1800, 1, 1)) {
                isBlankField = true;
            }
        } break;
        case FormField::Float :
        case FormField::UShort : {
            QLineEdit* lineField = (QLineEdit*)widgetField;
            if (!lineField->hasAcceptableInput()) {
                isBlankField = true;
            }
        } break;
        case FormField::ComboB : {
            QComboBox* comboField = (QComboBox*)widgetField;
            if (comboField->currentIndex() == -1) {
                isBlankField = true;
            }
        } break;
        }

        if (isBlankField && widgetField->isEnabled()) {
            widgetField->setStyleSheet(errStyle);
            isOpenErrDialog = true;
        } else {
            widgetField->setStyleSheet("");
        }
    }
    this->repaint();

    if(isOpenErrDialog) {
        qDebug()<<"Ошибка заполнения"<<"Данные заполнены некорректно"<<endl;

        return;
    }

    /// Reading form values
    ///
    foreach (FormField field, _examination.fields()) {
        QWidget* widgetField = this->findChild<QWidget*>(field.name());
        QString fieldValue;

        switch (field.type()) {
        case FormField::String : {
            fieldValue = ((QTextEdit*)widgetField)->toPlainText();
        } break;
        case FormField::Date : {
            fieldValue = ((QDateEdit*)widgetField)->date().toString(Qt::ISODate);
        } break;
        case FormField::Float :
        case FormField::UShort : {
            fieldValue = ((QLineEdit*)widgetField)->text();
        } break;
        case FormField::ComboB : {
            fieldValue = ((QComboBox*)widgetField)->currentText();
        } break;
        }

        _examination.setFieldValue(field.name(), fieldValue);
    }

    _examination.setDate(QDateTime::currentDateTime());

    //this->close();

    if (m_isEditingMod) {
        emit formEditedExaminationReady();
    } else {
        emit formNewExaminationReady();
    }
}

void ExaminationEdit::onPushButtonCalculate_65()
{
    if (_ui.formfield_65->isEnabled()){
        int res =( _ui.formfield_63->text().toFloat()
                     - _ui.formfield_63->text().toFloat()) / 0.0001 * 100;
        _ui.formfield_65->setText(QString::number(res));
    }
}

void ExaminationEdit::onPushButtonCalculate_69_77()
{
    QVector<float> values = {
        _ui.formfield_47->text().toFloat() / powf(_ui.formfield_46->text().toFloat(), 2) * 10000,
        0.45,//TODO:Calculate
        _ui.formfield_44->text().toFloat() / _ui.formfield_45->text().toFloat(),
        (_ui.formfield_66->text().toFloat() + _ui.formfield_67->text().toFloat()) / 2.f / _ui.formfield_47->text().toFloat(),
        _ui.formfield_68->text().toFloat() / _ui.formfield_47->text().toFloat(),
        _ui.formfield_63->text().toFloat() * _ui.formfield_57->text().toFloat() / 100.f,
        0.56,//TODO:Calculate
        0.64,//TODO:Calculate
        0.011*_ui.formfield_62->text().toFloat()
            + 0.014*_ui.formfield_56->text().toFloat()
            + 0.008*_ui.formfield_58->text().toFloat()
            + 0.009*_ui.formfield_47->text().toFloat()
            - 0.009*_ui.formfield_46->text().toFloat()
            + 0.014*_examination.client().age() - 0.27
    };

    for (int i = 69; i <= 77; ++i) {
        QTextEdit* field = this->findChild<QTextEdit*>(QString("formfield_%1").arg(i));
        if (!field) {
            qDebug() << "Error: ExaminationEdit::onPushButtonCalculate_66_72()"
                     << QString("Invalid conversion - formfield_%1").arg(i);
            return;
        }
        if(field->isEnabled()){
            field->setText(QString::number(values[i-69]));      //TODO:Calculate
        }
    }
}

void ExaminationEdit::onComboBoxChanged_87(int index)
{
    if (index == 3){ // if ComboBoxChanched_87 curent index - 3
        _ui.formfield_88->setEnabled(true);
        _ui.formfield_89->setEnabled(true);
    } else {
        _ui.formfield_88->setEnabled(false);
        _ui.formfield_89->setEnabled(false);
        _ui.formfield_88->setCurrentIndex(-1);
        _ui.formfield_89->setCurrentIndex(-1);
    }
}

void ExaminationEdit::setPage(int index)
{
    static int lastPage = _ui.stackedWidget->count() - 1;
    static int firstPage = 0;

    if(index == firstPage) {
        _ui.pushButton_previousPage->setEnabled(false);
    }
    else if (index == lastPage) {
        _ui.pushButton_saveForm->setEnabled(true);
        _ui.pushButton_nextPage->setEnabled(false);
    }
    else if (index > firstPage && index < lastPage) {
        _ui.pushButton_nextPage->setEnabled(true);
        _ui.pushButton_saveForm->setEnabled(false);
        _ui.pushButton_previousPage->setEnabled(true);
    }
    else {
        return; // set index not available
    }

    _ui.stackedWidget->setCurrentIndex(index);
    _ui.progressBar->setValue(index+1);
}

void ExaminationEdit::setupValidators()
{
    _ui.formfield_1->setValidator(new QIntValidator(40, 300));
    _ui.formfield_2->setValidator(new QIntValidator(40, 300));
    _ui.formfield_3->setValidator(new QIntValidator(40, 300));
    _ui.formfield_4->setValidator(new QIntValidator(40, 300));
    _ui.formfield_27->setValidator(new QIntValidator(0, 100));
    _ui.formfield_30->setValidator(new QIntValidator(20, 40));
    _ui.formfield_31->setValidator(new QIntValidator(20, 40));
    _ui.formfield_33->setValidator(new QIntValidator(1000, 2999));
    _ui.formfield_35->setValidator(new QIntValidator(1, 10));
    _ui.formfield_36->setValidator(new QIntValidator(1, 10));
    _ui.formfield_42->setValidator(new QDoubleValidator(60.0,   200.0,  1));
    _ui.formfield_43->setValidator(new QDoubleValidator(60.0,   200.0,  1));
    _ui.formfield_44->setValidator(new QDoubleValidator(60.0,   200.0,  1));
    _ui.formfield_45->setValidator(new QDoubleValidator(60.0,   200.0,  1));
    _ui.formfield_46->setValidator(new QDoubleValidator(100.0,  200.0,  1));
    _ui.formfield_47->setValidator(new QDoubleValidator(30.0,   300.0,  1));
    _ui.formfield_48->setValidator(new QDoubleValidator(20.0,   100.0,  1));
    _ui.formfield_49->setValidator(new QDoubleValidator(20.0,   100.0,  1));
    _ui.formfield_50->setValidator(new QDoubleValidator(10.0,   60.0,   1));
    _ui.formfield_51->setValidator(new QDoubleValidator(10.0,   60.0,   1));
    _ui.formfield_52->setValidator(new QDoubleValidator(30.0,   100.0,  1));
    _ui.formfield_53->setValidator(new QDoubleValidator(30.0,   100.0,  1));
    _ui.formfield_54->setValidator(new QDoubleValidator(20.0,   50.0,   1));
    _ui.formfield_55->setValidator(new QDoubleValidator(20.0,   50.0,   1));
    _ui.formfield_56->setValidator(new QIntValidator(60, 220));
    _ui.formfield_57->setValidator(new QIntValidator(60, 220));
    _ui.formfield_58->setValidator(new QIntValidator(40, 150));
    _ui.formfield_59->setValidator(new QIntValidator(40, 150));
    _ui.formfield_60->setValidator(new QIntValidator(20, 100));
    _ui.formfield_61->setValidator(new QIntValidator(20, 100));
    _ui.formfield_62->setValidator(new QIntValidator(30, 150));
    _ui.formfield_63->setValidator(new QIntValidator(30, 150));
    _ui.formfield_64->setValidator(new QIntValidator(30, 360));
    _ui.formfield_66->setValidator(new QDoubleValidator(2.0,    100.0,  1));
    _ui.formfield_67->setValidator(new QDoubleValidator(2.0,    100.0,  1));
    _ui.formfield_68->setValidator(new QDoubleValidator(100.0,  5000.0, 1));
    _ui.formfield_78->setValidator(new QDoubleValidator(3.33,   5.55,   2));
    _ui.formfield_79->setValidator(new QDoubleValidator(3.6,    7.8,    1));
    _ui.formfield_80->setValidator(new QDoubleValidator(65.0,   85.0,   1));
    _ui.formfield_81->setValidator(new QDoubleValidator(15.25,  76.25,  2));
    _ui.formfield_82->setValidator(new QDoubleValidator(200.0,  400.0,  1));
    _ui.formfield_83->setValidator(new QDoubleValidator(2.0,    11.1,   1));
}

Examination ExaminationEdit::examination() const
{
    return _examination;
}
