// /* This file is part of the KDE project
//     SPDX-FileCopyrightText: 2023 Stefano Crocco <stefano.crocco@alice.it>
// 
//     SPDX-License-Identifier: LGPL-2.0-or-later
// */

#include "choosepagesaveformatdlg.h"
#include "ui_choosepagesaveformatdlg.h"

#include <QButtonGroup>

ChoosePageSaveFormatDlg::ChoosePageSaveFormatDlg(QWidget* parent)
    : m_ui(new Ui::ChoosePageSaveFormatDlg), m_choicesGroup(new QButtonGroup(this))
{
    m_ui->setupUi(this);
    m_choicesGroup->addButton(m_ui->m_singleHTMLPage, QWebEngineDownloadItem::SavePageFormat::SingleHtmlSaveFormat);
    m_choicesGroup->addButton(m_ui->m_asDirectory, QWebEngineDownloadItem::SavePageFormat::CompleteHtmlSaveFormat);
    m_choicesGroup->addButton(m_ui->m_asMHTMLPage, QWebEngineDownloadItem::SavePageFormat::MimeHtmlSaveFormat);
    connect(m_choicesGroup, &QButtonGroup::idClicked, this, &ChoosePageSaveFormatDlg::updateInfoText);
    updateInfoText(m_choicesGroup->checkedId());
}

ChoosePageSaveFormatDlg::~ChoosePageSaveFormatDlg()
{
}

QWebEngineDownloadItem::SavePageFormat ChoosePageSaveFormatDlg::choosenFormat() const
{
    return static_cast<QWebEngineDownloadItem::SavePageFormat>(m_choicesGroup->checkedId());
}

void ChoosePageSaveFormatDlg::updateInfoText(int id)
{
    QString info;
    switch (id) {
        case QWebEngineDownloadItem::SingleHtmlSaveFormat:
            info = i18n("The page will be saved as a single HTML file. Only the text of the page will be saved. External resources such as images won't be saved");
            break;
        case QWebEngineDownloadItem::CompleteHtmlSaveFormat:
            info = i18n("The page will be saved as an HTML file. Any external resources the page requires, such as images, will be saved inside an apposite directory");
            break;
        case QWebEngineDownloadItem::MimeHtmlSaveFormat:
            info = i18n("The page will be saved as a MTHML file which contains both the text of the page and any external resources it requires, such as images. <br><strong>Note</strong>: not all browsers are able to display this kind of files");
            break;
        case QWebEngineDownloadItem::UnknownSaveFormat: //This should never happen
            info = QString();
            break;
    }
    m_ui->m_info->setText(info);
}
