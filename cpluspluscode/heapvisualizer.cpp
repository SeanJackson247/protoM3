#include "heapvisualizer.h"

#include <QGraphicsItem>
#include "prect.h"
#include "ptext.h"
#include "architecture.h"

HeapVisualizer* currentContext;

int SQUARE_SIZE=60;
int COL_HEIGHT=20;

void myCallback(PWidget* _this,QMouseEvent* mouseEvent){
/*    qDebug()<<"Hello from my callback!";
    qDebug()<<"Calc x and y....";
    int x = mouseEvent->x();
    int y = mouseEvent->y();
    x = x/SQUARE_SIZE;
    y = y/SQUARE_SIZE;
    int c = (x*SQUARE_SIZE)+y;
    qDebug()<<"Entry:"<<c<<" of "<<currentContext->entries.size();
    if(c<currentContext->entries.size()){
        qDebug()<<currentContext->entries[c];
    }
    currentContext->currentHighlighted = c;
    currentContext->redraw();*/
}

HeapVisualizer::HeapVisualizer():PWidget(400,400){
    currentContext=this;
    this->currentHighlighted=0;
    this->initThresholdSet=false;
    //this->pwidget = new PWidget(400,400);
    this->setTitle("Heap Visualizer");
    //this->pwidget->setMousePressCallback(myCallback);
    this->show();
/*    this->title="Heap Visualizer";
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
   this->heapCanvas = new HeapCanvas(this->window,this);
   this->layout->addWidget(this->browser);
   this->label->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
   this->label->resize(480,50);
   this->layout->addWidget(this->heapCanvas);
   this->heapCanvas->resize(480,100);
   this->heapCanvas->setStyleSheet("background-color: #ceceec");
   this->window->setStyleSheet("background-color: black;color:blue;text-decoration: bold;font-family:Courier New;font-size:16px;");
   this->window->show();
   this->label->setText("Heap Size: loading...");
   this->size = 0;*/
}

HeapVisualizer::setHeapSize(QString size){
  //  this->label->setText("Heap Size: "+size);
    this->size=size.toInt();
}

void HeapVisualizer::setInitThreshold(int i){
    this->initThresholdSet=true;
    this->initThreshold=i;
}

void HeapVisualizer::redraw(){
/*    if(this->entries.size()>0){
        this->draw(0,this->entries[0]);
    }
    else{
        qDebug()<<"Error could not redraw";
    }*/
}

