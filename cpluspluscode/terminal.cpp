#include "terminal.h"

#include <QScrollBar>

Terminal::Terminal(QString title){
    this->title=title;
//   this->interpreter=interpreter;
    int ow = 480;
   int h = 480;
   this->window = new QWidget();
   this->icon = new QIcon("./images/icon.png");
   this->window->setWindowIcon(*this->icon);
   this->window->resize(ow, h);
   this->window->setWindowTitle(title);
   this->layout = new QVBoxLayout(window);
   this->label = new QTextEdit("");
   this->browser = new QTextBrowser();
   this->browser->setDocument(this->label->document());
   this->layout->addWidget(this->browser);
   this->window->setStyleSheet("background-color: black;color:blue;text-decoration: bold;font-family:Courier New;font-size:16px;");
   this->window->show();
   //GUIRequest* guiRequest = new GUIRequest();
   //emit this->interpreter->guiRequest(guiRequest);
}

void Terminal::out(QString str){
//   this->browser->setHtml(browser->toHtml()+str+"<br>");
    this->browser->setHtml(browser->toHtml()+str
                           );
    this->browser->verticalScrollBar()->setSliderPosition(this->browser->verticalScrollBar()->maximum());
 //   this->label->setText(this->label->toPlainText()+str+"\n");
}

void Terminal::clear(){
    this->browser->setText("");
}