void HeapVisualizer::draw(int location,QString prettyByte){


    /*

//    QPen pen(Qt::red);
 //   QBrush brush(Qt::red);
  //  int x=location;
//    int y=location;
    int w=SQUARE_SIZE;
    int h=SQUARE_SIZE;
    int x = location/COL_HEIGHT;
    int y = location%COL_HEIGHT;
//    int MAX_HEAP_SIZE = 1024*1024;
    QString colour="#ffffff";
    int size=0;
    if(location<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
        colour="#777777";
    }
    else if(this->initThresholdSet==true && location<this->initThreshold){
//        colour="#aaaaaa";
        if(QString::compare(prettyByte,"Type:Integer:Type")==0){
            colour="#fff0f0";
        }
        else if(QString::compare(prettyByte,"Type:Key:Type")==0){
            colour="#0f0fff";
        }
        else if(QString::compare(prettyByte,"Type:String:Type")==0){
            colour="#0fff0f";
        }
        else if(QString::compare(prettyByte,"Type:LettedScope:Type")==0){
            colour="#77eeee";
        }
        else if(QString::compare(prettyByte,"Type:FixedScope:Type")==0){
            colour="#ee77ee";
        }
        else if(prettyByte.length()==0){
            colour="#f1f11f";
        }
        else{
            colour="#aaaaaa";
            //size++;
        }
    }
    else if(QString::compare(prettyByte,"Type:Integer:Type")==0){
        colour="#ff0000";
    }
    else if(QString::compare(prettyByte,"Type:Key:Type")==0){
        colour="#0000ff";
    }
    else if(QString::compare(prettyByte,"Type:String:Type")==0){
        colour="#00ff00";
    }
    else if(QString::compare(prettyByte,"Type:LettedScope:Type")==0){
        colour="#c934b8";
    }
    else if(QString::compare(prettyByte,"Type:FixedScope:Type")==0){
        colour="#9734c9";
    }
    else if(prettyByte.length()==0){
        colour="#f0f00f";
    }
    else{
        colour="#22ccff";
        size++;
    }
    qDebug()<<"prettyByte:"<<prettyByte;
    int fs=5;
    if(location>=this->entries.size()){
        this->entries.resize(location+1);
        this->squares.resize(location+1);
        this->texts.resize(location+1);
        PRect* square = new PRect(this,x,y,w,h,colour,size);
        PText* text = new PText(this,prettyByte,x,y+5,fs,colour,200);
        this->squares[location]=square;
        this->texts[location]=text;
        //this->drawables.resize(this->drawables.size()+2);
        //this->drawables[this->drawables.size()]=square;
        //this->drawables[this->drawables.size()+1]=text;
        /*
        this->drawables.resize(location+2);
        this->drawables[location]=new PRect(this,x,y,w,h,colour,0);
        this->drawables[location+1]=new PText(this,prettyByte,x,y,fs,colour);*/
/*    }
    this->entries[location]=prettyByte;
    this->squares[location]->setValues(x,y,w,h,colour,size);
    colour="#000000";
    this->texts[location]->setValues(prettyByte,x,y+5,fs,colour,200);
//    this->drawables[0]->drawables[location]=
  //  this->repaint();

/*
//    this->pwidget->drawRect(x,y,10,10,"#"+prettyByte);

//    this->pwidget->fillRect(0,0,this->pwidget->innerWidth(),this->pwidget->innerHeight(),"#000000");

    if(location>=this->entries.size()){
        this->entries.resize(location+1);
    }
    this->entries[location]=prettyByte;
    this->fillRect(0,0,this->innerWidth(),this->innerHeight(),"#000000");
    this->drawText("Heap Size:"+QString::number(this->size),10,(SQUARE_SIZE*COL_HEIGHT)+10,14,"#00ff00");
    if(this->entries.size()>0){
       //QByteArray bytes;
       //bytes.resize(1);
       //bytes[0]=this->entries[this->currentHighlighted];
       this->drawText("Current Highighted("+QString::number(this->currentHighlighted)+"):'"+this->entries[this->currentHighlighted],10,(SQUARE_SIZE*COL_HEIGHT)+40,14,"#0000ff");
    }
    int i=0;
    while(i<this->entries.size()){
        int x = i/COL_HEIGHT;
        int y = i%COL_HEIGHT;
        if(this->entries[i]==0){
            this->drawRect(x*SQUARE_SIZE,y*SQUARE_SIZE,SQUARE_SIZE,SQUARE_SIZE,"#ff0000");
        }
        else if(this->initThresholdSet==true){
            if(i==this->initThreshold){
                this->drawRect(x*SQUARE_SIZE,y*SQUARE_SIZE,SQUARE_SIZE,SQUARE_SIZE,"#00ff00");
                this->drawText(this->entries[i],x*SQUARE_SIZE,y*SQUARE_SIZE,14,"#ffffff");
            }
            else{
                this->drawRect(x*SQUARE_SIZE,y*SQUARE_SIZE,SQUARE_SIZE,SQUARE_SIZE,"#ffffff");
                this->drawText(this->entries[i],x*SQUARE_SIZE,y*SQUARE_SIZE,14,"#ffffff");
            }
        }
        else{
            this->drawRect(x*SQUARE_SIZE,y*SQUARE_SIZE,SQUARE_SIZE,SQUARE_SIZE,"#ffffff");
            this->drawText(this->entries[i],x*SQUARE_SIZE,y*SQUARE_SIZE,14,"#ffffff");
//            this->pwidget->drawRect(x*10,y*10,10,10,"#ffffff");
        }
        i++;
    }*/
   /* auto item = new QGraphicsRectItem(x-50,y-50,w,h);
    item->setBrush(QBrush(Qt::red));

    auto item2 = new QGraphicsRectItem(x,y,w,h);
    item2->setBrush(QBrush(Qt::green));
*/

  //  this->heapCanvas->scene->setSceneRect(0,0,this->heapCanvas->scene->width(),400);
//    this->heapCanvas->scene->centerOn(this->heapCanvas->scene->width()/2,this->heapCanvas->scene->height()/2);
    //this->heapCanvas->scene->addItem(item);
    //this->heapCanvas->scene->addItem(item2);
    //this->heapCanvas->scene->setSceneRect(0,0,this->heapCanvas->scene->width(),400);

   // this->heapCanvas->scene->addRect(x,y,w,h,pen,brush);
}
